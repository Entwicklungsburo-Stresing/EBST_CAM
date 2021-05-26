/* lscpcie.c
 *
 * Copyright 2020-2021 Bernhard Lang, University of Geneva
 * Copyright 2020-2021 Entwicklungsbuero Stresing (http://www.stresing.de/)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "lscpcie.h"
#include "constants.h"
#include "types.h"
#include "../kernelspace/registers.h"
#include "../kernelspace/ioctl.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdarg.h>
#include "../../shared_src/enum.h"
#include "local-config.h"

#define memory_barrier() asm volatile ("" : : : "memory")

static int error_reporting = 1;

static const struct dev_descr init_dev_descr = {
	.handle = -1,
	.number_of_tlps = 0,
	.dma_reg = 0
};

static struct dev_descr dev_descr[MAX_PCIE_BOARDS];
static int number_of_pcie_boards = -1;


void set_error_reporting(int level)
{
	error_reporting = level;
}

void error_message(const char *format, ...)
{
	if (!error_reporting)
		return;
	va_list ap;
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
}

struct dev_descr *lscpcie_get_descriptor(uint dev)
{
	return dev < number_of_pcie_boards ? dev_descr + dev : NULL;
}

int lscpcie_driver_init(void)
{
	int result, handle, i;

	handle = open("/dev/lscpcie0", O_RDWR);
	if (handle < 0)
		return -errno;	// failed to open device zero

	result =
	    ioctl(handle, LSCPCIE_IOCTL_NUM_BOARDS,
		  &number_of_pcie_boards);
	close(handle);

	if (result < 0) {
		number_of_pcie_boards = -1;
		return result;
	}

	for (i = 0; i < number_of_pcie_boards; i++)
		dev_descr[i] = init_dev_descr;

	return number_of_pcie_boards;
}

int lscpcie_open(uint dev_no, uint16_t fiber_options, uint8_t memory_options)
{
	int result = 0, handle, no_tlps, hardware_present;
	char name[16];
	int page_size = sysconf(_SC_PAGE_SIZE);

	if (dev_no >= number_of_pcie_boards)
		return -ENODEV;
	if (dev_descr[dev_no].handle >= 0)
		return -EBUSY;
	struct dev_descr *dev = &dev_descr[dev_no];

	sprintf(name, "/dev/lscpcie%d", dev_no);
	handle = open(name, O_RDWR);
	if ((handle < 0) && error_reporting) {
		error_message("error on opening '%s': ", name);
		if (error_reporting)
			perror(0);
		return handle;
	}

	dev->handle = handle;

	// dma control struct in ram
	dev->control
	    =
	    mmap(NULL, sizeof(struct control_struct), PROT_READ | PROT_WRITE,
		 MAP_SHARED, handle, page_size);

	if (dev->control == MAP_FAILED) {
		error_message("mmap on dma control ram failed\n");
		if (error_reporting)
			perror(0);
		goto error;
	}

	hardware_present = lscpcie_hardware_present(dev);
	if (hardware_present < 0)
		goto error;

	fprintf(stderr, "found io spce of size 0x%08x\n", dev->control->io_size);
	if (hardware_present)
	{
		// map io registers to user space memory
		dev->dma_reg
		    =
		    mmap(NULL, dev->control->io_size,
			 PROT_READ | PROT_WRITE, MAP_SHARED, handle, 0);

		if (dev->dma_reg == MAP_FAILED) {
			error_message("mmap on io memory failed\n");
			if (error_reporting)
				perror(0);
			result = errno;
			goto error;
		}
		// s0 addres space pointer
		dev->s0 = (struct s0_reg_struct *)
			(((uint8_t *) dev->dma_reg) + 0x80);
	}
	else
	{
		/* no hardware found, debug mode */
		dev->dma_reg = NULL;
		dev->s0 = NULL;
		dev->control->used_dma_size = dev->control->dma_buf_size;
	}
	if (memory_options & USE_DMA_MAPPING) {
		dev->mapped_buffer
			= mmap(NULL, dev->control->dma_buf_size,
				PROT_READ | PROT_WRITE, MAP_SHARED, handle,
				2 * page_size);
		if (dev->mapped_buffer == MAP_FAILED)
			return -1;
	} else {
		dev->mapped_buffer = 0;
	}
	return handle;

      error:
	lscpcie_close(dev_no);
	return result;
}

void lscpcie_close(uint dev_no)
{
	struct dev_descr *dev = lscpcie_get_descriptor(dev_no);
	if (!dev)
		return;

	if (dev->dma_reg != MAP_FAILED)
		munmap(dev->dma_reg, 0x100);

	if (dev->control != MAP_FAILED)
		munmap(dev->control, sizeof(struct control_struct));

	if (dev->handle >= 0)
		close(dev->handle);

	dev_descr[dev_no] = init_dev_descr;
}

ssize_t lscpcie_readout(struct dev_descr *dev, uint16_t * buffer,
			size_t items_to_read)
{
	ssize_t result;
	size_t bytes_read = 0, bytes_to_read = items_to_read * 2;

	while (bytes_read < bytes_to_read) {
		result =
		    read(dev->handle, buffer + bytes_read / 2,
			 bytes_to_read);
		if (result > 0) {
			bytes_read += result;
			bytes_to_read -= result;
		} else if (result < 0) {
			fprintf(stderr,
				"read ccd returned %ld, error code %d\n",
				result, errno);
			return result;
		}
	}

	return bytes_read / 2;
}

size_t lscpcie_bytes_available(struct dev_descr *dev) {
	ssize_t diff = dev->control->write_pos - dev->control->read_pos;
	if (diff < 0) return diff += dev->control->used_dma_size;
	return diff;
}

size_t lscpcie_bytes_free(struct dev_descr *dev) {
	ssize_t diff = dev->control->read_pos - dev->control->write_pos;
	if (diff <= 0)
		diff += dev->control->used_dma_size;
	return diff - 1;
}

int lscpcie_fifo_overflow(struct dev_descr *dev) {
	return dev->control->status & DEV_FIFO_OVERFLOW;
}

int lscpcie_dma_overflow(struct dev_descr *dev) {
	return dev->control->status & DEV_DMA_OVERFLOW;
}

void lscpcie_set_debug(struct dev_descr *dev, int flags, int mask)
{
	dev->control->debug_mode = (dev->control->debug_mode & ~mask) | flags;
}


/*******************************************************************************/
/*                                 fiber link                                  */
/*******************************************************************************/

int lscpcie_hardware_present(struct dev_descr *dev) {
	return dev->control->status & DEV_HARDWARE_PRESENT;
}

uint32_t get_scan_index(struct dev_descr *dev)
{
	return dev->s0->SCAN_INDEX;
}

/*******************************************************************************/
/*                              register access                                */
/*******************************************************************************/

int lscpcie_read_config32(struct dev_descr *dev, uint16_t address,
			  uint32_t * val)
{
	int result;
	reg_info_t reg = {.address = address };

	if ((result =
	     ioctl(dev->handle, LSCPCIE_IOCTL_GET_CONF, &reg)) < 0)
		return result;
	*val = reg.value;

	return 0;
}

int lscpcie_write_config32(struct dev_descr *dev, uint16_t address,
			   uint32_t val)
{
	reg_info_t reg = {.address = address,.value = val };


	return ioctl(dev->handle, LSCPCIE_IOCTL_SET_CONF, &reg);
}

/*******************************************************************************/
/*                                 debugging                                   */
/*******************************************************************************/

int lscpcie_dump_s0(struct dev_descr *dev)
{
	int i;
	enum N { number_of_registers = 41 };
	char register_names[number_of_registers][30] = {
		"DBR \t\t",
		"CTRLA \t\t",
		"XCKLL \t\t",
		"XCKCNTLL\t",
		"PIXREG \t\t",
		"FIFOCNT \t",
		"VCLKCTRL\t",
		"'EBST' \t\t",
		"DAT \t\t",
		"EC \t\t",
		"TOR \t\t",
		"ARREG \t\t",
		"GIOREG \t\t",
		"nc\t\t",
		"IRQREG\t\t",
		"PCI board version",
		"R0 PCIEFLAGS\t",
		"R1 NOS\t\t",
		"R2 SCANINDEX\t",
		"R3 DMABUFSIZE\t",
		"R4 DMASPERINTR\t",
		"R5 BLOCKS\t",
		"R6 BLOCKINDEX\t",
		"R7 CAMCNT\t",
		"R8 GPX Ctrl\t",
		"R9 GPX Data\t",
		"R10 ROI 0\t",
		"R11 ROI 1\t",
		"R12 ROI 2\t",
		"R13 XCKDLY\t",
		"R14 ADSC\t",
		"R15 LDSC\t",
		"R16 BTimer\t",
		"R17 BDAT\t",
		"R18 BEC\t\t",
		"R19 BFLAGS\t",
		"R20 TR1\t\t",
		"R21 TR2\t\t",
		"R22 TR3\t\t",
		"R23 TR4\t\t",
		"R24 TR5\t\t"
	};			//Look-Up-Table for the S0 Registers

	printf("S0- registers   \n");

	for (i = 0; i < number_of_registers; i++) {
		printf("%s \t: 0x%08x\n", register_names[i],
			*(uint32_t*) (((const uint8_t *) dev->s0) + i * 4));
	}
	return lscpcie_dump_tlp(dev);
}

int lscpcie_dump_dma(struct dev_descr *dev)
{
	enum N { number_of_registers = 18 };
	char register_names[number_of_registers][20] = {
		"DCSR\t",
		"DDMACR\t",
		"WDMATLPA",
		"WDMATLPS",
		"WDMATLPC",
		"WDMATLPP",
		"RDMATLPP",
		"RDMATLPA",
		"RDMATLPS",
		"RDMATLPC",
		"WDMAPERF",
		"RDMAPERF",
		"RDMASTAT",
		"NRDCOMP\t",
		"RCOMPDSIZW",
		"DLWSTAT\t",
		"DLTRSSTAT",
		"DMISCCONT"
	};			//Look-Up-Table for the DMA Registers
	printf("DMA registers\n");
	for (int i = 0; i < number_of_registers; i++) {
		printf("%s \t: 0x%08x\n", register_names[i],
			*(uint32_t*) (((const uint8_t *) dev->dma_reg) + i * 4));
	}
	return 0;
}

int lscpcie_dump_tlp(struct dev_descr *dev)
{
	uint32_t data, actpayload;

	printf
	    ("PAY_LOAD values : 0 = 128 bytes, 1 = 256 bytes, 2 = 512 bytes\n");
	lscpcie_read_config32(dev, PCIeAddr_devCap, &data);
	printf("PAY_LOAD Supported : 0x%x\n", data & 0x7);

	lscpcie_read_config32(dev, PCIeAddr_devStatCtrl, &data);
	actpayload = (data >> 5) & 0x07;
	printf("PAY_LOAD : 0x%x\n", actpayload);

	lscpcie_read_config32(dev, PCIeAddr_devStatCtrl, &data);
	printf("MAX_READ_REQUEST_SIZE : 0x%x\n\n", (data >> 12) & 0x7);


	printf("number of pixels: %d \n", dev->control->number_of_pixels);

	switch (actpayload) {
	case 0:
		data = 0x20;
		break;
	case 1:
		data = 0x40;
		break;
	case 2:
		data = 0x80;
		break;
	case 3:
		data = 0x100;
		break;
	}

	printf("TLP_SIZE is: %d DWORDs = %d BYTEs\n", data, data * 4);

	printf("TLPS in DMAReg is: %d \n", dev->dma_reg->WDMATLPS);

	data = (dev->control->number_of_pixels - 1) / (data * 2) + 1 + 1;
	printf("number of TLPs should be: %d\n", data);
	printf("number of TLPs is: %d \n", dev->dma_reg->WDMATLPC);

	return 0;
}
