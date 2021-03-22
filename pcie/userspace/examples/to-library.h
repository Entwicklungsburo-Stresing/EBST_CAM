#ifndef _lscpcie_to_library_h_
#define _lscpcie_to_library_h_


#include "local-config.h"
#include "lscpcie.h"
#include <stdint.h>


#define memory_barrier() asm volatile ("" : : : "memory")

#define pulse_bit(reg, bit_mask) do { \
		dev->s0->reg |= bit_mask; \
		memory_barrier(); \
		dev->s0->reg &= ~(bit_mask);	\
	} while (0)


struct camera_info_struct {
	int n_blocks, n_scans, mem_size;
	trigger_mode_t trigger_mode;
	pixel_t *data;
	dev_descr_t *dev;
};

int lscpcie_init_scan(dev_descr_t *dev, int trigger_mode,
		int number_of_scans, int number_of_blocks,
		int dmas_per_interrupt);
int lscpcie_start_scan(dev_descr_t * dev);
int lscpcie_start_block(dev_descr_t *dev);
int lscpcie_end_block(dev_descr_t *dev);
int lscpcie_end_acquire(dev_descr_t *dev);


#endif /* _lscpcie_to_library_h_ */
