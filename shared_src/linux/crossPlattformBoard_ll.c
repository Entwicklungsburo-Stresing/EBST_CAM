#include "../crossPlattformBoard_ll.h"
#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../linux-driver/userspace/lscpcie.h"
#include "../../linux-driver/kernelspace/registers.h"
#include "../../linux-driver/userspace/local-config.h"

/* Acquire one block of data. Poll first XCKMSB /RS for being low and data
   being present in the buffer. Loop over if less than the needed data has
   been copied. */
   /*
int lscpcie_acquire_block_poll(struct dev_descr *dev, uint8_t *data,
            size_t n_scans) {
    int result, bytes_read = 0;
    size_t block_size =
        dev->control->number_of_pixels * dev->control->number_of_cameras
        * sizeof(pixel_t) * n_scans;

    result = lscpcie_start_block(dev);
    if (result < 0)
        return result;

    do {
        if (dev->s0->XCK.dword & (1 << XCK_RS))
            continue;

        if (dev->control->read_pos == dev->control->write_pos)
            continue;

        result = fetch_mapped_data(dev, data + bytes_read,
                    block_size - bytes_read);
        if (result < 0)
            return result;

        bytes_read += result;
        fprintf(stderr, "got %d bytes of data (irq %d)\n", result,
            dev->control->irq_count);
    } while (bytes_read < block_size);

        result = lscpcie_end_block(dev);
    if (result < 0)
        return result;

    return bytes_read;
}
*/

es_status_codes readRegister_32( uint32_t drvno, uint32_t* data, uint16_t address )
{
    //on linux: driver numbers are 0 and 1, on windows 1 and 2
    drvno--;
    struct dev_descr *dev = lscpcie_get_descriptor(drvno);
    if (!dev) return es_register_read_failed;
    lscpcie_read_reg32(dev, address, data);
    return es_no_error;
}

es_status_codes readRegister_16( uint32_t drvno, uint16_t* data, uint16_t address )
{
    //on linux: driver numbers are 0 and 1, on windows 1 and 2
    drvno--;
    struct dev_descr *dev = lscpcie_get_descriptor(drvno);
    if (!dev) return es_register_read_failed;
    lscpcie_read_reg16(dev, address, data);
    return es_no_error;
}

es_status_codes readRegister_8( uint32_t drvno, uint8_t* data, uint16_t address )
{
    //on linux: driver numbers are 0 and 1, on windows 1 and 2
    drvno--;
    struct dev_descr *dev = lscpcie_get_descriptor(drvno);
    if (!dev) return es_register_read_failed;
    lscpcie_read_reg8(dev, address, data);
    return es_no_error;
}

es_status_codes writeRegister_32( uint32_t drvno, uint32_t data, uint16_t address )
{
    //on linux: driver numbers are 0 and 1, on windows 1 and 2
    drvno--;
    struct dev_descr *dev = lscpcie_get_descriptor(drvno);
    if (!dev) return es_register_write_failed;
    lscpcie_write_reg32(dev, address, data);
    return es_no_error;
}

es_status_codes writeRegister_16( uint32_t drvno, uint16_t data, uint16_t address )
{
    //on linux: driver numbers are 0 and 1, on windows 1 and 2
    drvno--;
    struct dev_descr *dev = lscpcie_get_descriptor(drvno);
    if (!dev) return es_register_write_failed;
    lscpcie_write_reg16(dev, address, data);
    return es_no_error;
}

es_status_codes writeRegister_8( uint32_t drvno, uint8_t data, uint16_t address )
{
    //on linux: driver numbers are 0 and 1, on windows 1 and 2
    drvno--;
    struct dev_descr *dev = lscpcie_get_descriptor(drvno);
    if (!dev) return es_register_write_failed;
    lscpcie_write_reg8(dev, address, data);
    return es_no_error;
}

/**
 * @brief Read long (32 bit) from runtime register of PCIe board.
 *  
 * This function reads the memory mapped data , not the I/O Data. Reads data from PCIe conf space.
 * 
 * @param drvno board number (=1 if one PCI board)
 * @param data pointer to where data is stored
 * @param address offset of register (count in bytes)
 * @return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes readConfig_32( uint32_t drvno, uint32_t* data, uint16_t address )
{
    //on linux: driver numbers are 0 and 1, on windows 1 and 2
    drvno--;
    struct dev_descr *dev = lscpcie_get_descriptor(drvno);
    if (!dev) return es_register_read_failed;
    if(lscpcie_read_config32(dev, address, data))
        return es_register_read_failed;
    else
        return es_no_error;
}

es_status_codes writeConfig_32( uint32_t drvno, uint32_t data, uint16_t address )
{
    //on linux: driver numbers are 0 and 1, on windows 1 and 2
    drvno--;
    struct dev_descr *dev = lscpcie_get_descriptor(drvno);
    if (!dev) return es_register_write_failed;
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
    //on linux: driver numbers are 0 and 1, on windows 1 and 2
    drvno--;
	if (drvno > 1)
		return es_invalid_driver_number;
    struct dev_descr *dev = lscpcie_get_descriptor(drvno);
    if (!dev)
        return es_invalid_driver_handle;
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
    //on linux: driver numbers are 0 and 1, on windows 1 and 2
    drvno--;
    ES_LOG( "Setup DMA\n" );
    struct dev_descr *dev = lscpcie_get_descriptor(drvno);
    dev->control->bytes_per_interrupt = DMA_DMASPERINTR * dev->control->number_of_pixels * dev->control->number_of_cameras * sizeof(uint16_t);
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
    //on linux: driver numbers are 0 and 1, on windows 1 and 2
    drvno--;
    struct dev_descr *dev = lscpcie_get_descriptor(drvno);
    return (uint64_t) dev->control->dma_physical_start;
}

es_status_codes enableInterrupt( uint32_t drvno )
{
    return es_no_error;
}

void ResetBufferWritePos(uint32_t drvno)
{
    //on linux: driver numbers are 0 and 1, on windows 1 and 2
    struct dev_descr *dev = lscpcie_get_descriptor(drvno - 1);
	dev->control->write_pos = 0;
    dev->control->user_buffer_write_pos = userBuffer[drvno];
    ES_LOG("user_buffer_write_pos %p\n", dev->control->user_buffer_write_pos);
	dev->control->read_pos = 0;
	dev->control->irq_count = 0;
    return;
}

void copyRestData(size_t rest_in_bytes)
{
    return;
}

es_status_codes _InitBoard(uint32_t drvno)
{
    //on linux: driver numbers are 0 and 1, on windows 1 and 2
    drvno--;
    // open /dev/lscpcie<n>
    int result = lscpcie_open(drvno, 0, 1);
    if(result < 0) return es_open_device_failed;
    return es_no_error;
}

es_status_codes _InitDriver()
{
    if(lscpcie_driver_init() < 0) return es_driver_init_failed;
    else return es_no_error;
}

es_status_codes _ExitDriver(uint32_t drvno)
{
    //on linux: driver numbers are 0 and 1, on windows 1 and 2
    drvno--;
    lscpcie_close(drvno);
    return es_no_error;
}

/**
 * \brief Returns the address of a pixel located in userBuffer.
 * 
 * \param drvno indentifier of PCIe card
 * \param pixel position in one scan (0...(PIXEL-1))
 * \param sample position in samples (0...(nos-1))
 * \param block position in blocks (0...(nob-1))
 * \param CAM position in camera count (0...(CAMCNT-1))
 * \param Pointer to get address
 * \return es_status_codes
 *		- es_no_error
 *		- es_parameter_out_of_range
 */
es_status_codes GetAddressOfPixel( uint32_t drvno, uint16_t pixel, uint32_t sample, uint32_t block, uint16_t CAM, uint16_t** address )
{
	uint64_t index = 0;
	es_status_codes status = GetIndexOfPixel(drvno, pixel, sample, block, CAM, &index);
    drvno--;
    struct dev_descr *dev = lscpcie_get_descriptor(drvno);
	if (status != es_no_error) return status;
	*address = &((uint16_t*)dev->mapped_buffer)[index];
	return status;
}
