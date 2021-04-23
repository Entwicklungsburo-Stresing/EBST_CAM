/* proc.h
 *
 * Copyright 2020-2021 Entwicklungsbuero Stresing (http://www.stresing.de/)
 * Copyright 2020-2021 Bernhard Lang, University of Geneva
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 */

#ifndef _proc_h_
#define _proc_h_


#include <linux/fs.h>

struct dev_struct;

void proc_init_module(void);
void proc_init(struct dev_struct *dev);
void proc_clean_up(struct dev_struct *dev);
void proc_clean_up_module(void);


#endif	/* _proc_h_ */
