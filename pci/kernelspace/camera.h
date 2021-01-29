/* camera.h
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

#ifndef _camera_h_
#define _camera_h_

#include "../userspace/types.h"

struct dev_struct;

int camera_init(struct dev_struct *, struct board_vars *vars);
int camera_start(int with_readout, struct dev_struct *);
int camera_stop(struct dev_struct *);
int camera_finish(struct dev_struct *);
int camera_clear_fifo(struct dev_struct *);

extern int init_hardware(struct dev_struct *, struct board_vars *vars);
extern int clear_fifo(struct dev_struct *dev);
extern int enable_trigger(struct dev_struct *dev);
extern int disable_trigger(struct dev_struct *dev);
extern int read_fifo_overflow(struct dev_struct *dev);
extern int write_register(u8 what, u32 where, struct dev_struct *dev);
extern int write_register_word(u16 what, u32 where, struct dev_struct *dev);
extern int write_register_dword(u32 what, u32 where, struct dev_struct *dev);
extern int write_plx(u8 what, u32 where, struct dev_struct *dev);

extern int get_params(struct dev_struct *dev, struct lscpci_param *params);
extern int dump_registers(struct dev_struct *dev, void __user *arg);

#endif /* _camera_h_ */
