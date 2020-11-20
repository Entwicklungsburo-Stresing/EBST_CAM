#include <stdint.h> // for types, get this into types.h with kernel flag
#include "lscpcie.h"
#include "../kernelspace/ioctl.h"
#include "../kernelspace/registers.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
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

int main(int argc, char **argv) {
  int i, result, device_number, hardware_present;
  dev_descr_t *device_descriptor;
  /*
  uint8_t *io_ptr = MAP_FAILED, *dma_ptr = MAP_FAILED;
  lscpcie_control_t *control_ptr = MAP_FAILED;
  */

  if (argc > 1) device_number = atoi(argv[1]);
  else device_number = 0;

  /*
  snprintf(file_name, 31, "/dev/lscpcie%d", device_number);
  handle = open(file_name, O_RDWR);

  if (handle < 0) {
    fprintf(stderr, "error opening '%s'\n", file_name);
    perror(0);
  }
  */

  result = lscpcie_driver_init();
  if (result < 0) {
    fprintf(stderr, "error %d initialising driver\n", result);
    return result;
  }

  result = lscpcie_open(device_number, 0);
  if (result < 0) {
    fprintf(stderr, "error %d opening device %d\n", result, device_number);
    return result;
  }

  /*
  result = ioctl(handle, LSCPCIE_IOCTL_SET_DEBUG, 0xFFFFFFFF);
  if (result < 0) {
    fprintf(stderr, "error %d on ioctl call for setting debug flags\n", result);
    return result;
  }
  */

  result = lscpcie_set_debug(device_number, 0xFFFF, 0xFFFF);
  if (result < 0) {
    fprintf(stderr, "error %d on setting debug flags\n", result);
    return result;
  }

  printf("debug mode set to 0x%04x\n", result);

  /*
  control_ptr
    = mmap(NULL, sizeof(lscpcie_control_t), PROT_READ | PROT_WRITE, MAP_SHARED,
           handle, page_size);

  if (control_ptr == MAP_FAILED) {
    fprintf(stderr, "mmap on status buffer failed\n");
    perror(0);
    goto finish;
  }
  */

  device_descriptor = lscpcie_get_descriptor(device_number);

  printf("number of pixels %d\n", device_descriptor->control->number_of_pixels);
  printf("number of cameras %d\n",
         device_descriptor->control->number_of_cameras);
  printf("number of scans %d\n", device_descriptor->control->number_of_scans);
  printf("buffer 0x%08lx\n",
         (unsigned long) device_descriptor->mapped_buffer);
  printf("buffer size %d\n", device_descriptor->control->buffer_size);
  printf("dma buffer bus address %ld\n",
         device_descriptor->control->dma_physical_start);
  printf("write position %d\n", device_descriptor->control->write_pos);
  printf("read position %d\n", device_descriptor->control->read_pos);
  printf("number of blocks %d\n", device_descriptor->control->number_of_blocks);

  /*
  dma_ptr
    = mmap(NULL, control_ptr->buffer_size, PROT_READ | PROT_WRITE, MAP_SHARED,
           handle, 2 * page_size);

  if (dma_ptr == MAP_FAILED) {
    fprintf(stderr, "mmap on dma buffer failed\n");
    perror(0);
    goto finish;
  }

  printf("mmap returned for dma memory %p\n", dma_ptr);
  */

  //result = ioctl(handle, LSCPCIE_IOCTL_HARDWARE_PRESENT, &hwp);
  hardware_present = lscpcie_hardware_present(device_number);
  if (hardware_present < 0) {
    fprintf(stderr, "error %d occured when asking for hardware being present\n",
	    hardware_present);
    goto finish;
  }

  if (!hardware_present) {
    printf("no hardware present, skipping io register mmap test\n");
    printf("testing dma buffer instead\n");

    printf("writing 1200 data bytes .............\n");

    uint16_t buffer[1200];

    for (i = 0; i < 1200; i++)
      buffer[i] = i;

    device_descriptor->control->read_pos = 0;
    device_descriptor->control->write_pos = 0;
    printf("%d %d\n", device_descriptor->control->read_pos,
	   device_descriptor->control->write_pos);

    result = write(device_descriptor->handle, buffer, 1200);
    if (result != 1200) {
      fprintf(stderr, "error %d writing data\n", errno);
      return result;
    }

    result = read(device_descriptor->handle, buffer, 1200);
    if (result != 1200) {
      fprintf(stderr, "error %d reading back data\n", errno);
      return result;
    }

    for (i = 0; i < 1200; i++)
      if (buffer[i] != i) {
	fprintf(stderr, "byte %d doesn't match (%d)\n", i, buffer[i]);
	break;
      }
    if (i == 1200)
      printf("successfully read back 1200 data bytes via syscall\n");

    for (i = 0; i < 600; i++)
      if (((uint16_t*)device_descriptor->mapped_buffer)[i] != i) {
	fprintf(stderr, "byte %d doesn't match (%d)\n", i,
		device_descriptor->mapped_buffer[i]);
	break;
      }
    if (i == 600)
      printf("successfully read back 1200 data bytes via memory mapping\n");
  } else {
    /*
    io_ptr = mmap(NULL, 0x100, PROT_READ | PROT_WRITE, MAP_SHARED, handle, 0);

    if (io_ptr == MAP_FAILED) {
      fprintf(stderr, "mmap on io memory failed\n");
      perror(0);
      goto finish;
    }

    printf("mmap returned for io memory %p\n", io_ptr);
    */

    for (int i = 0; i < 512; i++) {
      if (!(i % 8)) {
	if (i) printf("\n");
	printf("0x%02x: ", i);
      }
      out_hex(((uint8_t*)device_descriptor->dma_reg)[i]);//io_ptr[i]);
      putchar(' ');
    }
    printf("\n");
  }

 finish:
  /*
  if (io_ptr != MAP_FAILED) munmap(io_ptr, 0x100);
  if (dma_ptr != MAP_FAILED) munmap(dma_ptr, control_ptr->buffer_size);
  if (control_ptr != MAP_FAILED) munmap(control_ptr, sizeof(lscpcie_control_t));
  */
  lscpcie_close(device_number);
  //close(handle);

  return 0;
}
