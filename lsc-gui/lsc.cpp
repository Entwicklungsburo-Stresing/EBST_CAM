﻿#include "lsc.h"
#include <sstream>
#include "../shared_src/Board.h"

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
    return InitMeasurement(settings_struct);
}

es_status_codes Lsc::startMeasurement()
{
    return StartMeasurement();
}

es_status_codes Lsc::returnFrame(uint32_t board, uint32_t sample, uint32_t block, uint16_t camera, uint16_t *pdest, uint32_t length)
{
    return ReturnFrame(board, sample, block, camera, pdest, length);
}

std::string Lsc::dumpS0Registers(uint32_t drvno)
{
    enum N { number_of_registers = 43 };
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
        "R0 PCIEFLAGS\t\t",
        "R1 NOS\t\t",
        "R2 SCANINDEX\t\t",
        "R3 DMABUFSIZE\t\t",
        "R4 DMASPERINTR\t",
        "R5 BLOCKS\t\t",
        "R6 BLOCKINDEX\t\t",
        "R7 CAMCNT\t\t",
        "R8 GPX Ctrl\t\t",
        "R9 GPX Data\t\t",
        "R10 ROI 0\t\t",
        "R11 ROI 1\t\t",
        "R12 ROI 2\t\t",
        "R13 XCKDLY\t\t",
        "R14 nc\t\t",
        "R15 nc\t\t",
        "R16 BTimer\t\t",
        "R17 BDAT\t\t",
        "R18 BEC\t\t",
        "R19 BFLAGS\t\t",
        "R20 ADSC1\t\t",
        "R21 LDSC1\t\t",
        "R22 ADSC2\t\t",
        "R23 LDSC2\t\t",
		"R24 ADSC3\t\t",
		"R25 LDSC3\t\t",
        "R26 DSCCTRL\t\t"
    }; //Look-Up-Table for the S0 Registers
    uint32_t data = 0;
    std::stringstream stream;
    for (int i = 0; i < number_of_registers; i++)
    {
        es_status_codes status = readRegisterS0_32(drvno, &data, i*4);
        if(status != es_no_error)
        {
            stream << "\nerror while reading register\n";
            return stream.str();
        }
        stream  << register_names[i]
                << "0x"
                << std::hex << data
                << '\n';
    }
    return stream.str();
}

std::string Lsc::dumpDmaRegisters(uint32_t drvno)
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
        es_status_codes status = readRegisterDma_32(drvno, &data, i*4);
        if(status != es_no_error)
        {
            stream << "\nerror while reading register\n";
            return stream.str();
        }
        stream  << register_names[i]
                << "0x"
                << std::hex << data
                << '\n';
    }
    return stream.str();
}

std::string Lsc::dumpTlp(uint32_t drvno)
{
    uint32_t data = 0;
    std::stringstream stream;
    stream  << "PAY_LOAD values:\t\t0 = 128 bytes\n\t\t\t1 = 256 bytes\n\t\t\t2 = 512 bytes\n";
    es_status_codes status = readConfig_32(drvno, &data, PCIeAddr_devCap);
    if(status != es_no_error)
    {
        stream << "\nerror while reading register\n";
        return stream.str();
    }
    data &= 0x7;
    stream  << "PAY_LOAD Supported:\t\t0x"
            << std::hex << data
            << '\n';
    status = readConfig_32(drvno, &data, PCIeAddr_devCap);
    if(status != es_no_error)
    {
        stream << "\nerror while reading register\n";
        return stream.str();
    }
    uint32_t actpayload = (data >> 5) & 0x07;
    stream << "PAY_LOAD:\t\t\t0x"
           << std::hex << actpayload
           << '\n';
    data >>= 12;
    data &= 0x7;
    stream << "MAX_READ_REQUEST_SIZE:\t0x"
           << std::hex << data
           << '\n';
    uint32_t pixel = 0;
    status = readRegisterS0_32(drvno, &pixel, S0Addr_PIXREGlow);
    pixel &= 0xFFFF;
    stream << "Number of pixels:\t\t"
           << std::dec << pixel
           << "\n";
    switch (actpayload)
    {
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
    status = readRegisterDma_32(drvno, &data, DmaAddr_WDMATLPS);
    if(status != es_no_error)
    {
        stream << "\nerror while reading register\n";
        return stream.str();
    }
    stream << "TLPS in DMAReg is:\t\t"
           << std::dec << data
           << "\n";
    if(data)
        data = (pixel - 1) / (data * 2) + 1;
    stream << "number of TLPs should be:\t"
           << std::dec << data
           << "\n";
    status = readRegisterDma_32(drvno, &data, DmaAddr_WDMATLPC);
    if(status != es_no_error)
    {
        stream << "\nerror while reading register\n";
        return stream.str();
    }
    stream << "number of TLPs is:\t\t"
           << std::dec << data
           << "\n";
    return stream.str();
}

es_status_codes Lsc::setTorOut(uint32_t drvno, uint8_t torOut)
{
    return SetTORReg(drvno, torOut);
}

es_status_codes Lsc::abortMeasurement(uint32_t drvno)
{
    emit blockDone();
    emit measureDone();
    return AbortMeasurement( drvno );
}