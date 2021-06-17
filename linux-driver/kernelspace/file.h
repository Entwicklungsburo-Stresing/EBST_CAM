/*
 * file.h
 *
 * Copyright 2020 Bernhard Lang, University of Geneva
 * Copyright 2020 Entwicklungsbuero Stresing (http://www.stresing.de/)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef _file_h_
#define _file_h_


#include <linux/types.h>
#include <linux/poll.h>


struct inode;
struct file;
struct dev_struct;


int lscpcie_open(struct inode *inode, struct file *filp);
int lscpcie_release(struct inode *inode, struct file *filp);
int buffer_free(struct dev_struct *dev);
int bytes_in_buffer(struct dev_struct *dev);
ssize_t lscpcie_write(struct file *filp, const char __user * buf,
		      size_t len, loff_t * off);
ssize_t lscpcie_read(struct file *filp, char __user * buf, size_t len,
		     loff_t * off);
unsigned int lscpcie_poll(struct file *filp, poll_table *wait);


#endif				/* _file_h_ */
