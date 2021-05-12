#pragma once
//  Board_ll.h
//	all low level functions for managing Interfaceboard

#define LSCPCIEJ_DEFAULT_LICENSE_STRING "872759e7d022a2499e5dcb42c25d1f889fdf524932d756df481fa728700d.WD1400_64_NL_Entwicklungsbuero_Stresing"
#define LSCPCIEJ_STRESING_DRIVER_NAME "lscpciej"
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

es_status_codes ReadLongIOPort( UINT32 drvno, UINT32 *DWData, ULONG PortOff );// read long from IO runreg

es_status_codes WriteLongIOPort( UINT32 drvno, UINT32 DataL, ULONG PortOff );// write long to IO runreg

