#include <stdint.h> // for types, get this into types.h with kernel flag
#include "../kernelspace/ioctl.h"
#include "constants.h"
#include "types.h"
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>


char half_hex(uint8_t x) {
  return x < 10 ? '0' + x : 'a' + x - 10;
}

void out_hex(uint8_t x) {
  printf("0x%c%c", half_hex(x >> 4), half_hex(x & 0x0F));
}

int main(void) {
  int handle = open("/dev/lscpcie0", O_RDWR), i, j, result;

  if (handle < 0) {
    fprintf(stderr, "error on opening '/dev/lscpcie0'\n");
    perror(0);
  }

  /*
  result = ioctl(handle, LSCPCIE_IOCTL_SET_DEBUG,
                 (DEBUG_BITS<<DEBUG_MASK_SHIFT) | D_BUFFERS | D_READOUT);

  if (result < 0) {
    fprintf(stderr, "error %d on ioctl call for setting debug flags\n", result);
    return result;
  }
  */

  uint16_t buffer[1200];

  for (i = 0; i < 1200; i++)
    buffer[i] = i;

  for (j = 0; j < 15000; j++) {
    printf("writing 1200 data bytes\n");
    result = write(handle, buffer, 1200);
    if (result != 1200) {
      fprintf(stderr, "error %d writing data\n", errno);
      return result;
    }

    result = read(handle, buffer, 1200);
    if (result != 1200) {
      fprintf(stderr, "error %d reading back data\n", errno);
      return result;
    }

    for (i = 0; i < 1200; i++)
      if (buffer[i] != i) {
        fprintf(stderr, "byte %d doesn't match (%d)\n", i, buffer[i]);
        break;
      }
    if (i == 1200) printf("successfully read back 1200 data bytes (%d)\n", j);
    else printf("missed %d bytes (%d)\n", 1200 - i, j);
  }

  close(handle);

  return 0;
}
