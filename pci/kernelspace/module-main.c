/* module-main.c
 *
 * Copyright (C) 2010-2016 Bernhard Lang, University of Geneva
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <linux/module.h>
#include <linux/proc_fs.h>

#include "defaults.h"
#include "device.h"
#include "proc.h"
#include "file.h"
#include "camera.h"
#include "module-main.h"


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bernhard Lang");
MODULE_DESCRIPTION("line scan camera by Entwicklungsbuero Gerhard Stresing");

static int max_boards = -1;
static int debug_module = 0;
static int num_pixels[MAX_BOARDS]      = { -1, -1, -1, -1 };
static int first_pixel[MAX_BOARDS]     = { -1, -1, -1, -1 };
static int used_pixels[MAX_BOARDS]     = { -1, -1, -1, -1 };
static int lines[MAX_BOARDS]           = { -1, -1, -1, -1 };
static int vfreq[MAX_BOARDS]           = { -1, -1, -1, -1 };
static int num_dma_buffers[MAX_BOARDS] = { -1, -1, -1, -1 };
static int plx_resource[MAX_BOARDS]    = { -1, -1, -1, -1 };
static int camera_resource[MAX_BOARDS] = { -1, -1, -1, -1 };
static int use_interrupt[MAX_BOARDS]   = { -1, -1, -1, -1 };

static int n_num_pixels      = MAX_BOARDS;
static int n_first_pixel     = MAX_BOARDS;
static int n_used_pixels     = MAX_BOARDS;
static int n_lines           = MAX_BOARDS;
static int n_vfreq           = MAX_BOARDS;
static int n_num_dma_buffers = MAX_BOARDS;
static int n_plx_resource    = MAX_BOARDS;
static int n_camera_resource = MAX_BOARDS;
static int n_use_interrupt   = MAX_BOARDS;

module_param(max_boards,            int, S_IRUGO);
module_param(debug_module,          int, S_IRUGO);
module_param_array(num_pixels,      int, &n_num_pixels,      S_IRUGO);
module_param_array(first_pixel,     int, &n_first_pixel,     S_IRUGO);
module_param_array(used_pixels,     int, &n_used_pixels,     S_IRUGO);
module_param_array(lines,           int, &n_lines,           S_IRUGO);
module_param_array(vfreq,           int, &n_vfreq,           S_IRUGO);
module_param_array(num_dma_buffers, int, &n_num_dma_buffers, S_IRUGO);
module_param_array(plx_resource,    int, &n_plx_resource,    S_IRUGO);
module_param_array(camera_resource, int, &n_camera_resource, S_IRUGO);
module_param_array(use_interrupt  , int, &n_use_interrupt,   S_IRUGO);

const struct dev_struct lscpci_device_init = {
  .initialised = 0,
  .hardware_present = 0,
  .n_pixels = NUM_PIXELS,
  .lines = LINES,
  .vfreq = VFREQ,
  .pci_config = 0,
  .pci_camera = 0,
  .pci_dev = 0,
  .dma.n_buffers = NUM_DMA_BUFFERS,
  .dma.descriptor_buffer = 0,
  .dma.buffers = 0,
  .dma.started = 0,
  .proc_entry = 0,
  .proc_registers_entry = 0,
  .have_irq = 0,
  .irq_count = 0,
  .vars.xckdelay = 3,
  .vars.pclk = 4,
  .vars.lines = 64,
  .vars.vfreq = 7,
  .vars.delay = 0x12C,
  .read_available = ATOMIC_INIT(1),
  .write_available = ATOMIC_INIT(1),
  .status = 0
};

static struct dev_struct lscpci_device[MAX_BOARDS+1];
static struct class *lscpci_class = 0;

struct dev_struct *device_data(uint8_t devno) {
  return devno < max_boards ? &lscpci_device[devno] : NULL;
}

static struct pci_device_id ids[] = {
  { PCI_DEVICE(VENDOR_ID, DEVICE_ID), },
  { 0, }
};
MODULE_DEVICE_TABLE(pci, ids);

static struct pci_driver pci_driver = {
  .name = NAME,
  .id_table = ids,
  .probe = probe_plx9056,
  .remove = remove_plx9056
};

struct file_operations proc_fops = {
  .owner = THIS_MODULE,
  .read = camera_read_proc
};

struct file_operations proc_registers_fops = {
  .owner = THIS_MODULE,
  .read = camera_read_registers_proc
};

#ifdef PDEBUG
# undef PDEBUG
#endif
#define PDEBUG(fmt, args...) do {                                 \
  if (debug_module) printk(KERN_WARNING "lscpci: " fmt, ## args); \
} while (0)


static int __init camera_module_init(void) {
  int result, i;
  dev_t dev;
  struct dev_struct *pdev;

  if (max_boards > MAX_BOARDS) {
    printk(KERN_WARNING NAME " %d is the maximum number of boards "
           " (c.f. the documentation how to change this).\n", MAX_BOARDS);
    max_boards = MAX_BOARDS;
  } else
    if (max_boards < 0) max_boards = MAX_BOARDS;

  lscpci_class = class_create(THIS_MODULE, NAME);
  if (IS_ERR(lscpci_class)) {
    printk(KERN_ERR "Error creating %s class \n", NAME);
    return PTR_ERR(lscpci_class);
  }

  for (i = 0; i < max_boards; i++) {
    lscpci_device[i] = lscpci_device_init;
    pdev = &lscpci_device[i];

    if ((n_num_pixels > i) && (num_pixels[i] > 0))
      pdev->n_pixels = num_pixels[i];

    if ((n_num_dma_buffers > i) && (num_dma_buffers[i] > 0))
      pdev->dma.n_buffers = num_dma_buffers[i];

#ifdef WITH_POLLING
    timer_setup(&pdev->poll_timer, camera_poll, 0);
    /*
    init_timer(&pdev->poll_timer);
    pdev->poll_timer.function = camera_poll;
    pdev->poll_timer.data = (unsigned long) pdev;
    */
#endif
  }
  lscpci_device[i] = lscpci_device_init; /* dummy at end */

  PDEBUG("registering major\n");
  if (major) {
    dev = MKDEV(major, 0);
    result = register_chrdev_region(dev, max_boards, NAME);
  } else {
    result = alloc_chrdev_region(&dev, 0, max_boards, NAME);
    major = MAJOR(dev);
  }

  if (result < 0) {
    printk(KERN_WARNING NAME": can't obtain major device number %d\n", major);
    goto failed;
  }

  PDEBUG("registering pci device\n");
  if ((result = pci_register_driver(&pci_driver)) != 0) {
    printk(KERN_ERR NAME " registering pci device failed with %d", result);
    return result;
  }

  //-->> this doesn't make sense, revise
  i = 0;
  while (lscpci_device[i].initialised) i++;
  printk(KERN_WARNING NAME" found %d boards\n", i);

  if (!i) {
    result = init_board(lscpci_device, 0);
    if (result) {
      printk(KERN_ERR NAME": failed to initialise debug board 0\n");
      goto failed;
    }
    result = init_board(&lscpci_device[1], 1);
    if (result) {
      printk(KERN_ERR NAME": failed to initialise debug board 1\n");
      goto failed;
    }
  }

  printk(KERN_WARNING NAME" ready.\n");

  return 0;

 failed:
  clean_up();
  printk(KERN_ERR NAME": loading lscpci failed\n");
  return result;
}

static void __exit camera_module_exit(void) {
  clean_up();
  printk(NAME" unloaded\n");
}

void clean_up(void) {
  int i;
  for (i = 0; i < max_boards; i++)
    if (lscpci_device[i].initialised)
      clean_up_board(&lscpci_device[i], i);

  PDEBUG("unregistering major\n");
  if (major) unregister_chrdev_region(MKDEV(major, 0), 1);

  PDEBUG("unregistering pci lscpci_device\n");
  pci_unregister_driver(&pci_driver);

  if (lscpci_class) class_destroy(lscpci_class);
}

int init_board(struct dev_struct *dev, uint8_t devno) {
  int result;
  char name[32];
  void *ptr_error;

  PDEBUG("initialising board %d\n", devno);

  if (devno >= max_boards) {
    printk(KERN_ERR NAME ": device number exceeds maximal number of boards\n");
    return -ENODEV;
  }

  if (dev->initialised) {
    printk(KERN_ERR NAME ": board %d is already initialised\n", devno);
    return -EBUSY;
  }

  dev->dev = MKDEV(major, devno);
  cdev_init(&dev->cdev, &fops);
  dev->cdev.owner = THIS_MODULE;
  dev->cdev.ops = &fops;
  result = cdev_add(&dev->cdev, dev->dev, 1);
  if (result) {
    printk(KERN_WARNING "Error %d adding "NAME" %d", result, minor);
    goto failed;
  }

  ptr_error
    = device_create(lscpci_class, NULL, dev->dev, NULL, "%s%d", NAME, devno);
  if (IS_ERR(ptr_error)) {
    printk(KERN_ERR "creation of device %s%d failed\n", NAME, devno);
    cdev_del(&dev->cdev);
    return PTR_ERR(ptr_error);
  }

  PDEBUG("initialising DMA\n");
  if ((result = init_dma(dev)) != 0) {
    printk(KERN_WARNING NAME " DMA failed\n");
    goto failed;
  }
  PDEBUG("DMA ok\n");

  PDEBUG("creating proc entries\n");

  sprintf(name, "%s%d", NAME, devno);
  dev->proc_entry = proc_create_data(name, 0, NULL, &proc_fops, dev);
  sprintf(name, "%s_registers%d", NAME, devno);
  dev->proc_registers_entry
    = proc_create_data(name, 0, NULL, &proc_registers_fops, dev);

  PDEBUG("initialising wait queues\n");

  init_waitqueue_head(&dev->writeq);
  init_waitqueue_head(&dev->readq);
  sema_init(&dev->write_sem, 1);
  sema_init(&dev->read_sem, 1);
  sema_init(&dev->size_sem, 1);

  dev->initialised = 1;

  return 0;

 failed:
  clean_up_board(dev, devno);
  printk(KERN_WARNING NAME": loading lscpci failed\n");
  return result;

}

void clean_up_board(struct dev_struct *dev, uint8_t devno) {
  char name[32];
  if (!dev->initialised) return;
  if (dev->proc_entry) {
    sprintf(name, "%s%d", NAME, devno);
    remove_proc_entry(name, NULL);
    dev->proc_entry = 0;
  }
  if (dev->proc_registers_entry) {
    sprintf(name, "%s_registers%d", NAME, devno);
    remove_proc_entry(name, NULL);
    dev->proc_registers_entry = 0;
  }
  PDEBUG("unregistering minor\n");

  device_destroy(lscpci_class, dev->dev);

  cdev_del(&dev->cdev);
  dev->initialised = 0;
}

void set_debug_module(int set) {
  debug_module = set;
}

int get_debug_module(void) {
  return debug_module;
}

module_init(camera_module_init);
module_exit(camera_module_exit);
