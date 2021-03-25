/* proc.c
 *
 * Copyright 2020-2021 Entwicklungsbuero Stresing (http://www.stresing.de/)
 * Copyright 2020-2021 Bernhard Lang, University of Geneva
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 */

#include "proc.h"
#include "device.h"
#include "registers.h"
#include "debug.h"
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

/* This proc entry in created upon loading the module. It tells the number of
   devices and can be used to add a debugging device. */

static struct proc_dir_entry *proc_entry = NULL;

/* count the number of initialised devices and return it as a two-byte
   number, */

ssize_t lscpcie_read_proc(struct file *filp, char __user * buf,
			  size_t count, loff_t * offp)
{
	int i, count_boards = 0, count_debug = 0;
	char result[3];

	// need at least two bytes space in buffer
	if (count < 3)
		return -EAGAIN;

	if (*offp)
		return 0; // second call retunrs zero to indicate end of file

	for (i = 0; i < MAX_BOARDS; i++)
		if (lscpcie_devices[i].minor >= 0) {
			if (lscpcie_devices[i].
			    status & DEV_HARDWARE_PRESENT)
				count_boards++;
			else
				count_debug++;
		}

	result[0] = '0' + count_boards;
	result[1] = '0' + count_debug;
	result[2] = '\n';

	if (copy_to_user((char __user *) buf, result, 3))
		return -EFAULT;

	*offp += 3;

	return 3;
}

/* Add debug device. Two colon-separated numbers are expected, first the number
   of pixels, second the number of cameras. The input has to be terminated
   with a new-line character. */

ssize_t lscpcie_write_proc(struct file *filp, const char __user * buf,
			   size_t count, loff_t * offp)
{
	int i, slot, result;
	static int n_pixels = 0;
	static int n_cameras = -1;
	char c;

	for (i = 0; i < count; i++) {
		get_user(c, buf++);

		/* start with pixels */
		if (n_cameras < 0) {
			if ((c >= '0') && (c <= '9')) {
				n_pixels = n_pixels * 10 + (c - '0');
				continue;
			}
			if (c != ':')
				goto out_done;
			n_cameras = 0;
			continue;
		}

		/* now cameras */
		if ((c >= '0') && (c <= '9')) {
			n_cameras = n_cameras * 10 + (c - '0');
			continue;
		}

		/* it should be a line end once we get here */
		if (c != '\n')
			goto out_done;

		for (slot = 0; slot < MAX_BOARDS; slot++)
			if (lscpcie_devices[slot].minor < 0) {
				result =
				    device_init(&lscpcie_devices
						[slot], slot);
				if (result < 0) {
					/* something unexpected happend,
					   report the error */
					i = result;
					break;
				}
				proc_init(&lscpcie_devices[slot]);
				break;
			}
		if (slot == MAX_BOARDS)
			i = -ENOMEM;	/* no free device left */
		goto out_done;
	}

	return i >= 0 ? i + 1 : i;

      out_done:
	n_pixels = 0;
	n_cameras = -1;

	return i >= 0 ? i + 1 : i;
}

struct file_operations proc_fops = {
	.owner = THIS_MODULE,
	.read = lscpcie_read_proc,
	.write = lscpcie_write_proc
};

void proc_init_module(void)
{
	if (debug_module)
		printk(KERN_WARNING NAME " creating module proc entry\n");

	proc_entry
	    =
	    proc_create_data(NAME, S_IRUGO | S_IWUGO, NULL, &proc_fops,
			     NULL);
}

void proc_clean_up_module(void)
{
	if (debug_module)
		printk(KERN_WARNING
		       "lscpcie: removing module proc entry\n");
	if (proc_entry) {
		remove_proc_entry(NAME, NULL);
		proc_entry = NULL;
	}
}

/* convert a single byte into a binary ascii number as 0b1010'0101 */

const char *binary(char *buf, unsigned char n)
{
	int pos = 10, i, mask = 0x01;
	for (i = 0; i < 8; i++, mask <<= 1) {
		*(buf + pos--) = (n & mask) ? '1' : '0';
		if (i == 3)
			*(buf + pos--) = '\'';
	}
	buf[0] = '0';
	buf[1] = 'b';
	buf[11] = 0;
	return buf;
}

/* Get the contents of the pci io registers and print them out line by line
   in a readable format.
   Since there is no garantie that the buffer provided by the system call is
   large enough for the entire register contents, a register count keeps track
   of what has already been printed.
*/

ssize_t lscpcie_read_registers_proc(struct file *filp, char __user * buf,
				    size_t count, loff_t * offp)
{
	ssize_t len = *offp, l;
	int val, result;
	struct dev_struct *dev = PDE_DATA(file_inode(filp));
	char line_buf[128], ascii_buffer[12];

	if (!(dev->status & DEV_HARDWARE_PRESENT))
		return -ENODEV;

	if ((dev->proc_actual_register > 0x80)
	    && !reg_names[dev->proc_actual_register - 0x80][0]) {
		/* last register has been printed in previous call */
		dev->proc_actual_register = 0;
		return 0;
	}

	if (!dev->proc_actual_register) {
		/* first call */
		if (count < 4)
			return 0;	/* not enogh space */
		result = copy_to_user(buf + len, "DMA\n", 4);
		if (result)
			return -EFAULT;
		len += 4;
		dev->proc_actual_register++;
	}

	while ((dev->proc_actual_register < 0x80)
	       && dma_reg_names[dev->proc_actual_register][0]) {
		val =
		    ioread8(dev->mapped_pci_base +
			    dev->proc_actual_register);
		l = snprintf(line_buf, 127, "  0x%02x (%s): 0x%02x %s\n",
			     dev->proc_actual_register,
			     dma_reg_names[dev->proc_actual_register], val,
			     binary(ascii_buffer, val));
		if (len + l >= count)
			return len;	/* not enough space for next line */
		result = copy_to_user(buf + len, line_buf, l + 1);
		if (result)
			return -EFAULT;
		len += l;
		dev->proc_actual_register++;
	}

	if (len + 8 >= count)
		return len;

	result = copy_to_user(buf + len, "space 0\n", 8);
	if (result)
		return -EFAULT;
	len += 8;
	dev->proc_actual_register = 0x80;

	while (reg_names[dev->proc_actual_register - 0x80][0]) {
		val =
		    ioread8(dev->mapped_pci_base +
			    dev->proc_actual_register);
		l = snprintf(line_buf, 127, "  0x%02x (%s): 0x%02x %s\n",
			     dev->proc_actual_register,
			     reg_names[dev->proc_actual_register - 0x80],
			     val, binary(ascii_buffer, val));
		if (len + l >= count)
			return len;
		result = copy_to_user(buf + len, line_buf, l + 1);
		if (result)
			return -EFAULT;
		len += l;
		dev->proc_actual_register++;
	}

	return len;
}

ssize_t lscpcie_read_registers_long_proc(struct file *filp,
					 char __user * buf, size_t count,
					 loff_t * offp)
{
	ssize_t len = *offp, l;
	int result;
	u32 val;
	struct dev_struct *dev = PDE_DATA(file_inode(filp));
	char line_buf[128];

	if (!(dev->status & DEV_HARDWARE_PRESENT))
		return -ENODEV;

	if (!reg_names_long[dev->proc_actual_register_long][0]) {
		/* last register has been printed in previous call */
		dev->proc_actual_register_long = 0;
		return 0;
	}

	while (reg_names_long[dev->proc_actual_register_long][0]) {
		val =
		    ioread32(dev->mapped_pci_base +
			     4 * dev->proc_actual_register_long + 0x80);
		l = snprintf(line_buf, 127, "  0x%02x (%s): 0x%08x\n",
			     dev->proc_actual_register_long,
			     reg_names_long[dev->
					    proc_actual_register_long],
			     val);
		if (len + l >= count)
			return len;
		result = copy_to_user(buf + len, line_buf, l + 1);
		if (result)
			return -EFAULT;
		len += l;
		dev->proc_actual_register_long++;
	}

	return len;
}

/* read register contents and dump them to the buffer */

ssize_t lscpcie_read_io_proc(struct file *filp, char __user * buf,
			     size_t count, loff_t * offp)
{
	int bytes_to_copy, total_bytes, result;
	static dma_reg_t dma_data;
	static s0_t s0_data;
	int offset = *offp;
	struct dev_struct *dev = PDE_DATA(file_inode(filp));

	if (!(dev->status & DEV_HARDWARE_PRESENT))
		return -ENODEV;

	if (offset >= sizeof(dma_reg_t) + sizeof(s0_t))
		return 0;	/* done */

	if (!offset) {
		printk(KERN_WARNING NAME ": copying register contents\n");
		memcpy_fromio(&dma_data, dev->mapped_pci_base,
			      sizeof(dma_reg_t));
		memcpy_fromio(&s0_data, dev->mapped_pci_base + 0x80,
			      sizeof(s0_t));
	}
	// dma registers
	bytes_to_copy = sizeof(dma_reg_t) - offset;
	if (bytes_to_copy > 0) {
		if (bytes_to_copy > count)
			bytes_to_copy = count;
		result =
		    copy_to_user(buf, ((u8 *) & dma_data) + offset,
				 bytes_to_copy);
		if (result)
			return -EFAULT;
		count -= bytes_to_copy;
		offset += bytes_to_copy;
		total_bytes = bytes_to_copy;
	} else {
		total_bytes = 0;
	}

	// s0 registers
	offset -= sizeof(dma_reg_t);
	bytes_to_copy = sizeof(s0_t) - offset;
	if (bytes_to_copy > 0) {
		if (bytes_to_copy > count)
			bytes_to_copy = count;
		result
		    =
		    copy_to_user(buf + total_bytes,
				 ((u8 *) & s0_data) + offset,
				 bytes_to_copy);
		if (result)
			return -EFAULT;
		total_bytes += bytes_to_copy;
	}

	*offp += total_bytes;

	return total_bytes;
}

/* This proc entry returns nothing. It is used to block a 'reading' process
   until something appears in the dma buffer. The buffer's content may then
   directly be read via memory mapping. */

ssize_t lscpcie_read_data_proc(struct file *filp, char __user * buf,
			       size_t count, loff_t * offp)
{
	return 0;
}

struct file_operations proc_data_fops = {
	.owner = THIS_MODULE,
	.read = lscpcie_read_data_proc
};

struct file_operations proc_registers_fops = {
	.owner = THIS_MODULE,
	.read = lscpcie_read_registers_proc
};

struct file_operations proc_registers_long_fops = {
	.owner = THIS_MODULE,
	.read = lscpcie_read_registers_long_proc
};

struct file_operations proc_io_fops = {
	.owner = THIS_MODULE,
	.read = lscpcie_read_io_proc
};

/* init proc files per device instance */

void proc_init(struct dev_struct *dev)
{
	char name[32];

	PDEBUG(D_PROC, "creating main device proc entry");
	sprintf(name, "%s%d", NAME, dev->minor);
	dev->proc_data_entry =
	    proc_create_data(name, 0, NULL, &proc_data_fops, dev);

	if (!(dev->status & DEV_HARDWARE_PRESENT)) {
		PDEBUG(D_PROC,
		       "no hardware present, not creating io proc entries");
		return;
	}

	sprintf(name, "%s%d_registers", NAME, dev->minor);
	dev->proc_registers_entry
	    = proc_create_data(name, 0, NULL, &proc_registers_fops, dev);

	sprintf(name, "%s%d_registers_long", NAME, dev->minor);
	dev->proc_registers_long_entry
	    =
	    proc_create_data(name, 0, NULL, &proc_registers_long_fops,
			     dev);

	sprintf(name, "%s%d_pci_io", NAME, dev->minor);
	dev->proc_io_entry
	    = proc_create_data(name, 0, NULL, &proc_io_fops, dev);
}

void proc_clean_up(struct dev_struct *dev)
{
	char name[32];

	PDEBUG(D_PROC, "removing proc entries\n");

	if (dev->proc_data_entry) {
		sprintf(name, "%s%d", NAME, dev->minor);
		remove_proc_entry(name, NULL);
		dev->proc_data_entry = 0;
	}

	if (dev->proc_registers_entry) {
		sprintf(name, "%s%d_registers", NAME, dev->minor);
		remove_proc_entry(name, NULL);
		dev->proc_registers_entry = 0;
	}

	if (dev->proc_registers_long_entry) {
		sprintf(name, "%s%d_registers_long", NAME, dev->minor);
		remove_proc_entry(name, NULL);
		dev->proc_registers_long_entry = 0;
	}

	if (dev->proc_io_entry) {
		sprintf(name, "%s%d_pci_io", NAME, dev->minor);
		remove_proc_entry(name, NULL);
		dev->proc_io_entry = 0;
	}

	dev->proc_actual_register = 0;
	dev->proc_actual_register_long = 0;
}
