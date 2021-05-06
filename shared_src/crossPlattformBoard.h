#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "es_status_codes.h"
#include "crossPlattformBoard_ll.h"

es_status_codes StopSTimer( uint32_t drvno );
es_status_codes abortMeasurement( uint32_t drv );
es_status_codes resetBlockOn( uint32_t drvno );
es_status_codes resetMeasureOn( uint32_t drvno );
es_status_codes SetDMAReset( uint32_t drvno );

#ifdef __cplusplus
}
#endif
