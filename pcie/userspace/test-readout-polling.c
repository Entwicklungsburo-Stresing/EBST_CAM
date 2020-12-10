#include <stdint.h>
#include <memory.h>
#include "types.h"
#include "lscpcie.h"
#include "../kernelspace/registers.h"
#include <stdio.h>

#define CFG_NUMBER_OF_SCANS   5
#define CFG_NUMBER_OF_BLOCKS  2
#define CFG_BTIMER_IN_US      2500
#define CFG_STIMER_IN_US      400

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

  trigger_mode_t trigger_mode = xck;

  lscpcie_send_fiber(0, MASTER_ADDRESS_CAMERA, CAMERA_ADDRESS_PIXEL, device_descriptor[0].control->number_of_pixels);
  // and what about this? not present in board.c
  result = lscpcie_send_fiber(0, MASTER_ADDRESS_CAMERA, CAMERA_ADDRESS_TRIGGER_IN, trigger_mode);
  if (result < 0) return result;
  result = lscpcie_setup_dma(0, CFG_NUMBER_OF_BLOCKS);
  if (result) {
    fprintf(stderr, "error %d when setting up dma\n", result);
    goto finish;
  }

  //set output of O on PCIe card
  device_descriptor->s0->TOR = TOR_OUT_BTIMER;
  //set trigger mode to block timer and scan timer + shutter not on
  device_descriptor->s0->CTRLB = (CTRLB_BTI_TIMER | CTRLB_STI_TIMER) & ~(CTRLB_SHON);
  //set block timer and start block timer
  device_descriptor->s0->BTIMER = 1<<BTIMER_START | CFG_BTIMER_IN_US;
  //set slope of block trigger
  device_descriptor->s0->BFLAGS |= 1<<BFLAG_BSLOPE;
  //set number of scans
  device_descriptor->s0->NUMBER_OF_SCANS = CFG_NUMBER_OF_SCANS;

  //// >>>> reset_dma_counters
  device_descriptor->s0->DMAS_PER_INTERRUPT |= (1<<DMA_COUNTER_RESET);
  memory_barrier();
  device_descriptor->s0->DMAS_PER_INTERRUPT &= ~(1<<DMA_COUNTER_RESET);

  //reset the internal block counter - is not BLOCKINDEX!
  device_descriptor->s0->DMA_BUF_SIZE_IN_SCANS |= (1<<BLOCK_COUNTER_RESET);
  memory_barrier();
  device_descriptor->s0->DMA_BUF_SIZE_IN_SCANS &= ~(1<<BLOCK_COUNTER_RESET);

  // reset block counter
  device_descriptor->s0->BLOCK_INDEX |= (1<<BLOCK_INDEX_RESET);
  memory_barrier();
  device_descriptor->s0->BLOCK_INDEX &= ~(1<<BLOCK_INDEX_RESET);

  //set Block end stops timer:
  //when SCANINDEX reaches NOS, the timer is stopped by hardware.
  device_descriptor->s0->PCIEFLAGS |= (1<<PCIE_EN_RS_TIMER_HW);
  ////<<<< reset all counters

  // >> SetIntFFTrig
  device_descriptor->s0->XCK.dword &= ~(1<<XCKMSB_EXT_TRIGGER);
  //set measure on
  device_descriptor->s0->PCIEFLAGS |= 1<<PCIEFLAG_MEASUREON;
  for (int blk_cnt = 0; blk_cnt < CFG_NUMBER_OF_BLOCKS; blk_cnt++)
  {
    //block trigger
    if(!(device_descriptor->s0->CTRLA & 1<<CTRLA_TSTART)) while(!(device_descriptor->s0->CTRLA & 1<<CTRLA_TSTART));
    //make pulse for blockindex counter
    device_descriptor->s0->PCIEFLAGS |= 1<<PCIEFLAG_BLOCKTRIG;
    memory_barrier();
    device_descriptor->s0->PCIEFLAGS &= ~(1<<PCIEFLAG_BLOCKTRIG);
    // reset scan counter
    device_descriptor->s0->SCAN_INDEX |= (1<<SCAN_INDEX_RESET);
    memory_barrier();
    device_descriptor->s0->SCAN_INDEX &= ~(1<<SCAN_INDEX_RESET);
    //set block on
    device_descriptor->s0->PCIEFLAGS |= 1<<PCIEFLAG_BLOCKON;
    //start Stimer
    device_descriptor->s0->XCK.dword = (device_descriptor->s0->XCK.dword & ~XCK_EC_MASK) | (CFG_STIMER_IN_US & XCK_EC_MASK) | (1<<XCK_RS);
    //software trigger
    device_descriptor->s0->BTRIGREG |= 1<<FREQ_REG_SW_TRIG;
    memory_barrier();
    device_descriptor->s0->BTRIGREG &= ~(1<<FREQ_REG_SW_TRIG);
    //wait for end of readout
    do
      result = device_descriptor->s0->XCK.dword & (1<<XCK_RS);
    while (result);
    //reset block on
    device_descriptor->s0->PCIEFLAGS &= ~(1<<PCIEFLAG_BLOCKON);
  }
  //stop stimer
  device_descriptor->s0->XCK.dword &= ~(1<<XCK_RS);
  //stop btimer
  device_descriptor->s0->BTIMER &= ~(1<<BTIMER_START);
  //reset measure on
  device_descriptor->s0->PCIEFLAGS &= ~(1<<PCIEFLAG_MEASUREON);

  n = device_descriptor->control->number_of_pixels;
  for (i = 0; i < n; i++)
    printf("%d\t%d\n", i,
           ((uint16_t*)device_descriptor->mapped_buffer)[i]);

 finish:
  lscpcie_close(0);

  return 0;
}
