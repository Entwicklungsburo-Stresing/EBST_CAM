#include "../crossPlattformBoard_ll.h"
#include "../../lsc-gui/linux/userspace/lscpcie.h"

es_status_codes readRegister_32( uint32_t drvno, uint32_t* data, uint16_t address )
{
    struct dev_descr *dev = lscpcie_get_descriptor(drvno-1);
    //lscpcie_read_reg32(dev, address, data);
    *data = *(uint32_t*)(((uint8_t *)dev->dma_reg) + address);
    return es_no_error;
}

es_status_codes readRegister_16( uint32_t drvno, uint16_t* data, uint16_t address )
{
    struct dev_descr *dev = lscpcie_get_descriptor(drvno-1);
    lscpcie_read_reg16(dev, address, data);
    return es_no_error;
}

es_status_codes readRegister_8( uint32_t drvno, uint8_t* data, uint16_t address )
{
    struct dev_descr *dev = lscpcie_get_descriptor(drvno-1);
    lscpcie_read_reg8(dev, address, data);
    return es_no_error;
}

es_status_codes writeRegister_32( uint32_t drvno, uint32_t data, uint16_t address )
{
    struct dev_descr *dev = lscpcie_get_descriptor(drvno-1);
    lscpcie_write_reg32(dev, address, data);
    return es_no_error;
    /*
    if(lscpcie_write_reg32(dev, address, data))
        return es_register_write_failed;
    else
        return es_no_error;*/
}

es_status_codes writeRegister_16( uint32_t drvno, uint16_t data, uint16_t address )
{
    struct dev_descr *dev = lscpcie_get_descriptor(drvno-1);
    lscpcie_write_reg16(dev, address, data);
    return es_no_error;
    /*if(lscpcie_write_reg16(dev, address, data))
        return es_register_write_failed;
    else
        return es_no_error;*/
}

es_status_codes writeRegister_8( uint32_t drvno, uint8_t data, uint16_t address )
{
    struct dev_descr *dev = lscpcie_get_descriptor(drvno-1);
    lscpcie_write_reg8(dev, address, data);
    return es_no_error;
}

/**
 * \brief Check drvno for beeing legit
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
	/*else if (hDev[drvno] == INVALID_HANDLE_VALUE)
	{
		WDC_Err("Handle is invalid of drvno: %i", drvno);
		return es_invalid_driver_handle;
	}*/
	else
		return es_no_error;
}

/**
 * @brief Not implemented: Get the free and installed memory info.
 * 
 * @param pmemory_all how much is installed
 * @param pmemory_free how much is free
 */
void FreeMemInfo( uint64_t *pmemory_all, uint64_t *pmemory_free )
{
    //TODO implement me
    *pmemory_all = -1;
    *pmemory_free = -1;
    return;
}

es_status_codes SetupPCIE_DMA( uint32_t drvno )
{
    // TODO implement me
    return es_no_error;
}

es_status_codes SetTLPS( uint32_t drvno, uint32_t pixel )
{
    // TODO implement me
    return es_no_error;
}
