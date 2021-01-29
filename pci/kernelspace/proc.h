/* proc.h
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

#ifndef _proc_h_
#define _proc_h_

#include <linux/fs.h>

ssize_t camera_read_proc(struct file *filp, char __user *buf, size_t count,
                         loff_t *offp);
ssize_t camera_read_registers_proc(struct file *filp, char __user *buf,
                                   size_t count, loff_t *offp);

#endif /* _proc_h_ */
