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

#include "local-config.h"

/*
#define DMA_HW_BUFPARTS 2
#define DMA_DMASPERINTR DMA_BUFSIZEINSCANS / DMA_HW_BUFPARTS
*/


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
		return handle;	// failed to open device zero

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

/*Pixelsize with matching TLP Count (TLPC).
  Pixelsize = TLPS *TLPC - 1*TLPS
  (TLPS TLP size = 64)
  TLPC 0x Pixelsize
    1   64
    2	  128
    3	  192
    4	  256
    5	  320
    6	  384
    7	  448
    8	  512
    9	  576
    a	  640
    b	  704
    c	  768
    d	  832
    e	  896
    f		960
    10	1024
    11	1088
    12	1152
    13	1216
    14	1280
    15	1344
    16	1408
    17	1472
    18	1536
    19	1600
    1a	1664
    1b	1728
    1c	1792
    1d	1856
    1e	1920
    1f	1984
    20	2048
    21	2112
    22  2176
    ...
    40  4096
    41  4160
    ...
    81  8256
*/

int lscpcie_open(uint dev_no, uint16_t options)
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
	hardware_present = lscpcie_hardware_present(dev);
	if (hardware_present < 0)
		goto error;

	// dma control struct in ram
	dev->control
	    =
	    mmap(NULL, sizeof(lscpcie_control_t), PROT_READ | PROT_WRITE,
		 MAP_SHARED, handle, page_size);

	if (dev->control == MAP_FAILED) {
		error_message("mmap on dma control ram failed\n");
		if (error_reporting)
			perror(0);
		goto error;
	}

	switch (dev->control->number_of_pixels) {
	case 128:
		no_tlps = 0x2;
		break;
	case 192:
		no_tlps = 0x3;
		break;
	case 320:
		no_tlps = 0x5;
		break;
	case 576:
		no_tlps = 0x9;
		break;
	case 1088:
		no_tlps = 0x11;
		break;
	case 2112:
		no_tlps = 0x21;
		break;
	case 4160:
		no_tlps = 0x41;
		break;
	case 8256:
		no_tlps = 0x81;
		break;
	default:
		error_message("invalid number of pixels %d\n",
			      dev->control->number_of_pixels);
		return -EINVAL;
	}
	if (LEGACY_202_14_TLPCNT)
		no_tlps++;
	dev->number_of_tlps = no_tlps;

	printf("found io spce of size 0x%08x\n", dev->control->io_size);
	if (hardware_present) {
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
		dev->s0 = (s0_t *) (((uint8_t *) dev->dma_reg) + 0x80);

		// startval for CTRLA Reg  +slope, IFC=h, VON=1
		// clear CTRLB & CTRLC

		// note: the above mmap system call tells the operating system
		// to perform a write to pci io-registers when writing to the
		// corresponding virtual pointer, pretty cool, isn't it?
		dev->s0->CTRLA = 0x23;
		dev->s0->CTRLB = 0;
		dev->s0->CTRLC = 0;
		dev->s0->PIXREG = dev->control->number_of_pixels;
		dev->s0->CAM_CNT = dev->control->number_of_cameras & 0xF;

		// initialise number of pixels and clock scheme
		result
		    =
		    lscpcie_send_fiber(dev, MASTER_ADDRESS_CAMERA,
				       CAMERA_ADDRESS_PIXEL,
				       dev->control->number_of_pixels);
		if (result < 0)
			goto error;

		result
		    =
		    lscpcie_send_fiber(dev, MASTER_ADDRESS_CAMERA,
				       CAMERA_ADDRESS_VCLK, options);
		if (result < 0)
			goto error;
	} else {
		/* no hardware found, debug mode */
		dev->dma_reg = NULL;
		dev->s0 = NULL;
	}

	dev->mapped_buffer
	    = mmap(NULL, dev->control->dma_buf_size,
		   PROT_READ | PROT_WRITE, MAP_SHARED, handle,
		   2 * page_size);
	if (dev->mapped_buffer == MAP_FAILED)
		return -1;

	//if (_COOLER) ActCooling(drvno, FALSE); //deactivate cooler

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
		munmap(dev->control, sizeof(lscpcie_control_t));

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

void lscpcie_set_debug(struct dev_descr *dev, int flags, int mask)
{
	dev->control->debug_mode = (dev->control->debug_mode & ~mask) | flags;
}


/*******************************************************************************/
/*                                 fiber link                                  */
/*******************************************************************************/

int lscpcie_send_fiber(struct dev_descr *dev, uint8_t master_address,
		       uint8_t register_address, uint16_t data)
{
	uint32_t reg_val =
	    (master_address << 24) | (register_address << 16) | data;

	if (!dev->s0)
		return -ENODEV;

	dev->s0->DBR = reg_val;
	memory_barrier();
	dev->s0->DBR = reg_val | 0x4000000;
	memory_barrier();
	dev->s0->DBR = 0;
	usleep(1000);

	return 0;
}

int init_cam_control(struct dev_descr *dev, trigger_mode_t trigger_mode,
		     uint16_t options)
{
	int result;

	result
	    =
	    lscpcie_send_fiber(dev, MASTER_ADDRESS_CAMERA,
			       CAMERA_ADDRESS_PIXEL,
			       dev->control->number_of_pixels);
	if (result < 0)
		return result;

	result
	    =
	    lscpcie_send_fiber(dev, MASTER_ADDRESS_CAMERA,
			       CAMERA_ADDRESS_TRIGGER_IN, trigger_mode);
	if (result < 0)
		return result;

	result = lscpcie_send_fiber(dev, MASTER_ADDRESS_CAMERA,
				CAMERA_ADDRESS_PIXEL, options);	//??

	return result;
}

int lscpcie_hardware_present(struct dev_descr *dev) {
	return dev->control->status & DEV_HARDWARE_PRESENT;
}

/*******************************************************************************/
/*                                    dma                                      */
/*******************************************************************************/

int set_dma_address_in_tlp(struct dev_descr *dev)
{
	int result;
	uint64_t val64;
	uint32_t data = 0, tlp_mode;

	if (_FORCETLPS128)
		tlp_mode = 0;
	else {
		if ((result =
		     lscpcie_read_config32(dev, PCIeAddr_devCap,
					   &data)) < 0)
			return result;

		tlp_mode = data & 0x7;
	}

	data = (data & ~0xE0) | (1 << 13);

	switch (tlp_mode) {
	case 0:
		dev->tlp_size = 0x20;
		break;
	case 1:
		dev->tlp_size = 0x40;
		break;
	case 2:
		dev->tlp_size = 0x80;
		break;
	}

	data |= dev->tlp_size / 0x20;
	if ((result =
	     lscpcie_write_config32(dev, PCIeAddr_devStatCtrl, data)) < 0)
		return result;

	// WDMATLPA (Reg name): write the lower part (bit 02:31) of the DMA
	// adress to the DMA controller
	SET_BITS(dev->dma_reg->WDMATLPA,
		 (uint64_t) dev->control->dma_physical_start, 0xFFFFFFFC);
	printf("set WDMATLPA to physical address of dma buffer 0x%016lx\n",
	       (uint64_t) dev->control->dma_physical_start);

	//WDMATLPS: write the upper part (bit 32:39) of the address
	val64 = ((((uint64_t) dev->control->dma_physical_start) >> 8)
		 & 0xFF000000)
	    | dev->tlp_size;

	//64bit address enable
	if (DMA_64BIT_EN)
		val64 |= 1 << 19;

	SET_BITS(dev->dma_reg->WDMATLPS, val64, 0xFF081FFF);
	printf("set WDMATLPS to 0x%016lx (0x%016lx)\n", val64,
	       val64 & 0xFF081FFF);

	SET_BITS(dev->dma_reg->WDMATLPC, dev->number_of_tlps, 0x0000FFFF);
	printf("set WDMATLPC to 0x%08x (0x%08x)\n", dev->number_of_tlps,
	       dev->number_of_tlps & 0x0000FFFF);

	return 0;
}

/*
int set_dma_buffer_registers(struct dev_descr *dev) {
  // DMABufSizeInScans - use 1 block
  dev->s0->DMA_BUF_SIZE_IN_SCANS
    = dev->control->dma_num_scans;

  //scans per intr must be 2x per DMA_BUFSIZEINSCANS to copy hi/lo part
  //aCAMCNT: double the INTR if 2 cams
  dev->s0->DMAS_PER_INTERRUPT
    = dev->control->dma_num_scans
    *dev->control->number_of_pixels;

  dev->s0->NUMBER_OF_SCANS = dev->control->dma_num_scans;
  dev->s0->CAM_CNT = dev->control->number_of_cameras;

  return 0;
}
*/
/*
void dma_reset(struct dev_descr *dev) {
  dev->dma_reg->DCSR |= 0x01;
  memory_barrier();
  dev->dma_reg->DCSR &= ~0x01;
}


void dma_start(struct dev_descr *dev) {
  dev->control->read_pos = 0;
  dev->control->write_pos = 0;
  dev->dma_reg->DDMACR |= 0x01;
}
*/

uint32_t get_scan_index(struct dev_descr *dev)
{
	return dev->s0->SCAN_INDEX;
}

/*
int lscpcie_setup_dma(struct dev_descr *dev) {
  int result;

  // note: the dma buffer has to be allocated in kernel space
  if ((result = set_dma_address_in_tlp(dev)) < 0) return result;

  if ((result = set_dma_buffer_registers(dev)) < 0) return 0;

  // DREQ: every XCK h->l starts DMA by hardware
  // set hardware start des dma  via DREQ withe data = 0x4000000
  SET_BITS(dev->s0->IRQ.IRQREG, HWDREQ_EN ? 0x40000000 : 0,
           0x40000000);

  // note: enabling the interrupt is again a job for kernel code, do it in
  // start_camera
  return 0;
}
*/
/*
void start_pcie_dma_write(struct dev_descr *dev) {
  if (!HWDREQ_EN) {
    dma_reset(dev);
    dma_start(dev);
  }
}
*/
// note: disabling interrupt is a kernel code job
// as well as releasing the dma buffer
// do it in stop_camera
// therefore no equivalent to clean-up pcie dma


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
