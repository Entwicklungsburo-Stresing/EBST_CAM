#include "crossPlattformBoard.h"
#include "enum.h"
#include "es_status_codes.h"

/**
 * \brief Stop S Timer.
 * 
 * Clear Bit30 of XCK-Reg: 0= timer off
 * \param drvno PCIe board identifier.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes StopSTimer( uint32_t drvno )
{
	ES_LOG("Stop S Timer, drv: %u\n", drvno);
	return setBitS0(drvno, XCKMSB_stimer_on, S0Addr_XCKMSB);
}

/**
 * \brief Use this function to abort measurement.
 * 
 * \param drv PCIe board identifier.
 * \return es_status_codes:
 *		-es_no_error
 *		-es_register_read_failed
 *		-es_register_write_failed
 */
es_status_codes abortMeasurement( uint32_t drv )
{
	ES_LOG("Abort Measurement\n");
	es_status_codes status = StopSTimer( drv );
	if (status != es_no_error) return status;
	status = resetBlockOn(drv);
	if (status != es_no_error) return status;
	status = resetMeasureOn(drv);
	if (status != es_no_error) return status;
	return SetDMAReset( drv );
}

/**
 * \brief Resets BlockOn bit in PCIEFLAGS and notifies UI about it.
 * 
 * \param drvno PCIe board identifier.
 * \return es_status_codes:
 *		- es_no_error
 */
es_status_codes resetBlockOn( uint32_t drvno )
{
	//notifyBlockDone( drvno );
	return resetBitS0( PCIEFLAGS_bitindex_BLOCKON, S0Addr_PCIEFLAGS, drvno );
}

/**
 * \brief Resets setMeasureOn bit in PCIEFLAGS and notifies UI about it.
 * 
 * \param drvno PCIe board identifier.
 * \return es_status_codes:
 *		- es_no_error
 */
es_status_codes resetMeasureOn( uint32_t drvno )
{
	//notifyMeasureDone( drvno );
	return resetBitS0( PCIEFLAGS_bitindex_MEASUREON, S0Addr_PCIEFLAGS, drvno );
}

/**
 * \brief 
 * 
 * \param drvno board number (=1 if one PCI board).
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes SetDMAReset( uint32_t drvno )
{
	/*
	ULONG BitMask = 0x1;
	ULONG RegisterValues = 0x1;
	es_status_codes status = SetDMAReg(RegisterValues, BitMask, DmaAddr_DCSR, drvno);
	if (status != es_no_error)
	{
		ErrorMsg("switch on the Initiator Reset for the DMA failed");
		return status;
	}
	// DCSR: reset the Iniator Reset 
	RegisterValues = 0x0;
	status = SetDMAReg(RegisterValues, BitMask, DmaAddr_DCSR, drvno);
	if (status != es_no_error)
		ErrorMsg("switch off the Initiator Reset for the DMA failed");
	return status;
	*/
	return es_no_error;
}