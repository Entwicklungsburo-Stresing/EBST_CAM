/*****************************************************************//**
 * @file		module-main.c
 * @author		Bernhard Lang
 * @date		05.11.2020
 * @copyright	Copyright 2020-2021 Bernhard Lang, University of Geneva, Entwicklungsbuero Stresing (http://www.stresing.de/) This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License version 2 as published by the Free Software Foundation.
 *********************************************************************/


#include "module-main.h"
#include "pcie.h"
#include "proc.h"
#include "file.h"
#include "device.h"
#include "debug.h"
#include "registers.h"
#include "dma.h"

#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/pci.h>
#include <linux/version.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bernhard Lang");
MODULE_DESCRIPTION("line scan camera by Entwicklungsbuero Gerhard Stresing");

int major = 0;
struct class *lscpcie_class = 0;

/* keep track of information what has been initialised so far */
static int module_status = 0;
static bool chrdev_registered = false;

/* module parameters */
int num_pixels[MAX_BOARDS] = { -1, -1, -1, -1, -1 };
int num_cameras[MAX_BOARDS] = { -1, -1, -1, -1, -1 };
int dma_num_scans[MAX_BOARDS] = { -1, -1, -1, -1, -1 };
int blocks_in_irq[MAX_BOARDS] = { -1, -1, -1, -1, -1 };

static int n_num_pixels = MAX_BOARDS;
static int n_num_cameras = MAX_BOARDS;
static int n_dma_num_scans = MAX_BOARDS;
static int n_blocks_in_irq = MAX_BOARDS;

module_param_array(num_pixels, int, &n_num_pixels, S_IRUGO);
module_param_array(num_cameras, int, &n_num_cameras, S_IRUGO);
module_param_array(dma_num_scans, int, &n_dma_num_scans, S_IRUGO);
module_param_array(blocks_in_irq, int, &n_blocks_in_irq, S_IRUGO);

/* Each driver instance has its own debug flags which can be set individually
   through a ioctl calls. The global debug flag can be set at module load time
   and will apply to each newly created instance. The debug flag D_MODULE is
   stored spearately in the global variable module_debug since it belongs to
   the module as a whole and not to a specific driver instance.
 */
int debug = 0xFFFF;
int debug_module = 0xFFFF;

module_param(debug, int, S_IRUGO);

#define PMDEBUG(...) do {                         \
  if (debug_module)                               \
    printk(KERN_WARNING "lscpcie: " __VA_ARGS__); \
} while (0)


/* initial values for newly created device instance */
const struct dev_struct lscpcie_device_init = {
	.init_status = 0,
	.init_debug_mode = 0,
	.physical_pci_base = 0,
	//.mapped_pci_base = 0,
	.dma_reg = 0,
	.s0_reg = 0,
	.read_available = ATOMIC_INIT(1),
	.write_available = ATOMIC_INIT(1),
	.minor = -1,
	.dma_virtual_mem = 0,
	.control = 0,
	.proc_registers_entry = 0,
	.proc_registers_long_entry = 0,
	.proc_io_entry = 0
};

/* pci identifier */
static struct pci_device_id ids[] = {
	{ PCI_DEVICE(VENDOR_ID, DEVICE_ID), },
	{ 0, }
};

MODULE_DEVICE_TABLE(pci, ids);

/* bring the card back to a known state after a link or bus reset,
   returns 0 only if the card demonstrably works again */
static int lscpcie_revive(struct pci_dev *pdev, struct dev_struct *dev)
{
	u32 id = 0;
	u16 cmd = 0;
	int i;

	if (!dev->control || !dev->s0_reg)
		return -ENODEV;

	/* wait until the endpoint answers config reads again */
	for (i = 0; i < 100; i++) {
		pci_read_config_dword(pdev, PCI_VENDOR_ID, &id);
		if ((id & 0xffff) == VENDOR_ID)
			break;
		msleep(20);
	}
	if ((id & 0xffff) != VENDOR_ID) {
		dev_err(&pdev->dev, "no answer from card after reset\n");
		return -ENODEV;
	}

	pci_restore_state(pdev);	/* BARs, command, MSI, PCIe caps */
	pcie_set_mps(pdev, 128);
	pci_set_master(pdev);
	pci_save_state(pdev);		/* restore_state clears the saved flag */

	pci_read_config_word(pdev, PCI_COMMAND, &cmd);
	if (!(cmd & PCI_COMMAND_MEMORY) || !(cmd & PCI_COMMAND_MASTER) ||
	    !pdev->msi_enabled) {
		dev_err(&pdev->dev, "config state not restored (cmd 0x%04x, msi %d)\n",
			cmd, pdev->msi_enabled);
		return -EIO;
	}

	/* registers must answer, all ones means the link is still down */
	if (read_s0_dword(dev, PCIE_VERS) == 0xffffffff) {
		dev_err(&pdev->dev, "registers still read as all ones\n");
		return -EIO;
	}

	/* put the FPGA side into a known idle state and empty the ring */
	abort_measurement(dev);
	dev->control->write_pos = 0;
	dev->control->read_pos = 0;
	return 0;
}

static void lscpcie_mark_lost(struct dev_struct *dev)
{
	dev->link_lost = true;
	wake_up_interruptible(&dev->readq);
	wake_up_interruptible(&dev->writeq);
	wake_up_interruptible(&dev->proc_readq);
}

static pci_ers_result_t lscpcie_error_detected(struct pci_dev *pdev,
					       pci_channel_state_t state)
{
	struct dev_struct *dev = pci_get_drvdata(pdev);

	dev_err(&pdev->dev, "PCIe error, channel state %d\n", state);
	if (dev)
		lscpcie_mark_lost(dev);
	if (state == pci_channel_io_perm_failure)
		return PCI_ERS_RESULT_DISCONNECT;
	return PCI_ERS_RESULT_NEED_RESET;
}

static pci_ers_result_t lscpcie_slot_reset(struct pci_dev *pdev)
{
	struct dev_struct *dev = pci_get_drvdata(pdev);

	if (!dev || lscpcie_revive(pdev, dev))
		return PCI_ERS_RESULT_DISCONNECT;
	return PCI_ERS_RESULT_RECOVERED;
}

static void lscpcie_resume(struct pci_dev *pdev)
{
	struct dev_struct *dev = pci_get_drvdata(pdev);

	if (dev)
		dev->link_lost = false;
	dev_info(&pdev->dev, "card recovered, the running measurement was lost\n");
}

/* also used for manual resets through /sys/.../reset */
static void lscpcie_reset_prepare(struct pci_dev *pdev)
{
	struct dev_struct *dev = pci_get_drvdata(pdev);

	if (dev)
		lscpcie_mark_lost(dev);
}

static void lscpcie_reset_done(struct pci_dev *pdev)
{
	struct dev_struct *dev = pci_get_drvdata(pdev);

	if (dev && !lscpcie_revive(pdev, dev))
		dev->link_lost = false;
}

static const struct pci_error_handlers lscpcie_err_handler = {
	.error_detected = lscpcie_error_detected,
	.slot_reset     = lscpcie_slot_reset,
	.resume         = lscpcie_resume,
	.reset_prepare  = lscpcie_reset_prepare,
	.reset_done     = lscpcie_reset_done,
};

/* pci initialisation and removal functions */
static struct pci_driver pci_driver = {
	.name = NAME,
	.id_table = ids,
	.probe = probe_lscpcie,
	.remove = remove_lscpcie,
	.err_handler = &lscpcie_err_handler
};

/* general infrastruture and pci registration, devices are initialised
   through probe_lscpcie upon detection by the pci core */
static int __init lscpcie_module_init(void)
{
	dev_t dev;
	int i, result;

	if (debug & D_MODULE)
		debug_module = 1;

	printk(KERN_WARNING NAME " loading module.\n");
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 8, 0)
	lscpcie_class = class_create(NAME);
#else
	lscpcie_class = class_create(THIS_MODULE, NAME);
#endif
	if (IS_ERR(lscpcie_class)) {
		printk(KERN_ERR "Error creating %s class \n", NAME);
		return PTR_ERR(lscpcie_class);
	}
	PMDEBUG("created device class\n");

	/* read number of devices, write adds software devices */
	proc_init_module();

	for (i = 0; i < MAX_BOARDS; i++)
		lscpcie_devices[i] = lscpcie_device_init;

	if (major) {
                /* major device number given at module load */
		dev = MKDEV(major, 0);
		result = register_chrdev_region(dev, MAX_BOARDS, NAME);
	} else {
                /* let the kernel pick the next available major */
		result = alloc_chrdev_region(&dev, 0, MAX_BOARDS, NAME);
		major = MAJOR(dev);
	}

	if (result < 0) {
		printk(KERN_WARNING NAME
		       ": can't obtain major device number %d\n", major);
		goto failed;
	}
	chrdev_registered = true;

	PMDEBUG("got major %d\n", major);

	if ((result = pci_register_driver(&pci_driver)) != 0) {
		printk(KERN_ERR NAME
		       " registering pci device failed with %d", result);
		goto failed;
	}
	module_status |= MOD_PCI_REGISTERED;
	PMDEBUG("registered pci driver\n");

	printk(KERN_WARNING NAME " module ready.\n");

	return 0;

      failed:
	clean_up_lscpcie_module();
	printk(KERN_ERR NAME ": loading failed\n");

	return result;
}

static void __exit lscpcie_module_exit(void)
{
	PMDEBUG("unloading module\n");
	clean_up_lscpcie_module();
	printk(NAME " unloaded\n");
}

/* release all kernel resources allocated at module init in reversed order */
void clean_up_lscpcie_module(void)
{
	int i;

	if (module_status & MOD_PCI_REGISTERED) {
		PMDEBUG("unregistering pci driver\n");
		pci_unregister_driver(&pci_driver);
		module_status &= ~MOD_PCI_REGISTERED;
	}

	for (i = 0; i < MAX_BOARDS; i++)
		if (lscpcie_devices[i].minor >= 0)
			device_clean_up(&lscpcie_devices[i]);
	proc_clean_up_module();

	if (chrdev_registered) {
		PMDEBUG("unregistering major\n");
		unregister_chrdev_region(MKDEV(major, 0), MAX_BOARDS);
		chrdev_registered = false;
	}

	if (lscpcie_class && !IS_ERR(lscpcie_class)) {
		PMDEBUG("destroying class\n");
		class_destroy(lscpcie_class);
	}

	PMDEBUG("done cleaning up module\n");
}

int get_device_number(const struct dev_struct *dev)
{
	int i;

	for (i = 0; i < MAX_BOARDS; i++)
		if (dev == lscpcie_devices + i)
			return i;

	return -1;
}

module_init(lscpcie_module_init);
module_exit(lscpcie_module_exit);
