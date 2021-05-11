#pragma once
//  Board_ll.h
//	all low level functions for managing Interfaceboard

***REMOVED***#define LSCPCIEJ_STRESING_DRIVER_NAME "lscpciej"
#define MAXPCIECARDS 5

#include "ccdctl.h"
#include <limits.h>
#include <process.h>
#include "Jungo/windrvr.h"
#include "Jungo/wdc_lib.h"
#include "Jungo/wdc_defs.h"
#include "wchar.h"
#include "lscpciej_lib.h"
#include "shared_src/ESLSCDLL_pro.h"
#include "shared_src/enum.h"
#include "shared_src/struct.h"
#include "shared_src/es_status_codes.h"

extern WDC_DEVICE_HANDLE* hDev;

//Low level API
es_status_codes SetS0Reg( ULONG Data, ULONG Bitmask, CHAR Address, UINT32 drvno );
es_status_codes SetS0Bit( ULONG bitnumber, CHAR Address, UINT32 drvno );
es_status_codes ResetS0Bit( ULONG bitnumber, CHAR Address, UINT32 drvno );
es_status_codes ReadLongIOPort( UINT32 drvno, UINT32 *DWData, ULONG PortOff );// read long from IO runreg
es_status_codes ReadLongS0( UINT32 drvno, UINT32 * DWData, ULONG PortOff );	// read long from space0
es_status_codes ReadLongDMA( UINT32 drvno, UINT32 * DWData, ULONG PortOff );
es_status_codes ReadByteS0( UINT32 drvno, BYTE *data, ULONG PortOff );	// read byte from space0
es_status_codes WriteLongIOPort( UINT32 drvno, UINT32 DataL, ULONG PortOff );// write long to IO runreg
es_status_codes WriteLongS0( UINT32 drvno, UINT32 DWData, ULONG PortOff );// write long to space0
es_status_codes WriteLongDMA( UINT32 drvno, UINT32 DWData, ULONG PortOff );
es_status_codes WriteByteS0( UINT32 drv, BYTE DataByte, ULONG PortOff ); // write byte to space0
es_status_codes SendFLCAM( UINT32 drvno, UINT8 maddr, UINT8 adaddr, UINT16 data );
