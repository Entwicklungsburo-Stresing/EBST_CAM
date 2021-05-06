#include <stdint.h>
#include <stdio.h>
#include "es_status_codes.h"

#ifdef WIN32
#define ES_LOG(...) WDC_Err(__VA_ARGS__);
#endif

#ifdef __linux__
#define ES_LOG(...) fprintf(stderr, __VA_ARGS__);
#endif

es_status_codes setBitS0(uint32_t drvno, uint32_t bitnumber, uint16_t address);
es_status_codes resetBitS0(uint32_t drvno, uint32_t bitnumber, uint16_t address);