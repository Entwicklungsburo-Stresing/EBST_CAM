#pragma once

struct ffloopparams
{
	UINT32 board_sel;
	UINT32 exptus;
	UINT8 exttrig;
	UINT8 blocktrigger;
	UINT8 btrig_ch;
};

struct global_vars
{
	USHORT** userBuffer;
	WDC_DEVICE_HANDLE* hDev;
	//PWDC_DEVICE* pDev;
	ULONG* aPIXEL;
	ULONG* aCAMCNT;
	UINT32* Nospb;
};

struct global_settings
{
	UINT32 drvno; 
	UINT32 camcnt; 
	UINT32 pixel; 
	UINT32 xckdelay; 
	UINT32 sensor_type; 
	UINT32 _mshut; 
	UINT32 ExpTime; 
	UINT32 m_TOmodus; 
	UINT8 FFTMode;
	UINT32 FFTLines; 
	UINT16 number_of_regions;
	UINT32 lines;
	UINT8 keep_first; 
	UINT8* region_size;
	UINT8 Vfreq; 
	UINT32 lines_binning;
	UINT32 nos; 
	UINT32 nob; 
	UINT8 camera_system; 
	UINT16 trigger_mode; 
	UINT8 ADC_Mode; 
	UINT16 ADC_custom_pettern;
	UINT16 led_on; 
	UINT16 gain_high; 
	UINT8 gain; 
	UINT8 TrigMod; 
	UINT8 TOR_fkt; 
	UINT8 sti_mode; 
	UINT8 bti_mode; 
	UINT32 stime_in_microsec; 
	UINT32 btime_in_microsec; 
	UINT8 enable_gpx; 
	UINT32 gpx_offset; 
	UINT16 isIRSensor; 
	UINT8 Temp_level;
};