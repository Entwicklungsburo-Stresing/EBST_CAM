/*****************************************************************//**
 * @file		proc.h
 * @author		Bernhard Lang
 * @date		05.11.2020
 * @copyright	Copyright 2020-2021 Bernhard Lang, University of Geneva, Entwicklungsbuero Stresing (http://www.stresing.de/) This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License version 2 as published by the Free Software Foundation.
 *********************************************************************/

#ifndef _proc_h_
#define _proc_h_


#include <linux/fs.h>

struct dev_struct;

void proc_init_module(void);
void proc_init(struct dev_struct *dev);
void proc_clean_up(struct dev_struct *dev);
void proc_clean_up_module(void);


#endif	/* _proc_h_ */
