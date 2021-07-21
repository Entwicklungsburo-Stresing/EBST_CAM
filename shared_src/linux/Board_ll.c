#include "../Board_ll.h"
#include "../Board.h"
#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../../linux-driver/userspace/lscpcie.h"
#include "../../linux-driver/kernelspace/registers.h"
#include "../../linux-driver/userspace/local-config.h"

#define lscpcie_read_reg8(dev, addr, data) \
  do *data = *(uint8_t*)(((uint8_t *)dev->dma_reg) + addr); while (0)
#define lscpcie_read_reg16(dev, addr, data) \
  do *data = *(uint16_t*)(((uint8_t *)dev->dma_reg) + addr); while(0)
#define lscpcie_read_reg32(dev, addr, data) \
  do *data = *(uint32_t*)(((uint8_t *)dev->dma_reg) + addr); while(0)
#define lscpcie_write_reg8(dev, addr, data) \
  do *(uint8_t*) (((uint8_t *)dev->dma_reg) + addr) = data; while (0)
#define lscpcie_write_reg16(dev, addr, data) \
  do *(uint16_t*) (((uint8_t *)dev->dma_reg) + addr) = data; while (0)
#define lscpcie_write_reg32(dev, addr, data) \
  do *(uint32_t*) (((uint8_t *)dev->dma_reg) + addr) = data; while (0)

pthread_mutexattr_t attr;
pthread_mutex_t mutex;

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
	ES_LOG( "Setup DMA\n" );
	//on linux: driver numbers are 0 and 1, on windows 1 and 2
	struct dev_descr *dev = lscpcie_get_descriptor(drvno - 1);
	dev->control->bytes_per_interrupt = DMA_DMASPERINTR * aPIXEL[drvno] * sizeof(uint16_t);
	dev->control->used_dma_size = DMA_BUFFER_SIZE_IN_SCANS * aPIXEL[drvno] * sizeof(uint16_t);
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
	userBufferWritePos[drvno] = userBuffer[drvno];
	ES_LOG("user_buffer_write_pos %p\n", userBufferWritePos[drvno]);
	dev->control->read_pos = 0;
	dev->control->irq_count = 0;
	return;
}

void copyRestData(uint32_t drvno, size_t rest_in_bytes)
{
	struct dev_descr *dev = lscpcie_get_descriptor(drvno - 1);
	memcpy(userBufferWritePos[drvno], dev->mapped_buffer + dev->control->read_pos, rest_in_bytes);
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
	int result = lscpcie_driver_init();
	if(result < 0) return es_driver_init_failed;
	number_of_boards = result;
	return es_no_error;
}

es_status_codes _ExitDriver(uint32_t drvno)
{
	//on linux: driver numbers are 0 and 1, on windows 1 and 2
	drvno--;
	lscpcie_close(drvno);
	return es_no_error;
}

void* CopyDataToUserBuffer(void* param_drvno)
{
	//TODO: DIRTY HACK. setting drvno to param_drvno didn't work
	//uint32_t drvno = *((uint32_t*)param_drvno;
	uint32_t drvno = 1;
	ES_LOG("Copy data to user buffer started, user buffer: %p\n", userBuffer[drvno]);
	pthread_mutex_lock(&mutex);
	ssize_t bytes_to_read = sizeof(uint16_t) * aCAMCNT[drvno] * *Nospb * aPIXEL[drvno] * Nob;
	ES_LOG("bytes to read: %zd\n", bytes_to_read);
	ssize_t bytes_read = 0;
	ssize_t result;
	struct dev_descr *dev = lscpcie_get_descriptor(drvno - 1);
	ES_LOG("bytes per interrupt: %u\n", dev->control->bytes_per_interrupt);
	while (bytes_to_read && bytes_to_read >= dev->control->bytes_per_interrupt && !abortMeasurementFlag)
	{
		result = read(dev->handle, ((uint8_t *)userBuffer[drvno]) + bytes_read , bytes_to_read);
		ES_LOG("Copy to user buffer intterupt %u done, result: %zd\n", dev->control->irq_count, result);
		if (result < 0)
			return NULL;
		bytes_to_read -= result;
		bytes_read += result;
		userBufferWritePos[drvno] = (uint16_t*)(((uint8_t *)userBufferWritePos[drvno]) + result);
	}
	pthread_mutex_unlock(&mutex);
	ES_LOG("All copy to user buffer interrupts done\n");
	return NULL;
}

es_status_codes StartCopyDataToUserBufferThread(uint32_t drvno)
{
	ES_LOG("Start copy data to user buffer thread\n");
	if (pthread_mutexattr_init(&attr) == -1)
		return es_creating_thread_failed;
	if (pthread_mutex_init(&mutex, &attr) == -1)
		return es_creating_thread_failed;
	pthread_t tid;
	int err = pthread_create(&tid, NULL, &CopyDataToUserBuffer, (void*)&drvno);
	if(err)
		return es_creating_thread_failed;
	else
		return es_no_error;
}


