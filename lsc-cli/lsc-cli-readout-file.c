#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../shared_src/Board.h"
#include <unistd.h>

#include <errno.h>
#include "../linux-driver/userspace/lscpcie.h"

#define DRVNO 1
#define PIXEL 576
#define N_SCANS 500

int main(int argc, char **argv)
{
	es_status_codes status = InitDriver();
	if(status != es_no_error) return status;
	status = InitBoard(DRVNO);
	if(status != es_no_error) return status;

	struct global_settings settings_struct;
	memset(&settings_struct, 0, sizeof(settings_struct));
	settings_struct.drvno = DRVNO;
	settings_struct.board_sel = DRVNO;
	settings_struct.bti_mode = bti_BTimer;
	settings_struct.sti_mode = sti_I;
	settings_struct.btime_in_microsec = 1000000;
	settings_struct.nos = N_SCANS;
	settings_struct.nob = 1;
	settings_struct.camcnt = 2;
	settings_struct.pixel = PIXEL;
	settings_struct.FFTMode = full_binning;
	settings_struct.sensor_type = FFTsensor;
	settings_struct.FFTLines = 64;
	settings_struct.Vfreq = 7;

	status = InitMeasurement(settings_struct);
	if(status != es_no_error) return status;
	status = StartMeasurement();

	uint16_t *camera_data = calloc(2*PIXEL, sizeof(uint16_t));
	int bytes_to_read, bytes_read, result, i;
	int handle = lscpcie_get_descriptor(0)->handle;

	for (i = 0; i < N_SCANS; i++) {
		bytes_to_read = 2*PIXEL;
		bytes_read = 0;
		result = read(handle, ((uint8_t*)camera_data) + bytes_read,
					bytes_to_read);
		if (result < 0) {
		fprintf(stderr,
				"read on camera returned with error %d (errno: %d)\n",
				result, errno);
		return result;
		}
		bytes_to_read -= result;
		bytes_read += result;
	} while (bytes_to_read);

	for (int i=0; i<PIXEL; i++)
		ES_LOG("%i\t%u\t%u\n", i, camera_data[i], camera_data[i+PIXEL]);

	status = ExitDriver(DRVNO);

	free(camera_data);

	return status;
}
