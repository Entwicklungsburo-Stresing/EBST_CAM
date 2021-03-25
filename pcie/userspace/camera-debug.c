/* camera-debug.c
 *
 * Copyright (C) 2010-2021 Bernhard Lang, University of Geneva
 * Copyright 2020-2021 Entwicklungsbuero Stresing (http://www.stresing.de/)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "constants.h"
#include "lscpcie.h"

struct debug_option_struct {
  char name[16];
  uint16_t flag;
};

const struct debug_option_struct debug_options[] = {
  { "pci",        D_PCI        },
  { "start-stop", D_START_STOP },
  { "readout",    D_READOUT    },
  { "interrupt",  D_INTERRUPT  },
  { "buffers",    D_BUFFERS    },
  { "ioctl",      D_IOCTL      },
  { "module",     D_MODULE     },
  { "mmap",       D_IOCTL      },
  { "proc",       D_MODULE     },
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
  int arg_pos = 0, i, handle, result, device = 0, temp;
  char name[32], c, *end;
  while (++arg_pos < argc) {
    if (!strcmp(argv[arg_pos], "--help")) {
      help();
      return 0;
    }
    if (!strcmp(argv[arg_pos], "--all")) {
      debug_mode
        = D_START_STOP | D_READOUT | D_INTERRUPT | D_BUFFERS | D_IOCTL
        | D_MODULE;

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
      temp = strtol(argv[arg_pos], &end, 10);
      if (end && *end != 0) {
        fprintf(stderr, "undefined debug flag '%c'\n", c);
        return -3;
      }
      device = temp;
    }
  }

  if ((handle = lscpcie_driver_init()) < 0) {
    fprintf(stderr, "could not initialise camera driver\n");
    perror(0);
    return handle;
  }

  if ((handle = lscpcie_open(device, 0)) < 0) {
    fprintf(stderr, "could not open camera device file\n");
    perror(0);
    return handle;
  }

  struct dev_descr *dev = lscpcie_get_descriptor(device);
  printf("setting debug flags 0x%04x in 0x%04x\n", debug_mode, debug_mask);

  if ((result = lscpcie_set_debug(dev, debug_mode, debug_mask)) < 0) {
    fprintf(stderr, "setting debug resulted in error %d\n", result);
    return -4;
  }

  lscpcie_close(device);

  return 0;
}
