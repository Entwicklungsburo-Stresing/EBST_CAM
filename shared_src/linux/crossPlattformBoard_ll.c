#include "../crossPlattformBoard_ll.h"
#include "../../lsc-gui/linux/userspace/lscpcie.h"

es_status_codes readRegister_32( uint32_t drvno, uint32_t* data, uint16_t address )
{
    struct dev_descr *dev = lscpcie_get_descriptor(drvno-1);
    lscpcie_read_reg32(dev, address, data);
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
}

es_status_codes writeRegister_16( uint32_t drvno, uint16_t data, uint16_t address )
{
    struct dev_descr *dev = lscpcie_get_descriptor(drvno-1);
    lscpcie_write_reg16(dev, address, data);
    return es_no_error;
}

es_status_codes writeRegister_8( uint32_t drvno, uint8_t data, uint16_t address )
{
    struct dev_descr *dev = lscpcie_get_descriptor(drvno-1);
    lscpcie_write_reg8(dev, address, data);
    return es_no_error;
}

es_status_codes readConfig_32( uint32_t drvno, uint32_t* data, uint16_t address )
{
    struct dev_descr *dev = lscpcie_get_descriptor(drvno-1);
    if(lscpcie_read_config32(dev, address, data))
        return es_register_read_failed;
    else
        return es_no_error;
}
es_status_codes writeConfig_32( uint32_t drvno, uint32_t data, uint16_t address )
{
    struct dev_descr *dev = lscpcie_get_descriptor(drvno-1);
    if(lscpcie_write_config32(dev, address, data))
        return es_register_read_failed;
    else
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

es_status_codes SetupDma( uint32_t drvno )
{
    ES_LOG( "Setup DMA\n" );
    struct dev_descr *dev = lscpcie_get_descriptor(drvno-1);
    dev->control->bytes_per_interrupt = DMA_DMASPERINTR * dev->control->number_of_pixels * sizeof(uint16_t);
    dev->control->used_dma_size = dev->s0->DMA_BUF_SIZE_IN_SCANS * dev->control->number_of_pixels * dev->control->number_of_cameras * sizeof(uint16_t);
	if (dev->control->used_dma_size > dev->control->dma_buf_size)
		dev->control->used_dma_size = dev->control->dma_buf_size;
    ES_LOG("dmas per interrupt is %d\n", dev->s0->DMAS_PER_INTERRUPT);
	ES_LOG("bytes per interrupt is %d\n", dev->control->bytes_per_interrupt);
	ES_LOG("number of scans is %d\n", dev->s0->NUMBER_OF_SCANS);
	ES_LOG("buf size in scans is %d\n", dev->s0->DMA_BUF_SIZE_IN_SCANS);
    return es_no_error;
}

uint64_t getDmaAddress( uint32_t drvno)
{
    struct dev_descr *dev = lscpcie_get_descriptor(drvno-1);
    return (uint64_t) dev->control->dma_physical_start;
}

es_status_codes enableInterrupt( uint32_t drvno )
{
    return es_no_error;
}
