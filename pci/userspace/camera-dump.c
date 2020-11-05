/* ccd-dump.c
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
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <termios.h>
#include <stropts.h>
#include <stdint.h>
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

char buf[] = "0x0000'0000";

const char *binary(unsigned char n) {
  int pos = 10, i, mask = 0x01;
  for (i = 0; i < 8; i++, mask <<= 1) {
    *(buf + pos--) = (n & mask) ? '1' : '0';
    if (i == 3) *(buf + pos--) = '\'';
  }
  return buf;
}

int main(int argc, char **argv) {
  uint8_t buffer[1024];
  int result, handle, i, once = 0;
  char c;

  handle = open("/dev/lscpci0", O_RDONLY);
  if (handle < 0) {
    fprintf(stderr, "error on opening /dev/lscpci0: ");
    perror(0);
    return -1;
  }

  if ((argc > 1) && !strcmp(argv[1], "-1")) once = 1;

  do {
    result = ioctl(handle, LSCPCI_IOCTL_DUMP_REG, buffer);

    if (result) {
      fprintf(stderr, "ccd dump returned error");
      perror(0);
      close(handle);
      return -2;
    }

    if (!once) printf("\033[2J");

    for (i = 4; i < 48; i++)
      printf("0x%02x (%s): 0x%02x %s\n", i, reg_names[i-4], buffer[i],
             binary(buffer[i]));

    if (once) break;

    if (!_kbhit()) {
      sleep(1);
      continue;
    }

    c = getc(stdin);

  } while (c != 'q');

  close(handle);

  return result;
}
