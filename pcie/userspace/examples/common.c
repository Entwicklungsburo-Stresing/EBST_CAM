#include "local-config.h"
#include "lscpcie.h"
#include "common.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <errno.h>

#define memory_barrier() asm volatile ("" : : : "memory")

/*
#include "types.h"
#include "../kernelspace/registers.h"
#include <stdlib.h>
#include <memory.h>
#include <errno.h>

#define STAT_CTRL_MASK 0x0000FFFF
#define CTRLA_RW_MASK 0x3F
#define CTRLB_RW_MASK 0x3F
#define CTRLC_RW_MASK 0x06


#define pulse_bit(reg, bit_mask) do { \
		dev->s0->reg |= bit_mask; \
		memory_barrier(); \
		dev->s0->reg &= ~(bit_mask);	\
	} while (0)
*/


/* to be moved to lscpcie_open */
int lscpcie_init_device(unsigned int dev_no)
{
	int result;
	dev_descr_t *dev = lscpcie_get_descriptor(dev_no);

	result = set_dma_address_in_tlp(dev);
	if (result < 0)
		return result;

	/* HAMAMATSU 7030-0906 	VFreq | 64 lines */
	dev->s0->VCLKCTRL = (0x700000 << 8) | 0x80;

	return 0;
}

int lscpcie_init_scan(dev_descr_t *dev, int trigger_mode, int dmas_per_interrupt)
{
	int result;
	uint32_t hwd_req = (1<<IRQ_REG_HWDREQ_EN);

	result = set_dma_buffer_registers(dev);
	if (result < 0)
		return result;

	result = lscpcie_send_fiber(dev, MASTER_ADDRESS_CAMERA,
				    CAMERA_ADDRESS_TRIGGER_IN,
				    trigger_mode);
	if (result < 0)
		return result;

	if (HWDREQ_EN)
		SET_BITS(dev->s0->IRQ.IRQREG, hwd_req, hwd_req);
	else
		SET_BITS(dev->s0->IRQ.IRQREG, 0, hwd_req);

	// set trigger mode to block timer and scan timer + shutter not on
	dev->s0->CTRLB
	    = (CTRLB_BTI_TIMER | CTRLB_STI_TIMER) & ~(CTRLB_SHON);
	// set block timer and start block timer
	dev->s0->BTIMER =
	    (1 << BTIMER_START) | CFG_BTIMER_IN_US;
	// set slope of block trigger
	dev->s0->BFLAGS |= 1 << BFLAG_BSLOPE;

	// set output of O on PCIe card
	dev->s0->TOR = TOR_OUT_XCK;

	dev->s0->DMAS_PER_INTERRUPT = dmas_per_interrupt;
	dev->control->bytes_per_interrupt
	    = dmas_per_interrupt * dev->control->number_of_pixels
		* sizeof(pixel_t);

	return result;
}

int lscpcie_start_scan(dev_descr_t * dev)
{
	dev->s0->DMAS_PER_INTERRUPT |=
	    1 << DMA_COUNTER_RESET;
	memory_barrier();
	dev->s0->DMAS_PER_INTERRUPT &=
	    ~(1 << DMA_COUNTER_RESET);

	// reset the internal block counter - is not BLOCKINDEX!
	dev->s0->DMA_BUF_SIZE_IN_SCANS |=
	    1 << BLOCK_COUNTER_RESET;
	memory_barrier();
	dev->s0->DMA_BUF_SIZE_IN_SCANS &=
	    ~(1 << BLOCK_COUNTER_RESET);

	// reset block counter
	dev->s0->BLOCK_INDEX |= 1 << BLOCK_INDEX_RESET;
	memory_barrier();
	dev->s0->BLOCK_INDEX &= ~(1 << BLOCK_INDEX_RESET);

	// set Block end stops timer:
	// when SCANINDEX reaches NOS, the timer is stopped by hardware.
	dev->s0->PCIEFLAGS |= 1 << PCIE_EN_RS_TIMER_HW;
	////<<<< reset all counters

	// >> SetIntFFTrig
	dev->s0->XCK.dword &= ~(1 << XCKMSB_EXT_TRIGGER);
	dev->control->write_pos = 0;
	dev->control->read_pos = 0;
	// set measure on
	fprintf(stderr, "starting measurement\n");
	dev->s0->PCIEFLAGS |= 1 << PCIEFLAG_MEASUREON;

	return 0;
}

int lscpcie_start_block_soft(dev_descr_t *dev) {
	// make pulse for blockindex counter
	pulse_bit(PCIEFLAGS, 1<<PCIEFLAG_BLOCKTRIG);
	// reset scan counter
	pulse_bit(SCAN_INDEX, 1<<SCAN_INDEX_RESET);
	dev->s0->PCIEFLAGS |= 1<<PCIEFLAG_BLOCKON;
	// start Stimer -> set usecs and RS to one
	dev->s0->XCK.dword
	    = (dev->s0->XCK.dword & ~XCK_EC_MASK)
	    | (CFG_STIMER_IN_US & XCK_EC_MASK) | (1 << XCK_RS);
	// software trigger
	pulse_bit(BTRIGREG, 1<<FREQ_REG_SW_TRIG);

	return 0;
}

int lscpcie_end_block(dev_descr_t *dev) {
	// reset block on
	dev->s0->PCIEFLAGS &= ~(1 << PCIEFLAG_BLOCKON);

	return 0;
}

int lscpcie_end_acquire(dev_descr_t *dev) {
	dev->s0->XCK.dword &= ~(1 << XCK_RS);
	// stop btimer
	dev->s0->BTIMER &= ~(1 << BTIMER_START);
	// reset measure on
	dev->s0->PCIEFLAGS &= ~(1 << PCIEFLAG_MEASUREON);

	return 0;
}

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

	result = lscpcie_init_device(0);
	if (result < 0) {
		fprintf(stderr, "error %d when initialising device\n", result);
		return result;
	}

	fprintf(stderr, "initialising registers\n");

	result = lscpcie_init_scan(info->dev, info->trigger_mode, 2);
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

int fetch_data_mapped(dev_descr_t *dev, uint8_t *data, size_t max)
{
	int end_read = dev->control->write_pos, len;

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
