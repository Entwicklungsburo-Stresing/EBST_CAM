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
  int result;
  struct dev_descr *device_descriptor;

  if ((result = lscpcie_driver_init()) < 0) {
    fprintf(stderr, "initialising driver returned %d\n", result);
    return 1;
  }

  switch (result) {
  case 0: printf("didn't find an lscpcie board\n");    return 0;
  case 1: printf("found one lscpcie board\n");         break;
  case 2: printf("found %d lscpcie boards\n", result); break;
  }

  if ((result = lscpcie_open(0, 0)) < 0) {
    fprintf(stderr, "opening first board returned %d\n", result);
    return 2;
  }

  device_descriptor = lscpcie_get_descriptor(0);
  // >>> stop ff timer
  device_descriptor->s0->XCK.dword &= ~(1<<XCK_RS);
  // << stop ff timer

  lscpcie_close(0);

  return 0;
}
