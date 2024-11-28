/*****************************************************************//**
 * @file   CsimpleExample.c
 * @brief  Simple CLI example in C for using ESLSCDLL.dll or libESLSCDLL.so.
 * 
 * This is example is doing one measurement with DLLStartMeasurement_blocking(). 
 * During the measurement the library callbacks are used to print the current status.
 * After the measurement the 200 first pixels of the first sample are printed.
 * @author Florian Hahn
 * @date   30.10.2024
 *********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "../ESLSCDLL/ESLSCDLL.h"

void measureStartCallback()
{
	int64_t sample, block;
	DLLGetCurrentScanNumber(0, &sample, &block);
	printf("Measure start callback, sample: %"PRId64", block %"PRId64"\n", sample, block);
}

void measureDoneCallback()
{
	int64_t sample, block;
	DLLGetCurrentScanNumber(0, &sample, &block);
	printf("Measure done callback, sample: %"PRId64", block %"PRId64"\n", sample, block);
}

void blockStartCallback()
{
	int64_t sample, block;
	DLLGetCurrentScanNumber(0, &sample, &block);
	printf("Block start callback, sample: %"PRId64", block %"PRId64"\n", sample, block);
}

void blockDoneCallback()
{
	int64_t sample, block;
	DLLGetCurrentScanNumber(0, &sample, &block);
	printf("Block done callback, sample: %"PRId64", block %"PRId64"\n", sample, block);
}

void allBlocksDoneCallback()
{
	int64_t sample, block;
	DLLGetCurrentScanNumber(0, &sample, &block);
	printf("All blocks done callback, sample: %"PRId64", block %"PRId64"\n", sample, block);
}

int main()
{
	printf("Initialising driver...\n");
	uint8_t _number_of_boards = 0;
	es_status_codes status = DLLInitDriver(&_number_of_boards);
	if (status != es_no_error)
	{
		printf("Initialising driver failed, error: %d, %s\n", status, DLLConvertErrorCodeToMsg(status));
		return status;
	}
	printf("Initialising driver done, found %"PRIu8" boards\n", _number_of_boards);
	printf("Initialising board...\n");
	status = DLLInitBoard();
	if (status != es_no_error)
	{
		printf("Initialising board failed, error: %d, %s\n", status, DLLConvertErrorCodeToMsg(status));
		return status;
	}
	printf("Initialising Board done\n");

	DLLSetAllBlocksDoneHook(allBlocksDoneCallback);
	DLLSetBlockDoneHook(blockDoneCallback);
	DLLSetBlockStartHook(blockStartCallback);
	DLLSetMeasureDoneHook(measureDoneCallback);
	DLLSetMeasureStartHook(measureStartCallback);

	struct measurement_settings settings;

	DLLInitSettingsStruct(&settings);

	settings.board_sel = 1;
	settings.nos = 567;
	settings.nob = 10;
	settings.camera_settings[0].bti_mode = bti_BTimer;
	settings.camera_settings[0].sti_mode = sti_STimer;
	settings.camera_settings[0].sslope = sslope_pos;
	settings.camera_settings[0].bslope = bslope_pos;
	settings.camera_settings[0].btime_in_microsec = 60000;
	settings.camera_settings[0].stime_in_microsec = 10;
	settings.camera_settings[0].camcnt = 1;
	settings.camera_settings[0].pixel = 1024;
	settings.camera_settings[0].sensor_type = sensor_type_hsvis;
	settings.camera_settings[0].camera_system = camera_system_3030;
	settings.camera_settings[0].dac_output[0][0] = 55000;
	settings.camera_settings[0].dac_output[0][1] = 55000;
	settings.camera_settings[0].dac_output[0][2] = 55000;
	settings.camera_settings[0].dac_output[0][3] = 55000;
	settings.camera_settings[0].dac_output[0][4] = 55000;
	settings.camera_settings[0].dac_output[0][5] = 55000;
	settings.camera_settings[0].dac_output[0][6] = 55000;
	settings.camera_settings[0].dac_output[0][7] = 55000;
	settings.camera_settings[0].dma_buffer_size_in_scans = 1000;
	settings.camera_settings[0].use_software_polling = 1;
	printf("Setting global settings...\n");
	status = DLLSetGlobalSettings(settings);
	if (status != es_no_error)
	{
		printf("Setting global settings failed, error: %d, %s\n", status, DLLConvertErrorCodeToMsg(status));
		return status;
	}
	printf("Setting global settings done\n");
	printf("Initialising measurement...\n");
	status = DLLInitMeasurement();
	if (status != es_no_error)
	{
		printf("Initialising measurement failed, error: %d, %s\n", status, DLLConvertErrorCodeToMsg(status));
		return status;
	}
	printf("Initialising measurement done\n");
	printf("Starting measurement...\n");
	status = DLLStartMeasurement_blocking();
	if (status != es_no_error)
	{
		printf("Measurement failed, error: %d, %s\n", status, DLLConvertErrorCodeToMsg(status));
		return status;
	}
	printf("Measurement done\n");
	printf("Getting the first sample...\n");
	uint16_t* pdest = malloc(settings.camera_settings[0].pixel * sizeof(uint16_t));
	status = DLLCopyOneSample(0, 0, 0, 0, pdest);
	if (status != es_no_error)
	{
		printf("Copy one sample failed, error: %d, %s\n", status, DLLConvertErrorCodeToMsg(status));
		return status;
	}
	printf("Copy one sample done\n");
	for (int i = 0; i < 200; i++)
	{
		printf("Pixel %i: %"PRIu16"\n", i, pdest[i]);
	}
	printf("Exiting driver...\n");
	status = DLLExitDriver();
	if (status != es_no_error)
	{
		printf("Exit driver failed, error: %d, %s\n", status, DLLConvertErrorCodeToMsg(status));
	}
	return status;
}
