/*****************************************************************//**
 * @file		common.h
 * @author		Bernhard Lang
 * @date		05.11.2020
 * @copyright	Copyright 2020-2021 Bernhard Lang, University of Geneva, Entwicklungsbuero Stresing (http://www.stresing.de/). This program is free software; you can redistribute it and/or modify it under the terms of the LPGL-3.0 as published by the Free Software Foundation.
 *********************************************************************/

#ifndef _lscpcie_examples_common_h_
#define _lscpcie_examples_common_h_

#include "../lscpcie.h"
#include "../local-config.h"
#ifdef __cplusplus
extern "C" {
#endif

struct camera_info_struct {
	int n_blocks, n_scans, mem_size;
	enum trigger_mode trigger_mode;
	pixel_t *data;
	struct dev_descr *dev;
};

int init_7030(unsigned int dev_no);
int readout_init(int argc, char **argv, struct camera_info_struct *info);
int fetch_mapped_data(struct dev_descr *dev, uint8_t *data, size_t max);
void print_data(const struct camera_info_struct *info);
int kbhit(void);

#ifdef __cplusplus
}
#endif
#endif /* _lscpcie_examples_common_h_ */
