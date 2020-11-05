/* file.h
 *
 * Copyright (C) 2010-2020 Bernhard Lang, University of Geneva
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
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

#ifndef _file_h_
#define _file_h_

#include <linux/fs.h>
#include "device.h"

extern struct file_operations fops;
extern int major;
extern int minor;



int driver_open_camera(struct inode *inode, struct file *filp);
int release_camera(struct inode *inode, struct file *filp);
ssize_t write_camera(struct file *filp, const char __user *buf, size_t len,
                  loff_t *off);
ssize_t read_camera(struct file *filp, char __user *buf, size_t len,
                    loff_t *off);

ssize_t bytes_in_buffer(struct dev_struct *dev);
ssize_t buffer_free(struct dev_struct *dev);
u64 get_buffer_pointers(struct dev_struct *dev);

int setup_cdev(struct dev_struct *dev, int index);

#endif /* _file_h_ */
