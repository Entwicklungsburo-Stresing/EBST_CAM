/*
 * module-main.h
 *
 * Copyright 2020 Bernhard Lang, University of Geneva
 * Copyright 2020 Entwicklungsbuero Stresing (http://www.stresing.de/)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef _module_main_h_
#define _module_main_h_


struct dev_struct;

extern int major;
extern int debug_module;
extern int debug;

#define MAX_BOARDS 6

#define DEFAULT_NUMBER_OF_PIXELS    576
#define DEFAULT_NUMBER_OF_CAMERAS   1
#define DEFAULT_NUM_SCANS           1000
#define DEFAULT_NUM_BLOCKS          2

extern int num_pixels[MAX_BOARDS];
extern int num_cameras[MAX_BOARDS];
extern int num_scans[MAX_BOARDS];
extern int num_blocks[MAX_BOARDS];

extern struct class *lscpcie_class;

void clean_up_lscpcie_module(void);
int get_device_number(const struct dev_struct *);

#endif /* _module_main_h_ */