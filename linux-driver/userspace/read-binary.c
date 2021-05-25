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
