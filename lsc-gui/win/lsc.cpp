#include "../lsc.h"
#include "shared_src/board.h"
#include <sstream>

Lsc::Lsc()
{
    driverInstructions = "";
}
Lsc::~Lsc()
{
    CCDDrvExit(1);
}

es_status_codes Lsc::initDriver()
{
    return CCDDrvInit();
}

es_status_codes Lsc::initPcieBoard()
{
    return InitBoard(1);
}

es_status_codes Lsc::initMeasurement(struct global_settings* settings_struct)
{
    return InitMeasurement(settings_struct);
}

es_status_codes Lsc::startMeasurement(uint8_t boardsel)
{
    return StartMeasurement(boardsel);
}

es_status_codes Lsc::returnFrame(uint32_t board, uint32_t sample, uint32_t block, uint16_t camera, uint16_t *pdest, uint32_t length)
{
    return ReturnFrame(board, sample, block, camera, pdest, length);
}

std::string Lsc::dumpS0Registers()
{
    uint32_t drv = 1;
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
        "R14 ADSC\t\t",
        "R15 LDSC\t\t",
        "R16 BTimer\t\t",
        "R17 BDAT\t\t",
        "R18 BEC\t\t",
        "R19 BFLAGS\t\t",
        "R20 TR1\t\t",
        "R21 TR2\t\t",
        "R22 TR3\t\t",
        "R23 TR4\t\t",
        "R24 TR5\t\t"
    }; //Look-Up-Table for the S0 Registers
    uint32_t data = 0;
    std::stringstream stream;
    for (int i = 0; i < number_of_registers; i++) {
        //linux: lscpcie_read_s0_32(0, i * 4, &data);
        ReadLongS0(drv, &data, i * 4);
        stream << register_names[i]
            << "0x"
            << std::hex << data
            << '\n';
    }
    return stream.str();
}

std::string Lsc::dumpDmaRegisters()
{
    uint32_t drv = 1;
    enum N { number_of_registers = 18 };
    std::string register_names[number_of_registers] = {
        "DCSR\t\t",
        "DDMACR\t\t",
        "WDMATLPA\t\t",
        "WDMATLPS\t\t",
        "WDMATLPC\t\t",
        "WDMATLPP\t\t",
        "RDMATLPP\t\t",
        "RDMATLPA\t\t",
        "RDMATLPS\t\t",
        "RDMATLPC\t\t",
        "WDMAPERF\t\t",
        "RDMAPERF\t\t",
        "RDMASTAT\t\t",
        "NRDCOMP\t\t",
        "RCOMPDSIZW\t\t",
        "DLWSTAT\t\t",
        "DLTRSSTAT\t\t",
        "DMISCCONT\t\t"
    }; //Look-Up-Table for the DMA Registers
    uint32_t data = 0;
    std::stringstream stream;
    for (int i = 0; i < number_of_registers; i++) {
        //linux: lscpcie_read_dma_32(0, i * 4, &data);
        ReadLongDMA(drv, &data, i * 4);
        stream << register_names[i]
            << "0x"
            << std::hex << data
            << '\n';
    }
    return stream.str();
}

std::string Lsc::dumpTlp()
{
    uint32_t drv = 1;
    uint32_t data = 0;
    std::stringstream stream;
    stream << "PAY_LOAD values:\t0 = 128 bytes\n\t\t1 = 256 bytes\n\t\t2 = 512 bytes\n";
    ReadLongIOPort(drv, &data, PCIeAddr_devCap);
    data &= 0x7;
    stream << "PAY_LOAD Supported:\t0x"
        << std::hex << data
        << '\n';
    ReadLongIOPort(drv, &data, PCIeAddr_devStatCtrl);
    uint32_t actpayload = (data >> 5) & 0x07;
    stream << "PAY_LOAD:\t\t0x"
        << std::hex << actpayload
        << '\n';
    ReadLongIOPort(drv, &data, PCIeAddr_devStatCtrl);
    data >>= 12;
    data &= 0x7;
    stream << "MAX_READ_REQUEST_SIZE:\t0x"
        << std::hex << data
        << '\n'
        << "Number of pixels:\t"
        << std::dec << aPIXEL[drv]
        << "\n";
    switch (actpayload) {
    case 0: data = 0x20;  break;
    case 1: data = 0x40;  break;
    case 2: data = 0x80;  break;
    case 3: data = 0x100; break;
    }
    stream << "TLP_SIZE is:\t\t"
        << std::dec << data
        << " DWORDs = "
        << std::dec << data * 4
        << " BYTEs\n";
    ReadLongDMA(drv, &data, DmaAddr_WDMATLPS);
    stream << "TLPS in DMAReg is:\t"
        << std::dec << data
        << "\n";
    data = (aPIXEL[drv] - 1) / (data * 2) + 1;
    stream << "number of TLPs should be:\t"
        << std::dec << data
        << "\n";
    ReadLongDMA(drv, &data, DmaAddr_WDMATLPC);
    stream << "number of TLPs is:\t"
        << std::dec << data
        << "\n";
    return stream.str();
}

void Lsc::setTorOut(uint8_t torOut)
{
    return;
}
