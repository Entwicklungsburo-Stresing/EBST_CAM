#include <stdint.h>
#include <memory.h>
#include "types.h"
#include "lscpcie.h"
#include "../kernelspace/registers.h"
#include <stdio.h>

#define STAT_CTRL_MASK 0x0000FFFF
#define CTRLA_RW_MASK 0x3F
#define CTRLB_RW_MASK 0x3F
#define CTRLC_RW_MASK 0x06

#define memory_barrier() asm volatile ("" : : : "memory")

int main(void) {
  int result, i, n;
  dev_descr_t *device_descriptor;

  if ((result = lscpcie_driver_init()) < 0) {
    fprintf(stderr, "initialising driver returned %d\n", result);
    return 1;
  }

  switch (result) {
  case 0: printf("didn't find an lscpcie board\n");    return 0;
  case 1: printf("found one lscpcie board\n");         break;
  case 2: printf("found %d lscpcie boards\n", result); break;
  }

  // open /dev/lscpcie<n>
  if ((result = lscpcie_open(0, 0)) < 0) {
    fprintf(stderr, "opening first board returned %d\n", result);
    return 2;
  }

  // get memory mapped pointers etc
  device_descriptor = lscpcie_get_descriptor(0);

  // clear dma buffer to avoid reading stuff from previous debugging sessions
  memset((uint8_t*)device_descriptor->mapped_buffer, 0,
         device_descriptor->control->buffer_size);

  //// >>>> reset_dma_counters
  device_descriptor->s0->DMAS_PER_INTERRUPT |= (1<<DMA_COUNTER_RESET);
  memory_barrier();
  device_descriptor->s0->DMAS_PER_INTERRUPT &= ~(1<<DMA_COUNTER_RESET);

  //reset the internal block counter - is not BLOCKINDEX!
  device_descriptor->s0->DMA_BUF_SIZE_IN_SCANS |= (1<<BLOCK_COUNTER_RESET);
  memory_barrier();
  device_descriptor->s0->DMA_BUF_SIZE_IN_SCANS &= ~(1<<BLOCK_COUNTER_RESET);
  
  // reset scan counter
  device_descriptor->s0->SCAN_INDEX |= (1<<SCAN_INDEX_RESET);
  memory_barrier();
  device_descriptor->s0->SCAN_INDEX &= ~(1<<SCAN_INDEX_RESET);

  // reset block counter
  device_descriptor->s0->BLOCK_INDEX |= (1<<BLOCK_INDEX_RESET);
  memory_barrier();
  device_descriptor->s0->BLOCK_INDEX &= ~(1<<BLOCK_INDEX_RESET);

  //set Block end stops timer:
  //when SCANINDEX reaches NOS, the timer is stopped by hardware.
  device_descriptor->s0->PCIEFLAGS |= (1<<PCIE_EN_RS_TIMER_HW);
  ////<<<< reset all counters

  trigger_mode_t trigger_mode = xck;
  
  // >> SetIntFFTrig
  device_descriptor->s0->XCK.dword &= ~(1<<XCKMSB_EXT_TRIGGER);

  // and what about this? not present in board.c
  result
    = lscpcie_send_fiber(0, MASTER_ADDRESS_CAMERA, CAMERA_ADDRESS_TRIGGER_IN,
                         trigger_mode);
  if (result < 0) return result;
  // << SetIntTrig

  result = lscpcie_setup_dma(0, 1);
  if (result) {
    fprintf(stderr, "error %d when setting up dma\n", result);
    goto finish;
  }

  device_descriptor->s0->CTRLB = 0x44;
  device_descriptor->s0->BTIMER = 0x8000c350;
  device_descriptor->s0->PCIEFLAGS |= 0x20;
  device_descriptor->s0->BDAT = 123;
  device_descriptor->s0->BEC = 0x1;
  device_descriptor->s0->BFLAGS = 0x1;
  int exptime = 100;
  // >>> start Stimer
  device_descriptor->s0->XCK.dword
    = (device_descriptor->s0->XCK.dword & ~XCK_EC_MASK) | (exptime & XCK_EC_MASK)
    | (1<<XCK_RS) | 1<<31;
  // << start Stimer

  device_descriptor->dma_reg->DDMACR |= (1<<DDMACR_START_DMA_WRT);
  lscpcie_dump_s0(0);
  lscpcie_dump_dma(0);

  do
    result = device_descriptor->s0->XCK.dword & (1<<XCK_RS);
  while (result);

  n = device_descriptor->control->number_of_pixels;
  for (i = 0; i < n; i++)
    printf("%d\t%d\n", i,
           ((uint16_t*)device_descriptor->mapped_buffer)[i]);

 finish:
  lscpcie_close(0);

  return 0;
}