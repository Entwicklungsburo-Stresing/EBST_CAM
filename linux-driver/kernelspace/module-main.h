/* module-main.h
 *
 * Copyright 2020 Bernhard Lang, University of Geneva
 * Copyright 2020 Entwicklungsbuero Stresing (http://www.stresing.de/)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _module_main_h_
#define _module_main_h_


struct dev_struct;

extern int major;
extern int debug_module;
extern int debug;

#define MAX_BOARDS 5

/**
 * The parameter DEFAULT_NUMBER_OF_PIXELS and DEFAULT_DMA_NUM_SCANS
 * are now used as a maximum.
 * These values are used to the set the size of the DMA buffer.
 * This is done when the kernel module is loaded, which happens at boot time.
 * In general this is not a good system design, because these parameters
 * are set in software and can change. That is the reason why these values
 * can now be seen as maxium values, so that the DMA buffer is always big enough.
 * Of course this means, the DMA buffer is too big most cases. In numbers:
 * pixel * dma size in scans * * sizeof(uint16_t)
 * = 2048 * 1000 * 2 b = 4096000 b = 4000 kb = 3,9 Mb
 * If this leads to problems, DEFAULT_NUMBER_OF_PIXELS can safley be reduced
 * to the real number of pixlels of your camera. 4000 kb seems to be the
 * maximum which the Linux kernel allows to allocate. So DEFAULT_NUMBER_OF_PIXELS
 * cannot be higher than 2048.
 * 
 * Update: Since lscpie.c doesn't allow 2048, I set this to 1088 now.
*/
#define DEFAULT_NUMBER_OF_PIXELS    1088
#define DEFAULT_DMA_NUM_SCANS       1000
/**
 * DEFAULT_NUMBER_OF_CAMERAS is not a critical parameter for the kernel module.
*/
#define DEFAULT_NUMBER_OF_CAMERAS   1


extern int num_pixels[MAX_BOARDS];
extern int num_cameras[MAX_BOARDS];
extern int dma_num_scans[MAX_BOARDS];

extern struct class *lscpcie_class;

void clean_up_lscpcie_module(void);
int get_device_number(const struct dev_struct *);

#endif 	/* _module_main_h_ */
