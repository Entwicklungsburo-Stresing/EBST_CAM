/*****************************************************************//**
 * @file		CsimpleExample.c
 * @brief		Simple CLI example in C for using ESLSCDLL.dll or libESLSCDLL.so.
 * @details		This example is doing one measurement with DLLStartMeasurement_blocking(). During the measurement the library callbacks are used to print the current status. After the measurement the 200 first pixels of the first sample are printed.
 * @author		Florian Hahn
 * @date		30.10.2024
 * @copyright	Copyright Entwicklungsbuero Stresing. This software is released as public domain under the Unlicense.
*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "../ESLSCDLL/ESLSCDLL.h"

void measureStartCallback()
{
	int64_t sample, block;
	DLLGetCurrentScanNumber(0, &sample, &block);
	printf("Measure start callback, DLLGetCurrentScanNumber(): sample: %"PRId64", block %"PRId64"\n", sample, block);
}

void measureDoneCallback()
{
	int64_t sample, block;
	DLLGetCurrentScanNumber(0, &sample, &block);
	printf("Measure done callback, DLLGetCurrentScanNumber():  sample: %"PRId64", block %"PRId64"\n", sample, block);
}

void blockStartCallback(uint32_t blockIndex)
{
	printf("Block %"PRIu32" started\n", blockIndex);
	int64_t sample, block;
	DLLGetCurrentScanNumber(0, &sample, &block);
	printf("Block start callback, DLLGetCurrentScanNumber(): sample: %"PRId64", block %"PRId64"\n", sample, block);
}

void blockDoneCallback(uint32_t blockIndex)
{
	printf("Block %"PRIu32" ended\n", blockIndex);
	int64_t sample, block;
	DLLGetCurrentScanNumber(0, &sample, &block);
	printf("Block done callback, DLLGetCurrentScanNumber(): sample: %"PRId64", block %"PRId64"\n", sample, block);
}

void allBlocksDoneCallback(uint64_t measurementCnt)
{
	printf("Measurement cycle %"PRIu64" done\n", measurementCnt);
	int64_t sample, block;
	DLLGetCurrentScanNumber(0, &sample, &block);
	printf("All blocks done callback, DLLGetCurrentScanNumber(): sample: %"PRId64", block %"PRId64"\n", sample, block);
}

int main()
{
	printf("Initializing driver...\n");
	uint8_t _number_of_boards = 0;
	es_status_codes status = DLLInitDriver(&_number_of_boards);
	if (status != es_no_error)
	{
		printf("Initializing driver failed, error: %d, %s\n", status, DLLConvertErrorCodeToMsg(status));
		return status;
	}
	printf("Initializing driver done, found %"PRIu8" boards\n", _number_of_boards);

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
	settings.camera_settings[0].btime = 60000;
	settings.camera_settings[0].stime = 10;
	settings.camera_settings[0].timer_resolution_mode = timer_resolution_1us;
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
	printf("Initializing measurement...\n");
	status = DLLInitMeasurement(settings);
	if (status != es_no_error)
	{
		printf("Initializing measurement failed, error: %d, %s\n", status, DLLConvertErrorCodeToMsg(status));
		return status;
	}
	printf("Initializing measurement done\n");
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
	if(!pdest)
	{
		printf("Memory allocation failed for pdest\n");
		return es_allocating_memory_failed;
	}
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
