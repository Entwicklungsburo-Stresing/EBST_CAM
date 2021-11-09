/* readout-loop.c
 *
 * Copyright 2020-2021 Bernhard Lang, University of Geneva
 * Copyright 2020-2021 Entwicklungsbuero Stresing (http://www.stresing.de/)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "local-config.h"
#include "lscpcie.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include <errno.h>


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


int no_acquisition = 0;

struct camera_info_struct {
	int n_blocks, n_scans, mem_size, block_size_in_bytes;
	enum trigger_mode trigger_mode;
	pixel_t *data;
	struct dev_descr *dev;
};


void print_data(const struct camera_info_struct *info);

int camera_init() {
	int result;

	if ((result = lscpcie_driver_init()) < 0) {
		fprintf(stderr, "initialising driver returned %d\n", result);
		return result;
	}

	switch (result) {
	case 0:
		fprintf(stderr, "didn't find an lscpcie board\n");
		return 0;
	case 1:
		fprintf(stderr, "found one lscpcie board\n");
		break;
	case 2:
		fprintf(stderr, "found %d lscpcie boards\n", result);
		break;
	}

	// open /dev/lscpcie<n>
	if ((result = lscpcie_open(0, 0, USE_DMA_MAPPING)) < 0) {
		fprintf(stderr, "opening first board returned %d\n", result);
		return 2;
	}

	result = lscpcie_init_7030(0);
	if (result < 0) {
		fprintf(stderr, "error %d when initialising device\n", result);
		return result;
	}

	return 0;
}

int camera_release() {
	lscpcie_close(0);
	return 0;
}

/* common tasks to prepare hardware and memory for readout */
int readout_init(struct camera_info_struct *info) {
	int result;

	// get memory mapped pointers etc
	info->dev = lscpcie_get_descriptor(0);

	// clear dma buffer to avoid reading stuff from prev. debugging sessions
	fprintf(stderr, "clearing %d bytes of dma buffer\n",
		info->dev->control->dma_buf_size);
	memset((uint8_t *) info->dev->mapped_buffer, 0,
		info->dev->control->dma_buf_size);

	info->trigger_mode = xck;

	fprintf(stderr, "initialising registers\n");

	result = lscpcie_init_scan(info->dev, info->trigger_mode, info->n_scans,
				info->n_blocks, 2);
	if (result) {
		fprintf(stderr, "error %d when initialising scan\n", result);
		return result;
	}

	return 0;
}


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
	//lscpcie_dump_s0(dev);
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

int read_single_block(struct camera_info_struct *info) {
	int result, i;

	info->dev->s0->XCK.bytes.MSB &= 0xBF; // stop S Timer
	info->dev->s0->EC = 0; // reset SEC

	info->dev->s0->XCK.bytes.MSB |= (1<<XCKMSB_EXT_TRIGGER);

	result = lscpcie_start_scan(info->dev);
	if (result) {
		fprintf(stderr, "error %d when starting scan\n", result);
		return result;
	}

	if (no_acquisition) {
		lscpcie_dump_s0(info->dev);
		lscpcie_dump_dma(info->dev);
		lscpcie_dump_tlp(info->dev);
		lscpcie_close(0);
		exit(0);
	}

        for (i = 0; i < info->n_blocks; i++) {
		// wait for trigger signal
		if (!(info->dev->s0->CTRLA & (1 << CTRLA_TSTART)))
			continue;

		result = lscpcie_acquire_block_fs(info->dev,
						(uint8_t *) info->data
						+ i * info->block_size_in_bytes,
						info->n_scans,
						info->dev->handle);
		if (result < 0) {
			fprintf(stderr, "error %d when acquiring block\n",
				result);
			goto out;
		}
	}

	fprintf(stderr, "finished measurement\n");

	print_data(info);

      out:
	return result;
}

/** convenience stuff **/

int scan_command_line(int argc, char **argv, struct camera_info_struct *info) {
	int arg_pos = 0;

	while (++arg_pos < argc) {
		if (!strcmp(argv[1], "-n")) {
			no_acquisition = 1;
			continue;
		}
		if (!strcmp(argv[1], "--no-acquisition")) {
			no_acquisition = 1;
			continue;
		}
		break;
	}

	if (arg_pos > argc - 2) {
		fprintf(stderr,
	   "usage: test-readout-polling <number of scans> <number of blocks>\n");
		return -1;
	}

	info->n_scans = atoi(argv[arg_pos++]);
	info->n_blocks = atoi(argv[arg_pos++]);

	return arg_pos;
}

void print_data(const struct camera_info_struct *info) {
	int i = 0, block, scan, camera, pixel;
	int n_cams = info->dev->control->number_of_cameras;
	int n_pixel = info->dev->control->number_of_pixels;

        /*
	for (block = 0; block < info->n_blocks; block++)
		for (scan = 0; scan < info->n_scans; scan++)
			for (camera = 0; camera < n_cams; camera++)
				for (pixel = 0; pixel < n_pixel; pixel++, i++)
					printf("%d %d %d %d %d\n", block, scan,
					       camera, pixel, info->data[i]);
        */
        /*
	for (pixel = 0, i = 0; pixel < n_pixel; pixel++) {
		printf("%d", pixel);
		for (block = 0; block < info->n_blocks; block++)
			for (scan = 0; scan < info->n_scans; scan++)
				for (camera = 0; camera < n_cams; camera++, i++)
					printf("\t%hu", *(info->data + i));
		printf("\n");
	}
	printf("\n");
	*/
	int n = n_pixel * n_cams * info->n_scans * info->n_blocks;
	for (i = 0; i < n; i++)
	  printf("%d\t%hu\n", i, *(info->data + i));
}

int main(int argc, char **argv) {
	int i, n = atoi(argv[3]), result;
	struct camera_info_struct info;

        result = scan_command_line(argc, argv, &info);
	if (result < 0)
		return result;

	result = camera_init();
	if (result < 0)
		return result;

	result = readout_init(&info);
	if (result < 0)
		return result;

	info.block_size_in_bytes
          = sizeof(pixel_t) * info.dev->control->number_of_pixels
          * info.dev->control->number_of_cameras * info.n_scans;

	info.mem_size = info.block_size_in_bytes * info.n_blocks;

	info.data = malloc(info.mem_size);

	if (!info.data) {
		fprintf(stderr, "failed to allocate %d bytes of memory\n",
			info.mem_size);
		return -ENOMEM;
	}

	for (i = 0; i < n; i++)
		read_single_block(&info);

	result = lscpcie_end_acquire(info.dev);
	if (result)
		fprintf(stderr, "error %d when finishing acquisition\n", result);

	if (info.data)
		free(info.data);

	return camera_release();
}
