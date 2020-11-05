/* ccd-register-play.c
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
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <termios.h>
#include <stropts.h>
#include "lscpci.h"
#include "../kernelspace/ioctl.h"
#include "../kernelspace/register-names.h"

typedef int bool;
#define true (1==1)
#define false (1==0)

int _kbhit() {
    static const int STDIN = 0;
    static bool initialized = false;

    if (! initialized) {
        // Use termios to turn off line buffering
        struct termios term;
        tcgetattr(STDIN, &term);
        term.c_lflag &= ~ICANON;
        tcsetattr(STDIN, TCSANOW, &term);
        setbuf(stdin, NULL);
        initialized = true;
    }

    int bytesWaiting;
    ioctl(STDIN, FIONREAD, &bytesWaiting);
    return bytesWaiting;
}

void modify(int handle) {
  char line[256];
  int address, value;

  printf("change? ");
  fflush(stdout);
  fgets(line, 255, stdin);
  if (sscanf(line, "0x%x 0x%x\n", &address, &value) != 2) {
    printf("invalid line\n");
    sleep(3);
    return;
  }
  if ((address > 0xFF) || (value > 0xFF)) {
    printf("invalid value\n");
    sleep(3);
    return;
  }
  printf("got 0x%02x 0x%02x, ok? [N/y]\n", address, value);
  fgets(line, 255, stdin);
  if (strcmp(line, "y\n")) {
    printf("dropping\n");
    sleep(3);
    return;
  }
  printf("setting 0x%02x to 0x%02x\n", address, value);

  if (lscpci_set_register(handle, address, value)) {
    printf("setting register returned error ");
    perror(0);
  }
  sleep(3);
}

char buf[] = "0x0000'0000";

const char *binary(unsigned char n) {
  int pos = 10, i, mask = 0x01;
  for (i = 0; i < 8; i++, mask <<= 1) {
    *(buf + pos--) = (n & mask) ? '1' : '0';
    if (i == 3) *(buf + pos--) = '\'';
  }
  return buf;
}

int main(void) {
  uint8_t buffer[128];
  int result, handle, i;
  char c;
  handle = open("/dev/lscpci0", O_RDONLY);
  if (handle < 0) {
    fprintf(stderr, "error on opening /dev/lscpci0: ");
    perror(0);
    return -1;
  }

  do {
    result = ioctl(handle, LSCPCI_IOCTL_DUMP_REG, buffer);

    if (result) {
      fprintf(stderr, "ccd dump %d\n", result);
      close(handle);
      return -2;
    }

    printf("\033[2J");

    for (i = 4; i < 48; i++)
      printf("0x%02x (%s): 0x%02x %s\n", i, reg_names[i-4], buffer[i],
             binary(buffer[i]));

    if (!_kbhit()) {
      sleep(1);
      continue;
    }

    switch (c = getc(stdin)) {
    case 'q': break;
    case 'm':
      modify(handle);
      continue;
    }

  } while (c != 'q');

  close(handle);

  return result;
}
