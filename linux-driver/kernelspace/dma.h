/*****************************************************************//**
 * @file		dma.h
 * @author		Bernhard Lang
 * @date		05.11.2020
 * @copyright	Copyright 2020 Bernhard Lang, University of Geneva, Entwicklungsbuero Stresing (http://www.stresing.de/) This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License version 2 as published by the Free Software Foundation.
 *********************************************************************/

#ifndef _dma_h_
#define _dma_h_

#include <linux/types.h>

struct dev_struct;

int dma_init(struct dev_struct *dev);
void dma_finish(struct dev_struct *dev);
int dma_start(struct dev_struct *dev);
int dma_end(struct dev_struct *dev);
void reset_dma(struct dev_struct *dev);
void abort_measurement(struct dev_struct *dev);

#endif /* _dma_h_ */
