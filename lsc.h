#ifndef LSC_H
#define LSC_H

#include <stdint.h>

class Lsc
{
public:
    Lsc();
    ~Lsc();
    void initMeasurement();
    void startMeasurement();
    void returnFrame(uint32_t board, uint32_t sample, uint32_t block, uint16_t camera, uint16_t *pdest, uint32_t length);
};

#endif // LSC_H
