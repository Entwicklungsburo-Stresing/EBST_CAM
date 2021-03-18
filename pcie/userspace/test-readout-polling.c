#include "types.h"
#include "lscpcie.h"
#include "../kernelspace/registers.h"
#include "local-config.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <memory.h>

#define CFG_BTIMER_IN_US      500000
#define CFG_STIMER_IN_US      400

#define STAT_CTRL_MASK 0x0000FFFF
#define CTRLA_RW_MASK 0x3F
#define CTRLB_RW_MASK 0x3F
#define CTRLC_RW_MASK 0x06

#define memory_barrier() asm volatile ("" : : : "memory")

/* Performs a camera read with polling.
   After intitialising and starting the cameras, the read and write pointers
   in the control memory mapped from the driver are checked repeatedly and
   buffer contents are copied upon pointer change.
   The block trigger flag in CRTLA is polled ẗo detect block ends and start
   new block readings until the number of bytes copied from the mapped DMA
   buffer reaches the goal defined by the two command line arguments
   <number of scans> <number of blocks>.
*/

/* to be moved to lscpcie_open */
int lscpcie_init_device(uint dev_no)
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

int lscpcie_init_scan(dev_descr_t *dev, int trigger_mode)
{
	int result;
	uint32_t hwd_req = (1<<IRQ_REG_HWDREQ_EN);

	result = set_dma_buffer_registers(dev);
	if (result < 0)
		return result;

	result = lscpcie_send_fiber(0, MASTER_ADDRESS_CAMERA,
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

	dev->s0->DMAS_PER_INTERRUPT = 2;
	dev->control->bytes_per_interrupt
	    = dev->s0->DMAS_PER_INTERRUPT
	    * dev->control->number_of_pixels * 2;

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

int lscpcie_start_block(dev_descr_t * dev)
{
	// make pulse for blockindex counter
	dev->s0->PCIEFLAGS |= 1 << PCIEFLAG_BLOCKTRIG;
	memory_barrier();
	dev->s0->PCIEFLAGS &= ~(1 << PCIEFLAG_BLOCKTRIG);

	// reset scan counter
	dev->s0->SCAN_INDEX |= 1 << SCAN_INDEX_RESET;
	memory_barrier();
	dev->s0->SCAN_INDEX &= ~(1 << SCAN_INDEX_RESET);

	dev->s0->PCIEFLAGS |= 1 << PCIEFLAG_BLOCKON;
	// start Stimer -> set usecs and RS to one
	dev->s0->XCK.dword
	    = (dev->s0->XCK.dword & ~XCK_EC_MASK)
	    | (CFG_STIMER_IN_US & XCK_EC_MASK) | (1 << XCK_RS);

	// software trigger
	dev->s0->BTRIGREG |= 1 << FREQ_REG_SW_TRIG;
	memory_barrier();
	dev->s0->BTRIGREG &= ~(1 << FREQ_REG_SW_TRIG);

	return 0;
}

/* Rolls the read and write pointers in the mapped control memory and copies
   newly available bytes.
   Returns the number of copied bytes. */

int fetch_data(dev_descr_t *dev, uint8_t *data)
{
	int actual_write_pos = dev->control->write_pos, len;

	fprintf(stderr, "%d -> %d\n", dev->control->read_pos, actual_write_pos);
	if (actual_write_pos > dev->control->read_pos) {
		/* new data in one chunk */
		len = actual_write_pos - dev->control->read_pos;
		memcpy(data, dev->mapped_buffer + dev->control->read_pos, len);
		dev->control->read_pos += len;
		return len;
	}

	/* new data wraps around the end of the buffer */
	len = dev->control->dma_buf_size - dev->control->read_pos;
	memcpy(data, dev->mapped_buffer + dev->control->read_pos, len);

	memcpy(data + len, dev->mapped_buffer, actual_write_pos);
	dev->control->read_pos = actual_write_pos;

	return len + actual_write_pos;
}

int lscpcie_acquire_block(dev_descr_t *dev, uint8_t *data) {
	int result, bytes_read;

	result = lscpcie_start_block(dev);
	if (result) {
		fprintf(stderr, "error %d when starting block\n", result);
		return result;
	}

	while (dev->s0->XCK.dword & (1 << XCK_RS)) {
		if (dev->control->read_pos == dev->control->write_pos) continue;

		result = fetch_data(dev, data);
		if (result < 0)
			return result;

		bytes_read += result;
		fprintf(stderr, "got %d bytes of data, having now %d\n",
			result, bytes_read);
			dev->control->read_pos =
			    (dev->control->read_pos + result)
			    %
			    dev->control->dma_buf_size;
	} while (result);
	// reset block on
	dev->s0->PCIEFLAGS &=
	    ~(1 << PCIEFLAG_BLOCKON);

        return bytes_read;
}

int main(int argc, char **argv)
{
	if (argc != 3) {
		fprintf(stderr,
	   "usage: test-readout-polling <number of scans> <number of blocks>\n");
		return 1;
	}

	int n_blocks = atoi(argv[1]);
	int n_scans = atoi(argv[2]);
	int result, bytes_read, i, n_pixel, camcnt, mem_size,
	    prev_write_pos;
	int pixel, camera, scan, block;
	dev_descr_t *dev;
	uint16_t *camera_data = 0;
	trigger_mode_t trigger_mode;

	if ((result = lscpcie_driver_init()) < 0) {
		fprintf(stderr, "initialising driver returned %d\n", result);
		return 1;
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
	dev = lscpcie_get_descriptor(0);

	// clear dma buffer to avoid reading stuff from prev. debugging sessions
	fprintf(stderr, "clearing %d bytes of dma buffer\n",
		dev->control->dma_buf_size);
	memset((uint8_t *) dev->mapped_buffer, 0, dev->control->dma_buf_size);

	trigger_mode = xck;

	n_pixel = dev->control->number_of_pixels;
	camcnt = dev->control->number_of_cameras;

	mem_size = n_pixel * camcnt * n_blocks * n_scans * 2;
	camera_data = malloc(mem_size);
	if (!camera_data) {
		fprintf(stderr, "failed to allocate %d bytes of memory\n",
			mem_size);
		goto out_error;
	}

	/* already done in lscpcie_open
	   lscpcie_send_fiber(0, MASTER_ADDRESS_CAMERA, CAMERA_ADDRESS_PIXEL,
	   dev->control->number_of_pixels);
	 */
	// and what about this? not present in board.c
	// should go zo lscpcie_open or similar

	result = lscpcie_init_device(0);
	if (result < 0) {
		fprintf(stderr, "error %d when initialising device\n", result);
		goto out_error;
	}

	fprintf(stderr, "initialising registers\n");

	result = lscpcie_init_scan(dev, trigger_mode);
	if (result) {
		fprintf(stderr, "error %d when initialising scan\n", result);
		goto out_error;
	}

	result = lscpcie_start_scan(dev);
	if (result) {
		fprintf(stderr, "error %d when starting scan\n", result);
		goto out_error;
	}

	prev_write_pos = 0;
	bytes_read = 0;

	do {
		// wait for trigger signal
		if (dev->s0->CTRLA & (1 << CTRLA_TSTART))
			continue;

		result = lscpcie_acquire_block(dev, (uint8_t *) camera_data);
		if (result) {
			fprintf(stderr, "error %d when acquiring block\n",
				result);
			goto out_error;
		}
	} while (bytes_read < mem_size);

	fprintf(stderr, "finished measurement\n");

	dev->s0->XCK.dword &= ~(1 << XCK_RS);
	// stop btimer
	dev->s0->BTIMER &= ~(1 << BTIMER_START);
	// reset measure on
	dev->s0->PCIEFLAGS &= ~(1 << PCIEFLAG_MEASUREON);

	i = 0;
	for (block = 0; block < n_blocks; block++)
		for (scan = 0; scan < n_scans; scan++)
			for (camera = 0; camera < 2; camera++)
				for (pixel = 0; pixel < n_pixel; pixel++, i++)
					printf("%d %d %d %d: %d\n", block, scan,
						camera, pixel, camera_data[i]);

	fprintf(stderr, "write positions: %d %d\n", prev_write_pos,
		dev->control->write_pos);

      out_error:
	if (camera_data)
		free(camera_data);
	lscpcie_close(0);

	return 0;
}
