#include <stdint.h>
#include "types.h"
#include "lscpcie.h"
#include "../kernelspace/registers.h"
#include <stdio.h>

#define STAT_CTRL_MASK 0x0000FFFF
#define CTRLA_RW_MASK 0x3F
#define CTRLB_RW_MASK 0x3F
#define CTRLC_RW_MASK 0x06

int main(void) {
  int result;
  struct dev_descr *device_descriptor;
  uint8_t val8;
  uint32_t val32, read_back32;

  if ((result = lscpcie_driver_init()) < 0) {
    fprintf(stderr, "initialising driver returned %d\n", result);
    return 1;
  }

  printf("found %d lscpcie boards\n", result);

  if ((result = lscpcie_open(0, 0)) < 0) {
    fprintf(stderr, "opening first board returned %d\n", result);
    return 2;
  }

  device_descriptor = lscpcie_get_descriptor(0);

  //// pci config space
  if ((result = lscpcie_read_config32(0, PCIeAddr_devStatCtrl, &val32)) < 0) {
    fprintf(stderr, "reading tlp config returned %d\n", result);
    return 3;
  }

  printf("read 0x%08x from pci config stat-ctrl\n", val32);
  val32 &= 0xFFFFFF1F;
  val32 |= (1<<13);
  printf("setting to 0x%08x\n", val32);

  if ((result = lscpcie_write_config32(0, PCIeAddr_devStatCtrl, val32)) < 0) {
    fprintf(stderr, "writing tlp config returned %d\n", result);
    return 3;
  }

  if ((result
       = lscpcie_read_config32(0, PCIeAddr_devStatCtrl, &read_back32)) < 0) {
    fprintf(stderr, "reading back tlp config returned %d\n", result);
    return 3;
  }

  if ((val32 ^ read_back32) & 0xFFFF) {
    fprintf(stderr, "tlp config read back with 0x%08x, should be 0x%08x\n",
	    read_back32, val32);
    return 6;
  }
  printf("reading back ........... ok\n");

  // write 0x20 to tlp config
  val32 |= 0x20;
  printf("setting to 0x%08x\n", val32);
  if ((result = lscpcie_write_config32(0, PCIeAddr_devStatCtrl, val32)) < 0) {
    fprintf(stderr, "writing tlp config returned %d\n", result);
    return 3;
  }

  if ((result
       = lscpcie_read_config32(0, PCIeAddr_devStatCtrl, &read_back32)) < 0) {
    fprintf(stderr, "reading back tlp config returned %d\n", result);
    return 3;
  }

  if ((val32 ^ read_back32) & STAT_CTRL_MASK) {
    fprintf(stderr, "tlp config read back with 0x%08x, should be 0x%08x\n",
	    read_back32, val32);
    return 6;
  }
  printf("reading back ........... ok\n");

  //// S0 space
  // clear CTRLA through virtual memory
  printf("clearing CTRLA via memory mapping\n");
  device_descriptor->s0->CTRLA = 0;

  if ((result = lscpcie_read_s0_8(0, S0Addr_CTRLA, &val8)) < 0) {
    fprintf(stderr, "reading back CTRLA via ioctl returned %d\n", result);
    return 3;
  }

  if (val8 & CTRLA_RW_MASK) {
    fprintf(stderr, "CTRLA read back with %02x, should be zero\n", result);
    return 4;
  }
  printf("reading back ........... ok\n");

  // write 0x23 to CTRLA through virtual memory
  printf("setting CTRLA to 0x23 via memory mapping\n");
  device_descriptor->s0->CTRLA = 0x23;

  if ((result = lscpcie_read_s0_8(0, S0Addr_CTRLA, &val8)) < 0) {
    fprintf(stderr, "reading back CTRLA via ioctl returned %d\n", result);
    return 5;
  }

  if ((val8 & CTRLA_RW_MASK) != 0x23) {
    fprintf(stderr, "CTRLA read back with %02x, should be 0x23\n",
	    val8 & CTRLA_RW_MASK);
    return 6;
  }
  printf("reading back ........... ok\n");

  // clear CTRLA through ioctl
  printf("clearing CTRLA via ioctl\n");
  if ((result = lscpcie_write_s0_8(0, S0Addr_CTRLA, 0)) < 0) {
    fprintf(stderr, "writing to CTRLA returned %d\n", result);
    return 7;
  }

  if ((val8 = device_descriptor->s0->CTRLA) & CTRLA_RW_MASK) {
    fprintf(stderr,
	    "CTRLA read back with %02x via memory mapping, should be zero\n",
	    val8 & CTRLA_RW_MASK);
    return 4;
  }
  printf("reading back ........... ok\n");

  // set CTRLA to 0x23 through ioctl
  printf("setting CTRLA to 0x23 via ioctl\n");
  if ((result = lscpcie_write_s0_8(0, S0Addr_CTRLA, 0x23)) < 0) {
    fprintf(stderr, "writing to CTRLA returned %d\n", result);
    return 7;
  }

  if (((val8 = device_descriptor->s0->CTRLA) & CTRLA_RW_MASK) != 0x23) {
    fprintf(stderr,
	    "CTRLA read back with %02x via memory maping, should be 0x23\n",
	    val8 & CTRLA_RW_MASK);
    return 4;
  }
  printf("reading back ........... ok\n");

  //// DMA space
  // clear WDMATLPA through virtual memory

  printf("clearing WDMATLPA via memory mapping\n");
  device_descriptor->dma_reg->WDMATLPA = 0;

  if ((result = lscpcie_read_dma_32(0, DmaAddr_WDMATLPA, &val32)) < 0) {
    fprintf(stderr, "reading back WDMATLPA returned %d\n", result);
    return 3;
  }

  if (val32) {
    fprintf(stderr, "WDMATLPA read back with %02x, should be zero\n", result);
    return 4;
  }
  printf("reading back ........... ok\n");

  // write 0x23 to WDMATLPA through virtual memory
  printf("setting WDMATLPA to 0xAA via memory mapping\n");
  device_descriptor->dma_reg->WDMATLPA = 0xAA;

  if ((result = lscpcie_read_dma_8(0, DmaAddr_WDMATLPA, &val8)) < 0) {
    fprintf(stderr, "reading back WDMATLPA returned %d\n", result);
    return 5;
  }

  if (val8 != 0xAA) {
    fprintf(stderr, "WDMATLPA read back with %02x, should be 0xAA\n", result);
    return 6;
  }
  printf("reading back ........... ok\n");

  // clear WDMATLPA through ioctl
  if ((result = lscpcie_write_dma_8(0, DmaAddr_WDMATLPA, 0)) < 0) {
    fprintf(stderr, "writing to WDMATLPA returned %d\n", result);
    return 7;
  }

  if (device_descriptor->dma_reg->WDMATLPA) {
    fprintf(stderr, "WDMATLPA read back with %02x, should be zero\n", result);
    return 4;
  }

  // set WDMATLPA to 0xAA through ioctl
  printf("clearing WDMATLPA via ioctl\n");
  if ((result = lscpcie_write_dma_8(0, DmaAddr_WDMATLPA, 0xAA)) < 0) {
    fprintf(stderr, "writing to WDMATLPA returned %d\n", result);
    return 7;
  }

  if (device_descriptor->dma_reg->WDMATLPA != 0xAA) {
    fprintf(stderr, "WDMATLPA read back with %02x, should be 0xAA\n", result);
    return 4;
  }
  printf("reading back ........... ok\n");

  lscpcie_close(0);

  return 0;
}
