#include "es_status_codes.h"

#define NUMBER_OF_ERROR_CODES 30
#define BUFFER_SIZE 50

char errorMsg[NUMBER_OF_ERROR_CODES][BUFFER_SIZE] =
{
	"No error occurred",
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
	"An unknown error occurred",
	"Enabling interrupts failed",
	"Getting DMA buffer failed",
	"Unlocking DMA failed",
	"Camera not found",
	"Measurement aborted",
	"Creating thread failed",
	"Setting thread priority failed",
	"Measurement is already running",
	"Disabling interrupt failed",
	"Memory not initialized",
	"Creating file failed",
	"First measurement not done",
	"Measurement is running"
};

/**
 * \brief Converts a es_status_codes to a human readable error message.
 * 
 * \param status es_status_codes
 * \return char*
 */
char* ConvertErrorCodeToMsg(es_status_codes status)
{
	if (status >= 0 && status < NUMBER_OF_ERROR_CODES)
	{
		return errorMsg[status];
	}
	else
	{
		return errorMsg[es_unknown_error];
	}
}
