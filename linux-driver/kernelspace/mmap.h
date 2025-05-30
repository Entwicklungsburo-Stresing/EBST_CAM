/*****************************************************************//**
 * @file		mmap.h
 * @author		Bernhard Lang
 * @date		05.11.2020
 * @copyright	Copyright 2020-2021 Bernhard Lang, University of Geneva, Entwicklungsbuero Stresing (http://www.stresing.de/) This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License version 2 as published by the Free Software Foundation.
 *********************************************************************/

#ifndef _mmap_h_
#define _mmap_h_


struct inode;
struct file;
struct vm_area_struct;


int mmap_register_remap_mmap(struct file *filp,
			     struct vm_area_struct *vma);


#endif  /* _mmap_h_ */
