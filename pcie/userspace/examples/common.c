#include "common.h"
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

int init_7030(unsigned int dev_no)
{
	int result;
	struct dev_descr *dev = lscpcie_get_descriptor(dev_no);

	result = set_dma_address_in_tlp(dev);
	if (result < 0)
		return result;

	/* HAMAMATSU 7030-0906 	VFreq | 64 lines */
	dev->s0->VCLKCTRL = (0x700000 << 8) | 0x80;

	return 0;
}

/* common tasks to prepare hardware and memory for readout */
int readout_init(int argc, char **argv, struct camera_info_struct *info) {
	int no_acquisition = 0, result;

	if (argc != 3) {
		if ((argc != 4)
			||
			(strcmp(argv[1], "-n")
				&&
				strcmp(argv[1], "--no-acquisition"))) {
			fprintf(stderr,
	   "usage: test-readout-polling <number of scans> <number of blocks>\n");
			return 1;
		}

		no_acquisition = 1;
	}

	info->n_blocks = atoi(argv[1]);
	info->n_scans = atoi(argv[2]);

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
	if ((result = lscpcie_open(0, 0)) < 0) {
		fprintf(stderr, "opening first board returned %d\n", result);
		return 2;
	}
	// get memory mapped pointers etc
	info->dev = lscpcie_get_descriptor(0);

	// clear dma buffer to avoid reading stuff from prev. debugging sessions
	fprintf(stderr, "clearing %d bytes of dma buffer\n",
		info->dev->control->dma_buf_size);
	memset((uint8_t *) info->dev->mapped_buffer, 0,
		info->dev->control->dma_buf_size);

	info->trigger_mode = xck;

	result = init_7030(0);
	if (result < 0) {
		fprintf(stderr, "error %d when initialising device\n", result);
		return result;
	}

	fprintf(stderr, "initialising registers\n");

	result = lscpcie_init_scan(info->dev, info->trigger_mode, info->n_scans,
				info->n_blocks, 2);
	if (result) {
		fprintf(stderr, "error %d when initialising scan\n", result);
		return result;
	}

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

	info->mem_size = info->dev->control->number_of_pixels
		* info->dev->control->number_of_cameras * info->n_blocks
		* info->n_scans * sizeof(pixel_t);
	info->data = malloc(info->mem_size);
	if (!info->data) {
		fprintf(stderr, "failed to allocate %d bytes of memory\n",
			info->mem_size);
		return -ENOMEM;
	}

	return 0;
}

int fetch_data_mapped(struct dev_descr *dev, uint8_t *data, size_t max)
{
	int end_read = dev->control->write_pos, len;

	/*
	int i;
	for (i = 0; i < dev->control->dma_buf_size; i++)
        	printf("%d %d\n", i, *(dev->mapped_buffer + i));
	*/
	fprintf(stderr, "%d -> %d\n", dev->control->read_pos, end_read);
	if (end_read > dev->control->read_pos)
		/* new data in one chunk */
		len = end_read - dev->control->read_pos;
	else
		/* new data wraps around the end of the buffer */
		len = dev->control->dma_buf_size - dev->control->read_pos;

	if (len > max) len = max;
	memcpy(data, dev->mapped_buffer + dev->control->read_pos, len);

	if (end_read < dev->control->read_pos) {
		/* copy from buffer start */
		max -= len;
		if (end_read > max) end_read = max;
		if (end_read) {
			memcpy(data + len, dev->mapped_buffer, end_read);
			len += end_read;
		}
	}

	dev->control->read_pos
		= (dev->control->read_pos + len) % dev->control->dma_buf_size;

	return len;
}

void print_data(const struct camera_info_struct *info) {
	int i = 0, block, scan, camera, pixel;
	int n_cams = info->dev->control->number_of_cameras;
	int n_pixel = info->dev->control->number_of_pixels;

	for (block = 0; block < info->n_blocks; block++)
		for (scan = 0; scan < info->n_scans; scan++)
			for (camera = 0; camera < n_cams; camera++)
				for (pixel = 0; pixel < n_pixel; pixel++, i++)
					printf("%d %d %d %d: %d\n", block, scan,
					       camera, pixel, info->data[i]);
}

int kbhit(void)
{
	struct termios oldt, newt;
	int ch;
	int oldf;

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);

	if(ch == EOF)
		return 0;

	ungetc(ch, stdin);
	return 1;
}
