#include "to-library.h"
#include "../../kernelspace/registers-common.h"
#include <stdio.h>

/* commands which do not depend on the camera type but access the hardware by
   register reads and writes */

#define memory_barrier() asm volatile ("" : : : "memory")

/* prepare registers for individual scan */
int lscpcie_init_scan(struct dev_descr *dev, int trigger_mode,
		int number_of_scans, int number_of_blocks,
		int dmas_per_interrupt)
{
	int result;
	uint32_t hwd_req = (1<<IRQ_REG_HWDREQ_EN);

	//result = set_dma_buffer_registers(dev);
	//if (result < 0)
	//	return result;

	result = lscpcie_send_fiber(dev, MASTER_ADDRESS_CAMERA,
				    CAMERA_ADDRESS_TRIGGER_IN,
				    trigger_mode);
	if (result < 0)
		return result;

	if (HWDREQ_EN)
		SET_BITS(dev->s0->IRQREG.REG32, hwd_req, hwd_req);
	else
		SET_BITS(dev->s0->IRQREG.REG32, 0, hwd_req);

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

	dev->s0->NUMBER_OF_SCANS = number_of_scans * number_of_blocks;
	dev->s0->DMA_BUF_SIZE_IN_SCANS = number_of_scans * number_of_blocks * 2;

	return result;
}

/* reset and start counters */
int lscpcie_start_scan(struct dev_descr * dev)
{
	pulse_bit(DMAS_PER_INTERRUPT, 1<<DMA_COUNTER_RESET);

	// reset the internal block counter - is not BLOCKINDEX!
	pulse_bit(DMA_BUF_SIZE_IN_SCANS, 1<<BLOCK_COUNTER_RESET);

	// reset block counter
	pulse_bit(BLOCK_INDEX, 1<<BLOCK_INDEX_RESET);

	// reset scan counter
	pulse_bit(SCAN_INDEX, 1<<SCAN_INDEX_RESET);

	// set Block end stops timer:
	// when SCANINDEX reaches NOS, the timer is stopped by hardware.
	dev->s0->PCIEFLAGS |= 1 << PCIE_EN_RS_TIMER_HW;
	////<<<< reset all counters

	// >> SetIntFFTrig
	dev->s0->XCK.dword &= ~(1 << XCKMSB_EXT_TRIGGER);
	dev->control->write_pos = 0;
	dev->control->read_pos = 0;
	dev->control->irq_count = 0;
	// set measure on
	fprintf(stderr, "starting measurement\n");
	dev->s0->PCIEFLAGS |= 1 << PCIEFLAG_MEASUREON;

	return 0;
}

int lscpcie_start_block(struct dev_descr *dev) {
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

int lscpcie_end_block(struct dev_descr *dev) {
	// reset block on
	dev->s0->PCIEFLAGS &= ~(1 << PCIEFLAG_BLOCKON);

	return 0;
}

int lscpcie_end_acquire(struct dev_descr *dev) {
	dev->s0->XCK.dword &= ~(1 << XCK_RS);
	// stop btimer
	dev->s0->BTIMER &= ~(1 << BTIMER_START);
	// reset measure on
	dev->s0->PCIEFLAGS &= ~(1 << PCIEFLAG_MEASUREON);

	return 0;
}
