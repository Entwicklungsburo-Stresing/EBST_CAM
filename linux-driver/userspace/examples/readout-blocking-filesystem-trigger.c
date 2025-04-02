/*****************************************************************//**
 * @file		readout-blocking-filesystem-trigger.c
 * @author		Bernhard Lang
 * @date		05.11.2020
 * @copyright	Copyright 2020-2021 Bernhard Lang, University of Geneva, Entwicklungsbuero Stresing (http://www.stresing.de/). This program is free software; you can redistribute it and/or modify it under the terms of the LPGL-3.0 as published by the Free Software Foundation.
 *********************************************************************/

#include "local-config.h"
#include "common.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <memory.h>

/* Performs a camera read with blocking on /dev/lspcie0.
   After intitialising and starting the cameras, a read system call on
   /dev/lspcie0 blocks the execution until data has arrived in the buffer.
   The corresponding wakeup is issued by the driver's interrupt routine.
   Copying of the data is done within the read system call.
   The trigger flag in CRTLA is polled ẗo detect triggers and start
   new block readings until the number of bytes copied from the mapped DMA
   buffer reaches the goal defined by the two command line arguments
   <number of scans> <number of blocks>.
*/

/* Acquire one block of data. Poll first XCKMSB /RS for being low and then
   call read which returns only once some new data has been copied.
   Loop over if less than the needed data has been copied. */

int lscpcie_acquire_block_fs(struct dev_descr *dev, uint8_t *data,
			size_t n_scans, int camera_file_handle) {
	int result, bytes_read = 0;
	size_t block_size =
		dev->control->number_of_pixels * dev->control->number_of_cameras
		* sizeof(pixel_t) * n_scans;

	result = lscpcie_start_block(dev);
	lscpcie_dump_s0(dev);
	if (result < 0)
		return result;

	do {
		result = read(camera_file_handle, data + bytes_read,
			block_size - bytes_read);
		if (result < 0)
			return result;

		bytes_read += result;
		fprintf(stderr, "got %d bytes of data\n", result);
	} while (bytes_read < block_size);

        result = lscpcie_end_block(dev);
	if (result < 0)
		return result;

	return bytes_read;
}

int main(int argc, char **argv)
{
  int result, bytes_read, block_count = 0;
	struct camera_info_struct info;

	result = readout_init(argc, argv, &info);
	if (result < 0)
		return result;

	info.dev->s0->XCK.bytes.MSB &= 0xBF; // stop S Timer
	info.dev->s0->EC = 0; // reset SEC

	bytes_read = 0;
	info.dev->s0->XCK.bytes.MSB |= (1<<XCKMSB_EXT_TRIGGER);

	do {
		// wait for trigger signal
		if (!(info.dev->s0->CTRLA & (1 << CTRLA_TSTART)))
			continue;

		result = lscpcie_acquire_block_fs(info.dev,
						(uint8_t *) info.data
						+ bytes_read,
						info.n_scans, info.dev->handle);
		if (result < 0) {
			fprintf(stderr, "error %d when acquiring block\n",
				result);
			goto out;
		}
		bytes_read += result;
		fprintf(stderr, "having now %d (%d blocks)\n", bytes_read,
			++block_count);
	} while ((bytes_read < info.mem_size) &&! kbhit());

	if (kbhit()) {
		fprintf(stderr, "measurement interrupted\n");
		return 1;
	}

	fprintf(stderr, "finished measurement\n");

	result = lscpcie_end_acquire(info.dev);
	if (result)
		fprintf(stderr, "error %d when finishing acquisition\n", result);
	else
		print_data(&info);

      out:
	if (info.data)
		free(info.data);
	lscpcie_close(0);

	return result;
}
