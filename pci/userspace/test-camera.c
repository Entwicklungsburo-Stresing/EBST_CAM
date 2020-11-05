/* test-ccd.c
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
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "lscpci.h"
#include "constants.h"
#include "../kernelspace/register-names.h"

char buffer[] = "b0000'0000:0000'0000:0000'0000:0000'0000";

const char *to_bit8(uint8_t val, char *buf) {
  char *pos = buf;
  uint8_t mask = 0x80;
  *pos++ = ':';
  while (mask) {
    *pos++ = (val & mask) ? '1' : '0';
    if (mask == 0x10) *pos++ = '\'';
    mask >>= 1;
  }
  return buf;
}

const char *to_bit32(uint32_t val) {
  to_bit8(val & 0xFF, &buffer[30]);
  val >>= 8;
  to_bit8(val & 0xFF, &buffer[20]);
  val >>= 8;
  to_bit8(val & 0xFF, &buffer[10]);
  val >>= 8;
  to_bit8(val & 0xFF, buffer);
  return buffer;
}

int main(void) {
  int result, handle, i, data_count, data_size;
  uint16_t *data, number_of_pixels;
  uint8_t *registers = malloc(64+128*4);
  uint32_t val32;

  if ((handle = lscpci_open("/dev/lscpci0", O_RDWR)) < 0) {
    fprintf(stderr, "could not open camera device file\n");
    perror(0);
    return handle;
  }
  printf("handle: %d\n", handle);

  if ((result = lscpci_number_of_pixels(handle)) < 0) {
    fprintf(stderr, "could not get number of pixels\n");
    perror(0);
    return result;
  }
  number_of_pixels = result;

  data_size = number_of_pixels * 2 * sizeof(uint16_t);
  data = malloc(data_size);

  printf("got number of pixels %d\n", number_of_pixels);

  memset(data, 0xBB, data_size);

  val32 = D_START_STOP | D_BUFFERS | D_INTERRUPT | D_READOUT | D_PLX;
  lscpci_set_debug(handle, val32, val32);

  result = lscpci_start(handle);
  if (result < 0) {
    fprintf(stderr, "lscpci_start returned %d (%d): ", result, errno);
    perror(0);
  } else {
    printf("camera started\n");

    if ((result = read(handle, (unsigned char *) data, data_size)) < 0) {
      fprintf(stderr, "error %d reading %d bytes of data (%d): ", errno,
              data_size, result);
      perror(0);
    } else {
      printf("got %d bytes of data\n", result);

      data_count = 0;
      do {
        printf("%3d", data_count);
        for (i = 0; i < 10; i++)
          printf(" 0x%04x", data[data_count++]);
        printf("\n");
      } while (data_count < number_of_pixels);
    }
  }

  if ((result = lscpci_fetch_registers(handle, registers)) < 0) {
    fprintf(stderr, "error %d reading registers: ", errno);
    perror(0);
    return result;
  }

  printf("camera\n");
  
  for (i = 4; i < 64; i++)
    printf("0x%02x (%s): 0x%02x %s\n", i, reg_names[i-4], registers[i],
           to_bit8(registers[i], &buffer[30]));

  printf("PLX\n");
  
  for (i = 0; i < 512; i += 4) {
    val32 = *(uint32_t*) (registers + 64 + i);
    printf("0x%03x 0x%08x %s\n", i, val32, to_bit32(val32));
  }

  close(handle);

  return 0;
}
