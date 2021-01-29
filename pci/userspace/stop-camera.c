/* stop-ccd.c
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

int main(void) {
  int result, handle;

  if ((handle = lscpci_open("/dev/lscpci0", O_RDWR)) < 0) {
    fprintf(stderr, "could not open camera device file\n");
    perror(0);
    return handle;
  }

  if ((result = lscpci_stop(handle)) < 0) {
    fprintf(stderr, "could not stop camera\n");
    perror(0);
    return result;
  }

  return 0;
}
