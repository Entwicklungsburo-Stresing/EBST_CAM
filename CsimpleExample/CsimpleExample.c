﻿#include <stdio.h>
#include <stdlib.h>
#include "../ESLSCDLL/ESLSCDLL.h"

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
	printf("Initialising driver done, found %d boards\n", _number_of_boards);
	printf("Initialising board...\n");
	status = DLLInitBoard();
	if (status != es_no_error)
	{
		printf("Initialising board failed, error: %d, %s\n", status, DLLConvertErrorCodeToMsg(status));
		return status;
	}
	printf("Initialising Board done\n");

	struct measurement_settings settings;

	DLLInitSettingsStruct(&settings);

	settings.board_sel = 1;
	settings.nos = 1000;
	settings.nob = 1;
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
		printf("Pixel %i: %u\n", i, pdest[i]);
	}
	printf("Exiting driver...\n");
	status = DLLExitDriver();
	if (status != es_no_error)
	{
		printf("Exit driver failed, error: %d, %s\n", status, DLLConvertErrorCodeToMsg(status));
	}
	return status;
}