#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../shared_src/Board.h"
#include <unistd.h>

#define DRVNO 0
#define PIXEL 1088
#define COPY_TO_DISPLAY_BUFFER false

int main(int argc, char **argv)
{
	es_status_codes status = InitDriver();
	if(status != es_no_error) return status;
	status = InitBoard(DRVNO);
	if(status != es_no_error) return status;

	settings_struct.board_sel = 1;
	settings_struct.nos = 1000;
	settings_struct.nob = 1;
	settings_struct.camera_settings[DRVNO].bti_mode = bti_BTimer;
	settings_struct.camera_settings[DRVNO].sti_mode = sti_STimer;
	settings_struct.camera_settings[DRVNO].btime_in_microsec = 1000000;
	settings_struct.camera_settings[DRVNO].stime_in_microsec = 2000;
	settings_struct.camera_settings[DRVNO].camcnt = 1;
	settings_struct.camera_settings[DRVNO].pixel = PIXEL;
	settings_struct.camera_settings[DRVNO].sensor_type = sensor_type_hsvis;
	settings_struct.camera_settings[DRVNO].camera_system = camera_system_3030;

	status = InitMeasurement();
	if(status != es_no_error) return status;
	status = StartMeasurement();
	//wait for copy to user buffer done
	sleep(3);
	//print first 20 pixel
#if COPY_TO_DISPLAY_BUFFER
	//this method copies the data to a new buffer
	uint16_t* pdest = malloc(PIXEL * sizeof(uint16_t));
	status = ReturnFrame(DRVNO, 0, 0, 0, pdest, PIXEL);
#else
	//this method is directly accessing the user buffer
	uint16_t* pdest;
	status = GetAddressOfPixel(DRVNO, 0, 0, 0, 0, &pdest);
#endif
	if(status != es_no_error) return status;
	for (int i=0; i<20; i++)
		ES_LOG("Pixel %i: %u\n", i, pdest[i]);

	status = ExitDriver(DRVNO);
	return status;
}
