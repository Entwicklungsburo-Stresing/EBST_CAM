/* lscpcie.c
 *
 * Copyright 2020 Bernhard Lang, University of Geneva
 * Copyright 2020 Entwicklungsbuero Stresing (http://www.stresing.de/)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include "lscpcie.h"
#include "constants.h"
#include "types.h"
#include "../kernelspace/ioctl.h"
#include "../kernelspace/registers.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdarg.h>

#include "../kernelspace/local-config.h"

/*
#define DMA_HW_BUFPARTS 2
#define DMA_DMASPERINTR DMA_BUFSIZEINSCANS / DMA_HW_BUFPARTS
*/


#define memory_barrier() asm volatile ("" : : : "memory")

static int error_reporting = 1;

static const dev_descr_t init_dev_descr = {
  .handle = -1,
  .number_of_tlps = 0,
  .dma_reg = 0
};

static dev_descr_t dev_descr[MAX_PCIE_BOARDS];
static int number_of_pcie_boards = -1;


void set_error_reporting(int level) {
  error_reporting = level;
}

void error_message(const char *format, ... ) {
  if (!error_reporting) return;
  va_list ap;
  va_start(ap, format);
  vfprintf(stderr, format, ap);
  va_end(ap);
}

dev_descr_t *lscpcie_get_descriptor(uint dev) {
  return dev < number_of_pcie_boards ? dev_descr + dev : NULL;
}

int lscpcie_driver_init(void) {
  int result, handle, i;
  
  handle = open("/dev/lscpcie0", O_RDWR);
  if (handle < 0) return handle; // failed to open device zero

  result = ioctl(handle, LSCPCIE_IOCTL_NUM_BOARDS, &number_of_pcie_boards);
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
  Pixelsize = TLPS * TLPC - 1*TLPS
  (TLPS TLP size = 64)
  TLPC 0x Pixelsize
    2	64
    3	128
    4	192
    5	256
    6	320
    7	384
    8	448
    9	512
    a	576
    b	640
    c	704
    d	768
    e	832
    f	896
    10	960
    11	1024
    12	1088
    13	1152
    14	1216
    15	1280
    16	1344
    17	1408
    18	1472
    19	1536
    1a	1600
    1b	1664
    1c	1728
    1d	1792
    1e	1856
    1f	1920
    20	1984
    21	2048
    22	2112
    23  2176
    ...
    41  4096
    42  4160
    ...
    82  8256
*/

int lscpcie_open(uint dev, uint16_t options) {
  int result = 0, handle, no_tlps, hardware_present;
  char name[16];
  int page_size = sysconf(_SC_PAGE_SIZE);

  if (dev >= number_of_pcie_boards) return -ENODEV;
  if (dev_descr[dev].handle >= 0) return -EBUSY;

  sprintf(name, "/dev/lscpcie%d", dev);
  handle = open(name, O_RDWR);
  if ((handle < 0) && error_reporting) {
    error_message("error on opening '%s': ", name);
    if (error_reporting) perror(0);
    return handle;
  }

  dev_descr[dev].handle = handle;
  hardware_present = lscpcie_hardware_present(dev);
  if (hardware_present < 0) goto error;

  // dma control struct in ram
  dev_descr[dev].control
    = mmap(NULL, sizeof(lscpcie_control_t), PROT_READ | PROT_WRITE, MAP_SHARED,
           handle, page_size);

  if (dev_descr[dev].control == MAP_FAILED) {
    error_message("mmap on dma control ram failed\n");
    if (error_reporting) perror(0);
    goto error;
  }

  switch (dev_descr[dev].control->number_of_pixels) {
  case 128:  no_tlps = 0x3;  break;
  case 192:  no_tlps = 0x4;  break;
  case 320:  no_tlps = 0x6;  break;
  case 576:  no_tlps = 0xa;  break;
  case 1088: no_tlps = 0x12; break;
  case 2112: no_tlps = 0x22; break;
  case 4160: no_tlps = 0x42; break;
  case 8256: no_tlps = 0x82; break;
  default:
    error_message("invalid number of pixels %d\n",
                  dev_descr[dev].control->number_of_pixels);
    return -EINVAL;
  }

  dev_descr[dev].number_of_tlps = no_tlps;

  printf("found io spce of size 0x%08x\n", dev_descr[dev].control->io_size);
  if (hardware_present) {
    // map io registers to user space memory
    dev_descr[dev].dma_reg
      = mmap(NULL, dev_descr[dev].control->io_size, PROT_READ | PROT_WRITE,
             MAP_SHARED, handle, 0);

    if (dev_descr[dev].dma_reg == MAP_FAILED) {
      error_message("mmap on io memory failed\n");
      if (error_reporting) perror(0);
      result = errno;
      goto error;
    }

    // s0 addres space pointer
    dev_descr[dev].s0 = (s0_t*) (((uint8_t*)dev_descr[dev].dma_reg) + 0x80);

    // startval for CTRLA Reg  +slope, IFC=h, VON=1
    // clear CTRLB & CTRLC

    // note: the above mmap system call tells the operating system to perform
    // a write to pci io-registers when writing to the corresponding virtual
    // pointer, pretty cool, isn't it?
    dev_descr[dev].s0->CTRLA = 0x23;
    dev_descr[dev].s0->CTRLB = 0;
    dev_descr[dev].s0->CTRLC = 0;
    dev_descr[dev].s0->PIXREG = dev_descr[dev].control->number_of_pixels;
    dev_descr[dev].s0->CAM_CNT = dev_descr[dev].control->number_of_cameras & 0xF;

    // initialise number of pixels and clock scheme
    result
      = lscpcie_send_fiber(dev, MASTER_ADDRESS_CAMERA, CAMERA_ADDRESS_PIXEL,
                           dev_descr[dev].control->number_of_pixels);
    if (result < 0) goto error;

    result
      = lscpcie_send_fiber(dev, MASTER_ADDRESS_CAMERA, CAMERA_ADDRESS_VCLK,
                           options);
    if (result < 0) goto error;
  } else {
    dev_descr[dev].dma_reg = NULL;
    dev_descr[dev].s0 = NULL;
  }

  dev_descr[dev].mapped_buffer
    = mmap(NULL, dev_descr[dev].control->buffer_size, PROT_READ | PROT_WRITE,
           MAP_SHARED, handle, 2 * page_size);
  if (dev_descr[dev].mapped_buffer == MAP_FAILED) return -1;

  //if (_COOLER) ActCooling(drvno, FALSE); //deactivate cooler

  return handle;

 error:
  lscpcie_close(dev);
  return result;
}

void lscpcie_close(uint dev) {
  if (dev >= number_of_pcie_boards) return;

  if (dev_descr[dev].dma_reg != MAP_FAILED)
    munmap(dev_descr[dev].dma_reg, 0x100);

  if (dev_descr[dev].control != MAP_FAILED)
    munmap(dev_descr[dev].control, sizeof(lscpcie_control_t));

  if (dev_descr[dev].handle >= 0)
    close(dev_descr[dev].handle);

  dev_descr[dev] = init_dev_descr;
}

// not yet implemented in driver
int lscpcie_start(uint dev) {
  if (dev >= number_of_pcie_boards) return -ENODEV;
  //-->>set_dma_buffer_registers(dev
  return ioctl(dev_descr[dev].handle, LSCPCIE_IOCTL_START);
}

// not yet implemented in driver
int lscpcie_stop(uint dev) {
  if (dev >= number_of_pcie_boards) return -ENODEV;
  return ioctl(dev_descr[dev].handle, LSCPCIE_IOCTL_STOP);
}

ssize_t lscpcie_readout(uint dev, uint16_t *buffer, size_t items_to_read) {
  ssize_t result;
  size_t bytes_read = 0, bytes_to_read = items_to_read * 2;

  if (dev >= number_of_pcie_boards) return -ENODEV;

  while (bytes_read < bytes_to_read) {
    result = read(dev_descr[dev].handle, buffer + bytes_read / 2, bytes_to_read);
    if (result > 0) {
      bytes_read += result;
      bytes_to_read -= result;
    } else
      if (result < 0) {
	fprintf(stderr, "read ccd returned %ld, error code %d\n", result,
                errno);
	return result;
      }
  }

  return bytes_read / 2;
}

int lscpcie_hardware_present(uint dev) {
  int result, hwp;

  if (dev >= number_of_pcie_boards) return -ENODEV;
  result = ioctl(dev_descr[dev].handle, LSCPCIE_IOCTL_HARDWARE_PRESENT, &hwp);

  return result ? result : hwp;
}

int lscpcie_fifo_overflow(uint dev) {
  int result, fo;

  if (dev >= number_of_pcie_boards) return -ENODEV;
  result = ioctl(dev_descr[dev].handle, LSCPCIE_IOCTL_FIFO_OVERFLOW, &fo);

  return result ? result : fo;
}

int lscpcie_clear_fifo(uint dev) {
  if (dev >= number_of_pcie_boards) return -ENODEV;
  return ioctl(dev_descr[dev].handle, LSCPCIE_IOCTL_CLEAR_FIFO);
}

size_t lscpcie_bytes_free(uint dev) {
  int result, fb;

  if (dev >= number_of_pcie_boards) return -ENODEV;
  result = ioctl(dev_descr[dev].handle, LSCPCIE_IOCTL_FREE_BYTES, &fb);

  return result ? result : fb;
}

size_t lscpcie_bytes_available(uint dev) {
  int result, ba;

  if (dev >= number_of_pcie_boards) return -ENODEV;
  result = ioctl(dev_descr[dev].handle, LSCPCIE_IOCTL_BYTES_AVAILABLE, &ba);

  return result ? result : ba;
}

int lscpcie_set_debug(uint dev, int flags, int mask) {
  if (dev >= number_of_pcie_boards) return -ENODEV;
  return ioctl(dev_descr[dev].handle, LSCPCIE_IOCTL_SET_DEBUG,
               (mask << DEBUG_MASK_SHIFT) | flags);
}

int lscpcie_get_buffer_pointers(uint dev, uint64_t *pointers) {
  if (dev >= number_of_pcie_boards) return -ENODEV;
  return ioctl(dev_descr[dev].handle, LSCPCIE_IOCTL_BUFFER_POINTERS, pointers);
}


/*******************************************************************************/
/*                                 fiber link                                  */
/*******************************************************************************/

int lscpcie_send_fiber(uint dev, uint8_t master_address,
                       uint8_t register_address, uint16_t data) {
  uint32_t reg_val = (master_address << 24) | (register_address < 16) | data;

  if (dev >= number_of_pcie_boards) return -ENODEV;
  if (!dev_descr[dev].s0) return -ENODEV;

  dev_descr[dev].s0->DBR = reg_val;
  memory_barrier();
  dev_descr[dev].s0->DBR = reg_val | 0x4000000;
  memory_barrier();
  dev_descr[dev].s0->DBR = 0;
  usleep(1000);
  return 0;
}

int init_cam_control(uint dev, trigger_mode_t trigger_mode, uint16_t options) {
  int result;

  result
    = lscpcie_send_fiber(dev, MASTER_ADDRESS_CAMERA, CAMERA_ADDRESS_PIXEL,
                         dev_descr[dev].control->number_of_pixels);
  if (result < 0) return result;

  result
    = lscpcie_send_fiber(dev, MASTER_ADDRESS_CAMERA, CAMERA_ADDRESS_TRIGGER_IN,
                         trigger_mode);
  if (result < 0) return result;

  result
    = lscpcie_send_fiber(dev, MASTER_ADDRESS_CAMERA, CAMERA_ADDRESS_PIXEL,
                         options);

  return result;
}

/*******************************************************************************/
/*                                    dma                                      */
/*******************************************************************************/

int set_dma_address_in_tlp(uint dev) {
  int result;
  uint64_t val64;
  uint32_t data = 0, tlp_mode;

  if (_FORCETLPS128) tlp_mode = 0;
  else {
    if ((result = lscpcie_read_config32(dev, PCIeAddr_devCap, &data)) < 0)
      return result;

    tlp_mode = data & 0x7;
  }

  data = (data & ~0xE0) | (1<<13);

  switch (tlp_mode) {
  case 0: dev_descr[dev].tlp_size = 0x20; break;
  case 1: dev_descr[dev].tlp_size = 0x40; break;
  case 2: dev_descr[dev].tlp_size = 0x80; break;
  }

  data |= dev_descr[dev].tlp_size / 0x20;
  if ((result = lscpcie_write_config32(dev, PCIeAddr_devStatCtrl, data)) < 0)
    return result;

  // WDMATLPA (Reg name): write the lower part (bit 02:31) of the DMA adress
  // to the DMA controller
  SET_BITS(dev_descr[dev].dma_reg->WDMATLPA,
           (uint64_t) dev_descr[dev].dma_physical_address,
           0xFFFFFFFC);

  //WDMATLPS: write the upper part (bit 32:39) of the address  
  val64
    = ((((uint64_t) dev_descr[dev].control->dma_physical_start) >> 8)
       & 0xFF000000)
    | dev_descr[dev].tlp_size;

  //64bit address enable
  if (DMA_64BIT_EN) val64 |= 1<<19;

  SET_BITS(dev_descr[dev].dma_reg->WDMATLPS, val64, 0xFF081FFF);
  SET_BITS(dev_descr[dev].dma_reg->WDMATLPC, dev_descr[dev].number_of_tlps,
           0x0000FFFF);

  return 0;
}

int set_dma_buffer_registers(uint dev, uint number_of_blocks) {
  // DMABufSizeInScans - use 1 block
  dev_descr[dev].s0->DMA_BUF_SIZE_IN_SCANS
    = dev_descr[dev].control->number_of_scans;

  //scans per intr must be 2x per DMA_BUFSIZEINSCANS to copy hi/lo part
  //aCAMCNT: double the INTR if 2 cams
  dev_descr[dev].s0->DMAS_PER_INTERRUPT
    = dev_descr[dev].control->number_of_scans *
    dev_descr[dev].control->number_of_cameras
    / INTERRUPTS_PER_SCAN;

  //>>>> could be done in driver at module load
  dev_descr[dev].s0->NUMBER_OF_SCANS = dev_descr[dev].control->number_of_scans;
  dev_descr[dev].s0->NUMBER_OF_BLOCKS = number_of_blocks;
  dev_descr[dev].s0->CAM_CNT = dev_descr[dev].control->number_of_cameras;
  //<<<< could be done in driver at module load

  return 0;
}

void dma_reset(uint dev) {
  dev_descr[dev].dma_reg->DCSR |= 0x01;
  memory_barrier();
  dev_descr[dev].dma_reg->DCSR &= ~0x01;
}


void dma_start(uint dev) {
  dev_descr[dev].control->read_pos = 0;
  dev_descr[dev].control->write_pos = 0;
  dev_descr[dev].dma_reg->DDMACR |= 0x01;
}

uint32_t get_scan_index(uint dev) {
  return dev_descr[dev].s0->SCAN_INDEX;
}

int lscpcie_setup_dma(uint dev, uint32_t number_of_blocks) {
  int result;

  // note: the dma buffer has to be allocated in kernel space
  if ((result = set_dma_address_in_tlp(dev)) < 0) return result;

  if ((result = set_dma_buffer_registers(dev, number_of_blocks)) < 0) return 0;

  // DREQ: every XCK h->l starts DMA by hardware
  //set hardware start des dma  via DREQ withe data = 0x4000000
  SET_BITS(dev_descr[dev].s0->IRQ.IRQREG, HWDREQ_EN ? 0x40000000 : 0,
           0x40000000);

  // note: enabling the interrupt is again a job for kernel code, do it in
  // start_camera
  return 0;
}

void start_pcie_dma_write(uint dev) {
  if (!HWDREQ_EN) {
    dma_reset(dev);
    dma_start(dev);
  }
}

// note: disabling interrupt is a kernel code job
// as well as releasing the dma buffer
// do it in stop_camera
// therefore no equivalent to clean-up pcie dma


/*******************************************************************************/
/*                              register access                                */
/*******************************************************************************/

int lscpcie_read_config32(uint dev, uint16_t address, uint32_t *val) {
  int result;
  reg_info_t reg = { .address = address };

  if (dev >= number_of_pcie_boards) return -ENODEV;
  if ((result = ioctl(dev_descr[dev].handle, LSCPCIE_IOCTL_GET_CONF, &reg)) < 0)
    return result;
  *val = reg.value;

  return 0;
}

int lscpcie_write_config32(uint dev, uint16_t address, uint32_t val) {
  reg_info_t reg = { .address = address, .value = val };

  if (dev >= number_of_pcie_boards) return -ENODEV;

  return ioctl(dev_descr[dev].handle, LSCPCIE_IOCTL_SET_CONF, &reg);
}

int lscpcie_read_reg8(uint dev, uint16_t address, uint8_t *val) {
  int result;
  reg_info_t reg = { .address = address };

  if (dev >= number_of_pcie_boards) return -ENODEV;
  if ((result = ioctl(dev_descr[dev].handle, LSCPCIE_IOCTL_GET_REG8, &reg)) < 0)
    return result;
  *val = reg.value;

  return 0;
}

int lscpcie_read_reg16(uint dev, uint16_t address, uint16_t *val) {
  int result;
  reg_info_t reg = { .address = address };

  if (dev >= number_of_pcie_boards) return -ENODEV;
  if ((result = ioctl(dev_descr[dev].handle, LSCPCIE_IOCTL_GET_REG16, &reg)) < 0)
    return result;
  *val = reg.value;

  return 0;
}

int lscpcie_read_reg32(uint dev, uint16_t address, uint32_t *val) {
  int result;
  reg_info_t reg = { .address = address };

  if (dev >= number_of_pcie_boards) return -ENODEV;
  if ((result = ioctl(dev_descr[dev].handle, LSCPCIE_IOCTL_GET_REG32, &reg)) < 0)
    return result;
  *val = reg.value;

  return 0;
}

int lscpcie_write_reg8(uint dev, uint16_t address, uint8_t val) {
  reg_info_t reg = { .address = address, .value = val };

  if (dev >= number_of_pcie_boards) return -ENODEV;

  return ioctl(dev_descr[dev].handle, LSCPCIE_IOCTL_SET_REG8, &reg);
}

int lscpcie_write_reg16(uint dev, uint16_t address, uint16_t val) {
  reg_info_t reg = { .address = address, .value = val };

  if (dev >= number_of_pcie_boards) return -ENODEV;

  return ioctl(dev_descr[dev].handle, LSCPCIE_IOCTL_SET_REG16, &reg);
}

int lscpcie_write_reg32(uint dev, uint16_t address, uint32_t val) {
  reg_info_t reg = { .address = address, .value = val };

  if (dev >= number_of_pcie_boards) return -ENODEV;

  return ioctl(dev_descr[dev].handle, LSCPCIE_IOCTL_SET_REG32, &reg);
}

int lscpcie_set_bits_reg8(uint dev, uint16_t address, uint8_t bits,
                          uint8_t mask) {
  int result;
  uint8_t value;

  if ((result = lscpcie_read_reg8(dev, address, &value)) < 0) return result;

  return lscpcie_write_reg8(dev, address, (value & ~mask) | (bits & mask));
}

int lscpcie_set_bits_reg16(uint dev, uint16_t address, uint16_t bits,
                           uint32_t mask) {
  int result;
  uint16_t value;

  if ((result = lscpcie_read_reg16(dev, address, &value)) < 0) return result;
  return lscpcie_write_reg16(dev, address, (value & ~mask) | (bits & mask));
}

int lscpcie_set_bits_reg32(uint dev, uint16_t address, uint32_t bits,
                           uint32_t mask) {
  int result;
  uint32_t value;

  if ((result = lscpcie_read_reg32(dev, address, &value)) < 0) return result;
  return lscpcie_write_reg32(dev, address, (value & ~mask) | (bits & mask));
}


/*******************************************************************************/
/*                                 debugging                                   */
/*******************************************************************************/

int lscpcie_dump_s0(uint dev) {
  int i;
  uint32_t data = 0;
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
	}; //Look-Up-Table for the S0 Registers

  printf("S0- registers   \n" );

  for (i = 0; i < number_of_registers; i++) {
    lscpcie_read_s0_32(dev, i * 4, &data);
    printf("%s \t: 0x%08x\n", register_names[i], data);
  }
  return 0;
  //crashing when doing this:
  //return lscpcie_dump_tlp(dev);
}

int lscpcie_dump_dma(uint dev) {
  uint32_t data = 0;
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
	}; //Look-Up-Table for the DMA Registers
  printf("DMA registers\n");
  for (int i = 0; i < number_of_registers; i++) {
    lscpcie_read_dma_32(dev, i * 4, &data);
    printf("%s \t: 0x%08x\n", register_names[i], data);
  }
  return 0;
}

int lscpcie_dump_tlp(uint dev) {
  uint32_t data, actpayload;

  printf("PAY_LOAD values : 0 = 128 bytes, 1 = 256 bytes, 2 = 512 bytes\n");
  lscpcie_read_config32(dev, PCIeAddr_devCap, &data);
  printf("PAY_LOAD Supported : 0x%x\n", data & 0x7 );

  lscpcie_read_config32(dev, PCIeAddr_devStatCtrl, &data);
  actpayload = (data >> 5) & 0x07;
  printf("PAY_LOAD : 0x%x\n", actpayload);

  lscpcie_read_config32(dev, PCIeAddr_devStatCtrl, &data);
  printf("MAX_READ_REQUEST_SIZE : 0x%x\n\n", (data >> 12) & 0x7);


  printf("number of pixels: %d \n", dev_descr[dev].control->number_of_pixels);

  switch (actpayload) {
  case 0: data = 0x20;  break;
  case 1: data = 0x40;  break;
  case 2: data = 0x80;  break;
  case 3: data = 0x100; break;
  }

  printf("TLP_SIZE is: %d DWORDs = %d BYTEs\n", data, data * 4 );

  lscpcie_read_dma_32(dev, DmaAddr_WDMATLPS, &data);
  printf("TLPS in DMAReg is: %d \n", data);

  data = (dev_descr[dev].control->number_of_pixels - 1) / (data * 2) + 1 + 1;
  printf("number of TLPs should be: %d\n", data);
  lscpcie_read_dma_32(dev, DmaAddr_WDMATLPC, &data);
  printf("number of TLPs is: %d \n", data);

  return 0;
}
