/* lscpci.h
 *
 * Copyright 2020-2021 Bernhard Lang, University of Geneva
 * Copyright 2020-2021 Entwicklungsbuero Stresing (http://www.stresing.de/)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _lscpcie_h_
#define _lscpcie_h_


#define uconst const

#include "types.h"
#include "../kernelspace/registers-common.h"
#include <sys/types.h>


#ifdef __cplusplus
extern "C" {
#endif

struct dev_descr {
	int handle;	          // file handle to /dev/lscpcie<n>
	int number_of_tlps;	  // driver-internal
	int tlp_size;	          // driver-internal
	struct s0_reg_struct *s0; // memory mapped pointer to s0 pcie registers
	struct dma_reg_struct *dma_reg;	// memory mapped pointer to dma pcie registers
	const uint8_t *mapped_buffer;	// memory mapped pointer dma buffer
	struct control_struct *control;	// memory mapped pointer to driver control area
};

typedef enum { trig_xck = 0, trig_ext = 1, trig_dat = 2 } trigger_mode_t;

// the following flags should go elsewhere, they don't need to be exposed
#define IS_FFT  0x0001
#define IS_AREA 0x8000
#define LEGACY_202_14_TLPCNT_LSC 0

#define MASTER_ADDRESS_CAMERA     0x00
#define CAMERA_ADDRESS_PIXEL      0x01
#define CAMERA_ADDRESS_TRIGGER_IN 0x02
#define CAMERA_ADDRESS_VCLK       0x04

#define USE_DMA_MAPPING 0x01

#define SET_BITS(var, val, mask) var = (var & ~mask) | (val & mask)

// camera operations
int lscpcie_driver_init(void);
int lscpcie_open(uint dev_no, uint16_t fiber_options, uint8_t memory_options);
void lscpcie_close(uint dev_no);
int lscpcie_init_scan(struct dev_descr *dev, int trigger_mode,
		int number_of_scans, int number_of_blocks,
		int dmas_per_interrupt);
int lscpcie_start_scan(struct dev_descr * dev);
int lscpcie_start_block(struct dev_descr *dev);
int lscpcie_end_block(struct dev_descr *dev);
int lscpcie_end_acquire(struct dev_descr *dev);
ssize_t lscpcie_readout(struct dev_descr *dev, uint16_t * buffer,
			size_t items_to_read);
int lscpcie_setup_dma(struct dev_descr *dev);
int lscpcie_hardware_present(struct dev_descr *dev);
int lscpcie_fifo_overflow(struct dev_descr *dev);
int lscpcie_dma_overflow(struct dev_descr *dev);
int lscpcie_clear_fifo(struct dev_descr *dev);
size_t lscpcie_bytes_free(struct dev_descr *dev);
size_t lscpcie_bytes_available(struct dev_descr *dev);
int lscpcie_send_fiber(struct dev_descr *device_descriptor,
		       uint8_t master_address,
		       uint8_t register_address, uint16_t data);

int lscpcie_init_7030(unsigned int dev_no);

// register access through ioctl
int lscpcie_read_config32(struct dev_descr *dev, uint16_t address,
			  uint32_t * val);
int lscpcie_write_config32(struct dev_descr *dev, uint16_t address,
			   uint32_t val);
struct dev_descr *lscpcie_get_descriptor(uint dev_no);

#define lscpcie_set_s0_bit(dev, address, bit_number, set) \
  lscpcie_set_bits_reg32(dev, address, set ? mask : 0, mask)


// debugging
void lscpcie_set_debug(struct dev_descr *dev, int flags, int mask);
int lscpcie_dump_s0(struct dev_descr *dev);
int lscpcie_dump_dma(struct dev_descr *dev);
int lscpcie_dump_tlp(struct dev_descr *dev);


// additional
int set_dma_address_in_tlp(struct dev_descr *dev);
int set_dma_buffer_registers(struct dev_descr *dev);

#ifdef __cplusplus
}
#endif
#endif /* _lscpcie_h_ */
