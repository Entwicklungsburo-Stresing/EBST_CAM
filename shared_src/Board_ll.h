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

extern WDC_DEVICE_HANDLE* hDev;
//extern PWDC_DEVICE pDev;

//DMA Addresses
enum dma_addresses
{
	DmaAddr_DCSR = 0x000,
	DmaAddr_DDMACR = 0x004,
	DmaAddr_WDMATLPA = 0x008,
	DmaAddr_WDMATLPS = 0x00C,
	DmaAddr_WDMATLPC = 0x010,
	DmaAddr_WDMATLPP = 0x014,
	DmaAddr_RDMATLPP = 0x018,
	DmaAddr_RDMATLPA = 0x01C,
	DmaAddr_RDMATLPS = 0x020,
	DmaAddr_RDMATLPC = 0x024,
};

enum PCIEFLAGS_bits
{
	PCIEFLAGS_bit_XCKI = 0x01,
	PCIEFLAGS_bit_INTTRIG = 0x02,
	PCIEFLAGS_bit_ENRSTIMERHW = 0x04,
	PCIEFLAGS_bit_INTRSR = 0x08,
	PCIEFLAGS_bit_BLOCKTRIG = 0x10,
	PCIEFLAGS_bit_MEASUREON = 0x20,
	PCIEFLAGS_bit_BLOCKON = 0x40,
	PCIEFLAGS_bitindex_XCKI = 0,
	PCIEFLAGS_bitindex_INTTRIG = 1,
	PCIEFLAGS_bitindex_ENRSTIMERHW = 2,

	PCIEFLAGS_bitindex_BLOCKTRIG = 4,
	PCIEFLAGS_bitindex_MEASUREON = 5,
	PCIEFLAGS_bitindex_BLOCKON = 6
};

enum IRQFLAGS_bits
{ 
IRQFLAGS_bitindex_INTRSR = 31
};

enum CTRLB_bits
{
	CTRLB_bit_STI0 = 0x01,
	CTRLB_bit_STI1 = 0x02,
	CTRLB_bit_STI2 = 0x04,
	CTRLB_bit_SHON = 0x08,
	CTRLB_bit_BTI0 = 0x10,
	CTRLB_bit_BTI1 = 0x20,
	CTRLB_bit_BTI2 = 0x40,
	CTRLB_bitindex_STI0 = 0,
	CTRLB_bitindex_STI1 = 1,
	CTRLB_bitindex_STI2 = 2,
	CTRLB_bitindex_SHON = 3,
	CTRLB_bitindex_BTI0 = 4,
	CTRLB_bitindex_BTI1 = 5,
	CTRLB_bitindex_BTI2 = 6
};

//PCIe Addresses
enum pcie_addresses
{
	PCIeAddr_devCap = 0x5C,
	PCIeAddr_devStatCtrl = 0x60
};

//S0 Addresses
enum s0_addresses
{
	S0Addr_DBR = 0x0,
	S0Addr_CTRLA = 0x4,
	S0Addr_CTRLB = 0x5,
	S0Addr_CTRLC = 0x6,
	S0Addr_XCKLL = 0x8,
	S0Addr_XCKLH = 0x9,
	S0Addr_XCKHL = 0xa,
	S0Addr_XCKMSB = 0xb,
	S0Addr_XCKCNTLL = 0xc,
	S0Addr_XCKCNTLH = 0xd,
	S0Addr_XCKCNTHL = 0xe,
	S0Addr_XCKCNTMSB = 0xf,
	S0Addr_PIXREGlow = 0x10,
	S0Addr_PIXREGhigh = 0x11,
	S0Addr_BTRIGREG = 0x12,
	S0Addr_FF_FLAGS = 0x13,
	S0Addr_FIFOCNT = 0x14,
	S0Addr_VCLKCTRL = 0x18,
	S0Addr_VCLKFREQ = 0x1b,
	S0Addr_EBST = 0x1C,
	S0Addr_SDAT = 0x20,
	S0Addr_SEC = 0x24,
	S0Addr_TOR = 0x28,
	S0Addr_ARREG = 0x2C,
	S0Addr_GIOREG = 0x30,
	S0Addr_DELAYEC = 0x34,
	S0Addr_IRQREG = 0x38,
	S0Addr_PCI = 0x3C,
	S0Addr_PCIEFLAGS = 0x40,
	S0Addr_NOS = 0x44,
	S0Addr_ScanIndex = 0x48,
	S0Addr_DmaBufSizeInScans = 0x04C,		// length in scans
	S0Addr_DMAsPerIntr = 0x050,
	S0Addr_NOB = 0x054,
	S0Addr_BLOCKINDEX = 0x058,
	S0Addr_CAMCNT = 0x05C,
	S0Addr_TDCCtrl = 0x60,
	S0Addr_TDCData = 0x64,
	S0Addr_ROI0 = 0x68,
	S0Addr_ROI1 = 0x6C,
	S0Addr_ROI2 = 0x70,
	S0Addr_XCKDLY = 0x74,
	S0Addr_BTIMER = 0x80,
	S0Addr_BDAT = 0x84,
	S0Addr_BEC = 0x88,
	S0Addr_BSLOPE = 0x8C
};

//Cam Addresses könnten später bei unterschiedlichen cam systemen vaariieren
enum cam_addresses
{
	maddr_cam = 0x00,
	maddr_adc = 0x01,
	maddr_ioctrl = 0x02,
	cam_adaddr_gain = 0x00,
	cam_adaddr_pixel = 0x01,
	cam_adaddr_trig_in = 0x02,
	cam_adaddr_ch = 0x03,
	cam_adaddr_vclk = 0x04,
	cam_adaddr_LEDoff = 0x05,
	cam_adaddr_coolTemp = 0x06,
	adc_ltc2271_regaddr_reset = 0x00,
	adc_ltc2271_regaddr_outmode = 0x02,
	adc_ltc2271_regaddr_custompattern_msb = 0x03,
	adc_ltc2271_regaddr_custompattern_lsb = 0x04,
	adc_ads5294_regaddr_reset = 0x00,
	adc_ads5294_regaddr_mode = 0x25,
	adc_ads5294_regaddr_custompattern = 0x26,
	adc_ads5294_regaddr_gain_1_to_4 = 0x2A,
	adc_ads5294_regaddr_gain_5_to_8 = 0x2B,
	adc_ads5294_regaddr_2wireMode = 0x46,
	adc_ads5294_regaddr_wordWiseOutput = 0x28,
	adc_ads5294_regaddr_ddrClkAlign = 0x42,
};

enum cam_messages
{
	adc_ltc2271_msg_reset = 0x80,
	adc_ltc2271_msg_normal_mode = 0x01,
	adc_ltc2271_msg_custompattern = 0x05,
	adc_ads5294_msg_reset = 0x01,
	adc_ads5294_msg_ramp = 0x40,
	adc_ads5294_msg_custompattern = 0x10,
	adc_ads5294_msg_2wireMode = 0x8401,
	adc_ads5294_msg_wordWiseOutput = 0x80FF,
	adc_ads5294_msg_ddrClkAlign = 0x60,
};

enum sti_mode
{
	sti_I = 0,
	sti_S1 = 1,
	sti_S2 = 2,
	sti_unused = 3,
	sti_STimer = 4,
	sti_ASL = 5
};

BOOL SetS0Reg( ULONG Data, ULONG Bitmask, CHAR Address, UINT32 drvno );
BOOL SetS0Bit( ULONG bitnumber, CHAR Address, UINT32 drvno );
BOOL ResetS0Bit( ULONG bitnumber, CHAR Address, UINT32 drvno );
BOOL ReadLongIOPort( UINT32 drvno, UINT32 *DWData, ULONG PortOff );// read long from IO runreg
BOOL ReadLongS0( UINT32 drvno, UINT32 * DWData, ULONG PortOff );	// read long from space0
BOOL ReadLongDMA( UINT32 drvno, UINT32 * DWData, ULONG PortOff );
BOOL ReadByteS0( UINT32 drvno, BYTE *data, ULONG PortOff );	// read byte from space0
BOOL WriteLongIOPort( UINT32 drvno, UINT32 DataL, ULONG PortOff );// write long to IO runreg
BOOL WriteLongS0( UINT32 drvno, UINT32 DWData, ULONG PortOff );// write long to space0
BOOL WriteLongDMA( UINT32 drvno, UINT32 DWData, ULONG PortOff );
BOOL WriteByteS0( UINT32 drv, BYTE DataByte, ULONG PortOff ); // write byte to space0
void SendFLCAM( UINT32 drvno, UINT8 maddr, UINT8 adaddr, UINT16 data );
