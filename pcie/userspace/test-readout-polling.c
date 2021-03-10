#include <stdint.h>
#include <memory.h>
#include "types.h"
#include "lscpcie.h"
#include "../kernelspace/registers.h"
#include <stdlib.h>
#include <stdio.h>

#define CFG_BTIMER_IN_US      500000
#define CFG_STIMER_IN_US      400

#define STAT_CTRL_MASK 0x0000FFFF
#define CTRLA_RW_MASK 0x3F
#define CTRLB_RW_MASK 0x3F
#define CTRLC_RW_MASK 0x06

#define memory_barrier() asm volatile ("" : : : "memory")

int check_and_fetch_data(dev_descr_t *device_descriptor, uint8_t *camera_data,
                         int prev_write_pos, int bytes_read)
{
  int actual_write_pos = device_descriptor->control->write_pos, len;

  if (prev_write_pos == actual_write_pos) return 0;

  if (actual_write_pos > device_descriptor->control->read_pos) {
    len = actual_write_pos - device_descriptor->control->read_pos;
    memcpy(camera_data + bytes_read,
           device_descriptor->mapped_buffer
           + device_descriptor->control->read_pos,
           len);
    device_descriptor->control->read_pos += len;
    return len;
  }

  len = device_descriptor->control->dma_buf_size
    - device_descriptor->control->read_pos;
  memcpy(camera_data + bytes_read,
         device_descriptor->mapped_buffer
	 + device_descriptor->control->read_pos,
         len);
  bytes_read += len;

  memcpy(camera_data + bytes_read, device_descriptor->mapped_buffer,
         actual_write_pos);
  device_descriptor->control->read_pos = actual_write_pos;

  return len + actual_write_pos;
}  

int main(int argc, char ** argv)
{
  if (argc != 3) {
    fprintf(stderr,
          "usage: test-readout-polling <number of scans> <number of blocks>\n");
    return 1;
  }

  int n_blocks = atoi(argv[1]);
  int n_scans = atoi(argv[2]);
  int result, bytes_read;
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
         device_descriptor->control->dma_buf_size);

  trigger_mode_t trigger_mode = xck;

  int n_pixel = device_descriptor->control->number_of_pixels;
  int camcnt = device_descriptor->control->number_of_cameras;
  int mem_size = n_pixel * camcnt * n_blocks * n_scans * 2;
  uint8_t *camera_data = malloc(mem_size);
  if (!camera_data) {
    fprintf(stderr, "failed to allocate %d bytes of memory\n", mem_size);
    goto finish;
  }
    
  
  lscpcie_send_fiber(0, MASTER_ADDRESS_CAMERA, CAMERA_ADDRESS_PIXEL,
                     device_descriptor->control->number_of_pixels);
  // and what about this? not present in board.c
  result
    = lscpcie_send_fiber(0, MASTER_ADDRESS_CAMERA, CAMERA_ADDRESS_TRIGGER_IN,
                         trigger_mode);
  if (result < 0) return result;
  result = lscpcie_setup_dma(0, n_scans, n_blocks);
  if (result) {
    fprintf(stderr, "error %d when setting up dma\n", result);
    goto finish;
  }

  // set output of O on PCIe card
  device_descriptor->s0->TOR = TOR_OUT_XCK;
  // set trigger mode to block timer and scan timer + shutter not on
  device_descriptor->s0->CTRLB
    = (CTRLB_BTI_TIMER | CTRLB_STI_TIMER) & ~(CTRLB_SHON);
  // set block timer and start block timer
  device_descriptor->s0->BTIMER = (1<<BTIMER_START) | CFG_BTIMER_IN_US;
  // set slope of block trigger
  device_descriptor->s0->BFLAGS |= 1<<BFLAG_BSLOPE;
  //// >>>> reset_dma_counters
  device_descriptor->s0->DMAS_PER_INTERRUPT |= 1<<DMA_COUNTER_RESET;
  memory_barrier();
  device_descriptor->s0->DMAS_PER_INTERRUPT &= ~(1<<DMA_COUNTER_RESET);

  // reset the internal block counter - is not BLOCKINDEX!
  device_descriptor->s0->DMA_BUF_SIZE_IN_SCANS |= 1<<BLOCK_COUNTER_RESET;
  memory_barrier();
  device_descriptor->s0->DMA_BUF_SIZE_IN_SCANS &= ~(1<<BLOCK_COUNTER_RESET);

  // reset block counter
  device_descriptor->s0->BLOCK_INDEX |= 1<<BLOCK_INDEX_RESET;
  memory_barrier();
  device_descriptor->s0->BLOCK_INDEX &= ~(1<<BLOCK_INDEX_RESET);

  // set Block end stops timer:
  // when SCANINDEX reaches NOS, the timer is stopped by hardware.
  device_descriptor->s0->PCIEFLAGS |= 1<<PCIE_EN_RS_TIMER_HW;
  ////<<<< reset all counters

  // >> SetIntFFTrig
  device_descriptor->s0->XCK.dword &= ~(1<<XCKMSB_EXT_TRIGGER);
  device_descriptor->control->write_pos = 0;
  device_descriptor->control->read_pos = 0;
  // set measure on
  device_descriptor->s0->PCIEFLAGS |= 1<<PCIEFLAG_MEASUREON;
  int prev_write_pos = 0;
  int blk_cnt = 0;
  bytes_read = 0;
  do  {
    result = check_and_fetch_data(device_descriptor, camera_data, prev_write_pos,
                                  bytes_read);
    if (result < 0)
      goto finish;

    if (result) {
      bytes_read += result;
      prev_write_pos
        = (prev_write_pos + result) % device_descriptor->control->dma_buf_size;
    }

    // block trigger?
    if (device_descriptor->s0->CTRLA & (1<<CTRLA_TSTART)) continue;

    // make pulse for blockindex counter
    device_descriptor->s0->PCIEFLAGS |= 1<<PCIEFLAG_BLOCKTRIG;
    memory_barrier();
    device_descriptor->s0->PCIEFLAGS &= ~(1<<PCIEFLAG_BLOCKTRIG);
    // reset scan counter
    device_descriptor->s0->SCAN_INDEX |= 1<<SCAN_INDEX_RESET;
    memory_barrier();
    device_descriptor->s0->SCAN_INDEX &= ~(1<<SCAN_INDEX_RESET);
    // set block on
    device_descriptor->s0->PCIEFLAGS |= 1<<PCIEFLAG_BLOCKON;
    // start Stimer
    device_descriptor->s0->XCK.dword
      = (device_descriptor->s0->XCK.dword & ~XCK_EC_MASK)
      | (CFG_STIMER_IN_US & XCK_EC_MASK) | (1<<XCK_RS);
    // software trigger
    device_descriptor->s0->BTRIGREG |= 1<<FREQ_REG_SW_TRIG;
    memory_barrier();
    device_descriptor->s0->BTRIGREG &= ~(1<<FREQ_REG_SW_TRIG);
    // wait for end of readout
    do {
      result
        = check_and_fetch_data(device_descriptor, camera_data, prev_write_pos,
                               bytes_read);
      if (result < 0)
        goto finish;

      if (result) {
        bytes_read += result;
        prev_write_pos
          = (prev_write_pos + result) % device_descriptor->control->dma_buf_size;
      }
      result = device_descriptor->s0->XCK.dword & (1<<XCK_RS);
    } while (result);
    // reset block on
    device_descriptor->s0->PCIEFLAGS &= ~(1<<PCIEFLAG_BLOCKON);
  } while (++blk_cnt < n_blocks);
        
  device_descriptor->s0->XCK.dword &= ~(1<<XCK_RS);
  // stop btimer
  device_descriptor->s0->BTIMER &= ~(1<<BTIMER_START);
  // reset measure on
  device_descriptor->s0->PCIEFLAGS &= ~(1<<PCIEFLAG_MEASUREON);

  uint16_t data[576];
  lscpcie_readout(0,data,576);
  for (int i = 0; i < 576; i++)
    printf("%d\t%d\n", i, data[i]);
  for (int cur_block = 0; cur_block < n_blocks; cur_block++)
  {
    printf("block %i\n", cur_block);
    for (int cur_sample = 0; cur_sample < n_scans; cur_sample++)
    {
    printf("scan %i\n", cur_sample);
      for(int cur_cam = 0; cur_cam < camcnt; cur_cam++)
      {
        printf("cam %i\n", cur_cam);
        int offset
          = (cur_cam + cur_sample * camcnt + cur_block * n_scans * camcnt)
            * n_pixel;
        for (int i = 0; i < n_pixel; i++)
          printf("%d\t%d\n", i,
                 ((uint16_t*) device_descriptor->mapped_buffer)[offset + i]);
      }
    }
  }

 finish:
  free(camera_data);
  lscpcie_close(0);

  return 0;
}
