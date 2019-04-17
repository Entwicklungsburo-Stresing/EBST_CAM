#ifndef _PCI_STRINGS_H_
#define _PCI_STRINGS_H_

#include "wdc_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

DWORD DLLCALLCONV PciConfRegData2Str(WDC_DEVICE_HANDLE hDev,
    DWORD dwOffset, PCHAR pBuf, DWORD dwInLen, DWORD *pdwOutLen);
DWORD DLLCALLCONV PciExpressConfRegData2Str(WDC_DEVICE_HANDLE hDev,
    DWORD dwOffset, PCHAR pBuf, DWORD dwInLen, DWORD *pdwOutLen);
#ifdef __cplusplus
}
#endif

#endif /* _PCI_STRINGS_ */
