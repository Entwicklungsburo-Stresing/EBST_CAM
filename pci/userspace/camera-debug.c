/* ccd-debug.c
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

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "constants.h"
#include "lscpci.h"

struct debug_option_struct {
  char name[16];
  uint16_t flag;
};

const struct debug_option_struct debug_options[] = {
  { "start-stop", D_START_STOP },
  { "buffers",    D_BUFFERS    },
  { "buf-space",  D_BUFSPACE   },
  { "interrupt",  D_INTERRUPT  },
  { "polling",    D_POLLING    },
  { "readout",    D_READOUT    },
  { "ioctl",      D_IOCTL      },
  { "plx",        D_PLX        },
  { "module",     D_MODULE     },
  { "" }
};

void help(void) {
  int i = 0;
  printf("ccd-debug [options]\n");
  while (debug_options[i].name[0])
    printf("  --%s\n", debug_options[i++].name);
}

int debug_mode = 0, debug_mask;

int main(int argc, char **argv) {
  int arg_pos = 0, i, handle, result;
  char name[32], c;
  while (++arg_pos < argc) {
    if (!strcmp(argv[arg_pos], "--help")) {
      help();
      return 0;
    }
    if (!strcmp(argv[arg_pos], "--all")) {
      debug_mode
        = D_START_STOP | D_BUFFERS | D_INTERRUPT | D_READOUT | D_IOCTL | D_PLX
        | D_MODULE | D_BUFSPACE;

      break;
    }
    c = '+';
    if ((sscanf(argv[arg_pos], "--%s=%c", name, &c) != 2)
        &&
        (sscanf(argv[arg_pos], "--%s", name) != 1)) {
      fprintf(stderr, "invalid command option '%s'\n", argv[arg_pos]);
      return -1;
    }

    for (i = 0; debug_options[i].name[0]; i++)
      if (!strcmp(name, debug_options[i].name)) break;

    if (!debug_options[i].name) {
      fprintf(stderr, "undefined debug option in '%s'\n", argv[arg_pos]);
      return -2;
    }

    switch (c) {
    case '+':
      debug_mode |= debug_options[i].flag;
      debug_mask |= debug_options[i].flag;
      continue;

    case '-':
      debug_mode &= ~debug_options[i].flag;
      debug_mask |= debug_options[i].flag;
      continue;

    default:
      fprintf(stderr, "undefined debug flag '%c'\n", c);
      return -3;
    }
  }

  if ((handle = lscpci_open("/dev/lscpci0", O_RDWR)) < 0) {
    fprintf(stderr, "could not open camera device file\n");
    perror(0);
    return handle;
  }

  printf("setting debug flags 0x%04x in 0x%04x\n", debug_mode, debug_mask);

  if ((result = lscpci_set_debug(handle, debug_mode, debug_mask)) < 0) {
    fprintf(stderr, "setting debug resulted in error %d\n", result);
    return -4;
  }

  printf("read back 0x%04x\n", result);

  close(handle);

  return 0;
}
