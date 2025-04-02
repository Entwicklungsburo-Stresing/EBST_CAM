/*****************************************************************//**
 * @file		read-binary.c
 * @author		Bernhard Lang
 * @date		05.11.2020
 * @copyright	Copyright 2020-2021 Bernhard Lang, University of Geneva, Entwicklungsbuero Stresing (http://www.stresing.de/). This program is free software; you can redistribute it and/or modify it under the terms of the LPGL-3.0 as published by the Free Software Foundation.
 *********************************************************************/

#include <stdio.h>
#include <stdint.h>

char half_hex(uint8_t x) {
  if (x < 10) return '0' + x;
  return 'a' + x - 10;
}
  
void hex_out(uint8_t x) {
  printf("0x%c%c", half_hex(x >> 4), half_hex(x & 0x0F));
}

int main(int argc, char **argv) {
  FILE *in = fopen(argv[1], "rb");
  int count = 0;
  while (!feof(in)) {
    hex_out(fgetc(in));
    printf(" ");
    count++;
  }

  printf("\nread %d bytes\n", count);

  fclose(in);

  return 0;
}
