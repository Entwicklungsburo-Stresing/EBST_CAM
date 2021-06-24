#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../shared_src/Board.h"
#include <unistd.h>

#define DRVNO 1
#define PIXEL 576
#define COPY_TO_DISPLAY_BUFFER false

int main(int argc, char **argv)
{
	es_status_codes status = InitDriver();
	if(status != es_no_error) return status;
	status = InitBoard(DRVNO);
	if(status != es_no_error) return status;

	settings_struct.drvno = DRVNO;
	settings_struct.board_sel = DRVNO;
	settings_struct.bti_mode = bti_BTimer;
	settings_struct.sti_mode = sti_I;
	settings_struct.btime_in_microsec = 1000000;
	settings_struct.nos = 500;
	settings_struct.nob = 1;
	settings_struct.camcnt = 2;
	settings_struct.pixel = PIXEL;
	settings_struct.FFTMode = full_binning;
	settings_struct.sensor_type = FFTsensor;
	settings_struct.FFTLines = 64;
	settings_struct.Vfreq = 7;

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
