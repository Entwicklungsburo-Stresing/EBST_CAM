/* module-main.h
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

#ifndef _module_main_h_
#define _module_main_h_

struct dev_struct;

int init_board(struct dev_struct *dev, u8 devno);
void clean_up_board(struct dev_struct *dev, u8 devno);
void clean_up(void);
struct dev_struct *device_data(u8 devno);
void set_debug_module(int);
int get_debug_module(void);

#endif /* _module_main_h_ */
