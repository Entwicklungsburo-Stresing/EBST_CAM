#include "es_status_codes.h"

char errorMsg[100][50] =
{
	"No error occured",
	"No PCIe board found",
	"Driver initialization failed",
	"Initiating WDC debugging failed",
	"Setting driver name failed",
	"Invalid pixel count",
	"Invalid driver number",
	"Getting device info failed",
	"Opening device failed",
	"Invalid driver handle",
	"Register read failed",
	"Register write failed",
	"No space0 found",
	"Allocating user memory failed",
	"Not enough RAM",
	"Parameter out of bounds",
	"An unknown error occured",
	"Enabling interrupts failed",
	"Getting dma buffer failed",
	"Unlocking dma failed",
	"Camera not found",
	"Measurement aborted",
	"Creating thread failed",
	"Setting thread priority failed",
	"Measurement is already running",
	"Disabling interrupt failed"
};

char* ConvertErrorCodeToMsg(es_status_codes status)
{
	return errorMsg[status];
}
