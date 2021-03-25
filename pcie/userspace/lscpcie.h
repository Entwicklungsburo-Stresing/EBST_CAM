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
#include "../kernelspace/ioctl.h"
#include "../kernelspace/registers.h"
#include <sys/types.h>


#ifdef __cplusplus
extern "C" {
#endif

struct dev_descr {
	int handle;	// file handle to /dev/lscpcie<n>
	int number_of_tlps;	// driver-internal
	int tlp_size;	// driver-internal
	s0_t *s0;	// memory mapped pointer to s0 pcie registers
	dma_reg_t *dma_reg;	// memory mapped pointer to dma pcie registers
	const uint8_t *mapped_buffer;	// memory mapped pointer dma buffer
	lscpcie_control_t *control;	// memory mapped pointer to driver control area
};

typedef enum { xck = 0, exttrig = 1, dat = 2 } trigger_mode_t;

// the following flags should go elsewhere, they don't need to be exposed
#define IS_FFT  0x0001
#define IS_AREA 0x8000
#define LEGACY_202_14_TLPCNT 0

#define MASTER_ADDRESS_CAMERA     0x00
#define CAMERA_ADDRESS_PIXEL      0x01
#define CAMERA_ADDRESS_TRIGGER_IN 0x02
#define CAMERA_ADDRESS_VCLK       0x04

#define SET_BITS(var, val, mask) var = (var & ~mask) | (val & mask)

// camera operations
int lscpcie_driver_init(void);
int lscpcie_open(uint dev_no, uint16_t options);
int lscpcie_setup_dma(struct dev_descr *dev);
void lscpcie_close(uint dev_no);
int lscpcie_start(struct dev_descr *dev);
int lscpcie_stop(struct dev_descr *dev);
ssize_t lscpcie_readout(struct dev_descr *dev, uint16_t * buffer,
			size_t items_to_read);
int lscpcie_hardware_present(struct dev_descr *dev);
int lscpcie_fifo_overflow(struct dev_descr *dev);
int lscpcie_clear_fifo(struct dev_descr *dev);
size_t lscpcie_bytes_free(struct dev_descr *dev);
size_t lscpcie_bytes_available(struct dev_descr *dev);
int lscpcie_send_fiber(struct dev_descr *device_descriptor,
		       uint8_t master_address,
		       uint8_t register_address, uint16_t data);

// register access through ioctl
int lscpcie_read_config32(struct dev_descr *dev, uint16_t address,
			  uint32_t * val);
int lscpcie_write_config32(struct dev_descr *dev, uint16_t address,
			   uint32_t val);
int lscpcie_read_reg8(struct dev_descr *dev, uint16_t address,
		      uint8_t * val);
int lscpcie_read_reg16(struct dev_descr *dev, uint16_t address,
		       uint16_t * val);
int lscpcie_read_reg32(struct dev_descr *dev, uint16_t address,
		       uint32_t * val);
int lscpcie_write_reg8(struct dev_descr *dev, uint16_t address,
		       uint8_t val);
int lscpcie_write_reg16(struct dev_descr *dev, uint16_t address,
			uint16_t val);
int lscpcie_write_reg32(struct dev_descr *dev, uint16_t address,
			uint32_t val);
int lscpcie_set_bits_reg8(struct dev_descr *dev, uint16_t address,
			  uint8_t bits, uint8_t mask);
int lscpcie_set_bits_reg16(struct dev_descr *dev, uint16_t address,
			   uint16_t bits, uint32_t mask);
int lscpcie_set_bits_reg32(struct dev_descr *dev, uint16_t address,
			   uint32_t bits, uint32_t mask);
struct dev_descr *lscpcie_get_descriptor(uint dev_no);

#define lscpcie_read_s0_8(dev, address, val) \
  lscpcie_read_reg8(dev, address + 0x80, val)
#define lscpcie_read_s0_16(dev, address, val) \
  lscpcie_read_reg16(dev, address + 0x80, val)
#define lscpcie_read_s0_32(dev, address, val) \
  lscpcie_read_reg32(dev, address + 0x80, val)
#define lscpcie_write_s0_8(dev, address, val) \
  lscpcie_write_reg8(dev, address + 0x80, val)
#define lscpcie_write_s0_16(dev, address, val) \
  lscpcie_write_reg16(dev, address + 0x80, val)
#define lscpcie_write_s0_32(dev, address, val) \
  lscpcie_write_reg32(dev, address + 0x80, val)
#define lscpcie_set_bits_s0_8(dev, address, bits, mask) \
  lscpcie_set_bits_reg8(dev, address + 0x80, bits, mask)
#define lscpcie_set_bits_s0_16(dev, address, bits, mask) \
  lscpcie_set_bits_reg16(dev, address + 0x80, bits, mask)
#define lscpcie_set_bits_s0_32(dev, address, bits, mask) \
  lscpcie_set_bits_reg32(dev, address + 0x80, bits, mask)

#define lscpcie_read_dma_8      lscpcie_read_reg8
#define lscpcie_read_dma_16     lscpcie_read_reg16
#define lscpcie_read_dma_32     lscpcie_read_reg32
#define lscpcie_write_dma_8     lscpcie_write_reg8
#define lscpcie_write_dma_16    lscpcie_write_reg16
#define lscpcie_write_dma_32    lscpcie_write_reg32
#define lscpcie_set_bits_dma_8  lscpcie_set_bits_reg8
#define lscpcie_set_bits_dma_16 lscpcie_set_bits_reg16
#define lscpcie_set_bits_dma_32 lscpcie_set_bits_reg32

#define lscpcie_set_s0_bit(dev, address, bit_number, set) \
  lscpcie_set_bits_reg32(dev, address, set ? mask : 0, mask)


// debugging
int lscpcie_set_debug(struct dev_descr *dev, int flags, int mask);
int lscpcie_get_buffer_pointers(struct dev_descr *dev,
				uint64_t * pointers);
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
