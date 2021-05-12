#include "../crossPlattformBoard_ll.h"

es_status_codes setBitS0(uint32_t drvno, uint32_t bitnumber, uint16_t address)
{
    return es_no_error;
}

/**
 * Check drvno for beeing legit
 * 
 * \param drvno driver number
 * \return es_status_codes:
 *		- es_invalid_driver_number
 *		- es_invalid_driver_handle
 *		- es_no_error
 */
es_status_codes checkDriverHandle(uint32_t drvno)
{
	if ((drvno < 1) || (drvno > 2))
		return es_invalid_driver_number;
	else if (hDev[drvno] == INVALID_HANDLE_VALUE)
	{
		WDC_Err("Handle is invalid of drvno: %i", drvno);
		return es_invalid_driver_handle;
	}
	else
		return es_no_error;
}