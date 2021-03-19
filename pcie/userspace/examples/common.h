#ifndef _lscpcie_examples_common_h_
#define _lscpcie_examples_common_h_


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

int lscpcie_init_device(unsigned int dev_no);
int lscpcie_init_scan(dev_descr_t *dev, int trigger_mode,
		int dmas_per_interrupt);
int lscpcie_start_scan(dev_descr_t * dev);
int lscpcie_start_block_soft(dev_descr_t *dev);
int lscpcie_end_block(dev_descr_t *dev);
int lscpcie_end_acquire(dev_descr_t *dev);

int readout_init(int argc, char **argv, struct camera_info_struct *info);
int fetch_data_mapped(dev_descr_t *dev, uint8_t *data, size_t max);
void print_data(const struct camera_info_struct *info);


#endif /* _lscpcie_examples_common_h_ */
