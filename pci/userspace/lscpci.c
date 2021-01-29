/* lscpci.c
 *
 * Copyright (C) 2010-2016 Bernhard Lang, University of Geneva
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "lscpci.h"
#include "../kernelspace/register-names.h"
#include "../kernelspace/ioctl.h"

#include <stdio.h>

int lscpci_open(const char *name, int flags) {
  int handle = open(name, flags);
  if (handle < 0) {
    fprintf(stderr, "error on opening '%s': ", name);
    perror(0);
  }
  return handle;
}

int lscpci_number_of_pixels(int handle) {
  int result;
  uint16_t pixels;
  result = ioctl(handle, LSCPCI_IOCTL_GET_PIXELS, &pixels);
  if (result) return result;
  return pixels;
}

int lscpci_start(int handle) {
  return ioctl(handle, LSCPCI_IOCTL_START);
}

int lscpci_init(int handle) {
  return ioctl(handle, LSCPCI_IOCTL_INIT);
}

int lscpci_idle_run(int handle) {
  return ioctl(handle, LSCPCI_IOCTL_IDLE_RUN);
}

int lscpci_stop(int handle) {
  return ioctl(handle, LSCPCI_IOCTL_STOP);
}

ssize_t lscpci_readout(int handle, uint16_t *buffer, size_t items_to_read) {
  ssize_t result;
  size_t bytes_read = 0, bytes_to_read = items_to_read * 2;

  while (bytes_read < bytes_to_read) {
    result = read(handle, buffer + bytes_read / 2, bytes_to_read);
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

int lscpci_hardware_present(int handle) {
  int result, hwp;
  result = ioctl(handle, LSCPCI_IOCTL_HARDWARE_PRESENT, &hwp);
  return result ? result : hwp;
}

int lscpci_fifo_overflow(int handle) {
  int result, fo;
  result = ioctl(handle, LSCPCI_IOCTL_FIFO_OVERFLOW, &fo);
  return result ? result : fo;
}

int lscpci_clear_fifo(int handle) {
  return ioctl(handle, LSCPCI_IOCTL_CLEAR_FIFO);
}

size_t lscpci_bytes_free(int handle) {
  int result, fb;
  result = ioctl(handle, LSCPCI_IOCTL_FREE_BYTES, &fb);
  return result ? result : fb;
}

size_t lscpci_bytes_available(int handle) {
  int result, ba;
  result = ioctl(handle, LSCPCI_IOCTL_BYTES_AVAILABLE, &ba);
  return result ? result : ba;
}

int lscpci_dump_registers(int handle, unsigned char *reg) {
  return ioctl(handle, LSCPCI_IOCTL_DUMP_REG, reg);
}

int lscpci_set_register(int handle, uint8_t address, uint8_t value) {
  return ioctl(handle, LSCPCI_IOCTL_SET_REG_BYTE, (address<<8) + value);
}

/*
int lscpci_fetch_registers(int handle, uint8_t *registers) {
  return ioctl(handle, LSCPCI_IOCTL_FETCH_REG, registers);
}
*/

int lscpci_set_debug(int handle, uint16_t bits, uint16_t mask) {
  int result, val;

  val = (((uint32_t)mask) << 16) | bits;
  result = ioctl(handle, LSCPCI_IOCTL_SET_DEBUG, &val);
  if (result < 0) return result;
  return val;
}

int lscpci_get_buffer_pointers(int handle, lscpci_buffer_ptr_t *ptrs) {
  return ioctl(handle, LSCPCI_IOCTL_BUF_PTR, ptrs);
}
