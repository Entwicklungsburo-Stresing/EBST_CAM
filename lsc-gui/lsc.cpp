#include "lsc.h"
#include <cstdint>
#include <sstream>
#include "../shared_src/crossPlattformBoard.h"

#define memory_barrier() asm volatile ("" : : : "memory")
#define CFG_BTIMER_IN_US      500000
#define CFG_STIMER_IN_US      400

Lsc::Lsc()
{
    driverInstructions = "On Linux: Run 'sudo insmod lscpcie.ko' and 'sudo chmod 666 /dev/lscpcie0' before running this application";
}
Lsc::~Lsc()
{
    ExitDriver(0);
}

/**
 * @brief Inits PCIe board driver.
 * @return es_status_codes:
 *      - es_no_error
 *      - es_driver_init_failed
 */
es_status_codes Lsc::initDriver()
{
    return InitDriver();
}

/**
 * @brief Inits PCIe board.
 * @return es_status_codes:
 *      - es_no_error
 *      - es_open_device_failed
 *      - es_getting_dma_buffer_failed
 *      - es_unknown_error
 */
es_status_codes Lsc::initPcieBoard(uint32_t drvno)
{
    return InitBoard(drvno);
}

/**
 * @brief Init Measurement.
 * @param settings_struct
 * @return
 */
es_status_codes Lsc::initMeasurement(struct global_settings settings_struct)
{
    info.n_blocks = settings_struct.nob;
    info.n_scans = settings_struct.nos;
    info.trigger_mode = xck;
    return InitMeasurement(settings_struct);
}

/* Acquire one block of data. Poll first XCKMSB /RS for being low and data
   being present in the buffer. Loop over if less than the needed data has
   been copied. */
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

es_status_codes Lsc::startMeasurement()
{
    emit measureStart();
    //set measure on
    //lscpcie_start_scan(info.dev);
    //TODO: use crossPlattformBoard StartMeasurment

    info.mem_size = info.dev->control->number_of_pixels
        * info.dev->control->number_of_cameras * info.n_blocks
        * info.n_scans * sizeof(pixel_t);
    info.data = (pixel_t*)malloc(info.mem_size);
    if (!info.data)
    {
        fprintf(stderr, "failed to allocate %d bytes of memory\n",
            info.mem_size);
        return es_allocating_memory_failed;
    }
    int result, bytes_read = 0;
    do
    {
        // wait for block trigger signal
        if (!(info.dev->s0->CTRLA & (1 << CTRLA_TSTART)))
            continue;
        emit blockStart();
        result = lscpcie_acquire_block_poll(info.dev, (uint8_t *) info.data + bytes_read, info.n_scans);
        if (result < 0)
        {
            fprintf(stderr, "error %d when acquiring block\n", result);
            return es_unknown_error;
        }
        bytes_read += result;
        fprintf(stderr, "have now %d bytes\n", bytes_read);
        emit blockDone();
    } while (bytes_read < info.mem_size && ! abortMeasurementFlag);

    fprintf(stderr, "finished measurement\n");

    result = lscpcie_end_acquire(info.dev);
    if (result)
        fprintf(stderr, "error %d when finishing acquisition\n", result);

    emit measureDone();
    return es_no_error;
}

es_status_codes Lsc::returnFrame(uint32_t board, uint32_t sample, uint32_t block, uint16_t camera, uint16_t *pdest, uint32_t length)
{
    int n = info.dev->control->number_of_pixels;
    int nob = info.n_blocks;
    int nos = info.n_scans;
    int camcnt = info.dev->control->number_of_cameras;
    int offset = camera * n + sample * camcnt * n + block * nos * camcnt * n;
    memcpy( pdest, &((uint16_t*)info.data)[offset], length * sizeof( uint16_t ) );
    return es_no_error;
}

std::string Lsc::dumpS0Registers()
{
    enum N { number_of_registers = 41 };
    std::string register_names[number_of_registers] = {
        "DBR \t\t",
        "CTRLA \t\t",
        "XCKLL \t\t",
        "XCKCNTLL\t\t",
        "PIXREG \t\t",
        "FIFOCNT \t\t",
        "VCLKCTRL\t\t",
        "'EBST' \t\t",
        "DAT \t\t",
        "EC \t\t",
        "TOR \t\t",
        "ARREG \t\t",
        "GIOREG \t\t",
        "nc\t\t",
        "IRQREG\t\t",
        "PCI board version\t",
        "R0 PCIEFLAGS\t",
        "R1 NOS\t\t",
        "R2 SCANINDEX\t",
        "R3 DMABUFSIZE\t",
        "R4 DMASPERINTR\t",
        "R5 BLOCKS\t\t",
        "R6 BLOCKINDEX\t",
        "R7 CAMCNT\t",
        "R8 GPX Ctrl\t\t",
        "R9 GPX Data\t",
        "R10 ROI 0\t\t",
        "R11 ROI 1\t\t",
        "R12 ROI 2\t\t",
        "R13 XCKDLY\t",
        "R14 ADSC\t\t",
        "R15 LDSC\t\t",
        "R16 BTimer\t\t",
        "R17 BDAT\t\t",
        "R18 BEC\t\t",
        "R19 BFLAGS\t",
        "R20 TR1\t\t",
        "R21 TR2\t\t",
        "R22 TR3\t\t",
        "R23 TR4\t\t",
        "R24 TR5\t\t"
    }; //Look-Up-Table for the S0 Registers
    uint32_t data = 0;
    std::stringstream stream;
    for (int i = 0; i < number_of_registers; i++)
    {
        data = *(uint32_t*) (((const uint8_t *) info.dev->s0) + i * 4);
        stream  << register_names[i]
                << "0x"
                << std::hex << data
                << '\n';
    }
    return stream.str();
}

std::string Lsc::dumpDmaRegisters()
{
    enum N { number_of_registers = 18 };
    std::string register_names[number_of_registers] = {
        "DCSR\t\t",
        "DDMACR\t\t",
        "WDMATLPA\t",
        "WDMATLPS\t",
        "WDMATLPC\t",
        "WDMATLPP\t",
        "RDMATLPP\t\t",
        "RDMATLPA\t\t",
        "RDMATLPS\t\t",
        "RDMATLPC\t\t",
        "WDMAPERF\t",
        "RDMAPERF\t\t",
        "RDMASTAT\t\t",
        "NRDCOMP\t\t",
        "RCOMPDSIZW\t",
        "DLWSTAT\t\t",
        "DLTRSSTAT\t\t",
        "DMISCCONT\t"
    }; //Look-Up-Table for the DMA Registers
    uint32_t data = 0;
    std::stringstream stream;
    for (int i = 0; i < number_of_registers; i++)
    {
        data = *(uint32_t*) (((const uint8_t *) info.dev->dma_reg) + i * 4);
        stream  << register_names[i]
                << "0x"
                << std::hex << data
                << '\n';
    }
    return stream.str();
}

std::string Lsc::dumpTlp()
{
    uint32_t data = 0;
    std::stringstream stream;
    stream  << "PAY_LOAD values:\t\t0 = 128 bytes\n\t\t\t1 = 256 bytes\n\t\t\t2 = 512 bytes\n";
    lscpcie_read_config32(info.dev, PCIeAddr_devCap, &data);
    data &= 0x7;
    stream  << "PAY_LOAD Supported:\t\t0x"
            << std::hex << data
            << '\n';
    lscpcie_read_config32(info.dev, PCIeAddr_devStatCtrl, &data);
    uint32_t actpayload = (data >> 5) & 0x07;
    stream << "PAY_LOAD:\t\t\t0x"
           << std::hex << actpayload
           << '\n';
    lscpcie_read_config32(info.dev, PCIeAddr_devStatCtrl, &data);
    data >>= 12;
    data &= 0x7;
    stream << "MAX_READ_REQUEST_SIZE:\t0x"
           << std::hex << data
           << '\n'
           << "Number of pixels:\t\t"
           << std::dec << info.dev[0].control->number_of_pixels
           << "\n";
    switch (actpayload) {
    case 0: data = 0x20;  break;
    case 1: data = 0x40;  break;
    case 2: data = 0x80;  break;
    case 3: data = 0x100; break;
    }
    stream << "TLP_SIZE is:\t\t"
           << std::dec << data
           << " DWORDs\n\t\t\t="
           << std::dec << data*4
           << " BYTEs\n";
    data = info.dev->dma_reg->WDMATLPS;
    stream << "TLPS in DMAReg is:\t\t"
           << std::dec << data
           << "\n";
    data = (info.dev[0].control->number_of_pixels - 1) / (data * 2) + 1;
    stream << "number of TLPs should be:\t"
           << std::dec << data
           << "\n";
    data = info.dev->dma_reg->WDMATLPC;
    stream << "number of TLPs is:\t\t"
           << std::dec << data
           << "\n";
    return stream.str();
}

void Lsc::setTorOut(uint8_t torOut)
{
    _torOut = torOut;
    info.dev->s0->TOR = _torOut << TOR_TO_pos;
    return;
}

es_status_codes Lsc::_abortMeasurement()
{
    //lscpcie_end_block(info.dev);
    emit blockDone();
    emit measureDone();
    return AbortMeasurement( 1 );
}
