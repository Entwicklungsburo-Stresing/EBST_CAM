#include "lsc.h"
#include <memory.h>
#include <stdint.h>
#include "linux/userspace/types.h"
#include "linux/userspace/lscpcie.h"
#include "linux/kernelspace/registers.h"
#include <stdio.h>
#include <iomanip>

#define memory_barrier() asm volatile ("" : : : "memory")
#define CFG_BTIMER_IN_US      500000
#define CFG_STIMER_IN_US      400
dev_descr_t *device_descriptor;

Lsc::Lsc()
{
    driverInstructions = "Run 'sudo insmod lscpcie.ko' and 'sudo chmod 666 /dev/lscpcie0' before running this application";
}
Lsc::~Lsc()
{
}

/**
 * @brief Inits linux PCIe board driver.
 * @return
 *      - <1 failed
 *      - 1: success and one board found
 *      - 2: success and two boards found
 */
int Lsc::initDriver()
{
    return lscpcie_driver_init();
}

/**
 * @brief Inits PCIe board.
 * @return <0 on error, success otherwise
 */
int Lsc::initPcieBoard()
{
    int result;
    // open /dev/lscpcie<n>
    result = lscpcie_open(0, 0);
    if(result < 0) return result;
    // get memory mapped pointers etc
    device_descriptor = lscpcie_get_descriptor(0);
    // clear dma buffer to avoid reading stuff from previous debugging sessions
    memset((uint8_t*)device_descriptor->mapped_buffer, 0, device_descriptor->control->buffer_size);
    return result;
}
void Lsc::initMeasurement()
{
    lscpcie_send_fiber(0, MASTER_ADDRESS_CAMERA, CAMERA_ADDRESS_PIXEL, device_descriptor[0].control->number_of_pixels);
    trigger_mode_t trigger_mode = xck;
    int result = lscpcie_send_fiber(0, MASTER_ADDRESS_CAMERA, CAMERA_ADDRESS_TRIGGER_IN, trigger_mode);
    //if (result < 0)
        //return result;
    result = lscpcie_setup_dma(0);
    //if (result)
        //fprintf(stderr, "error %d when setting up dma\n", result);
    //set output of O on PCIe card
    device_descriptor->s0->TOR = TOR_OUT_XCK;
    //set trigger mode to block timer and scan timer + shutter not on
    device_descriptor->s0->CTRLB = (CTRLB_BTI_TIMER | CTRLB_STI_TIMER) & ~(CTRLB_SHON);
    //set block timer and start block timer
    device_descriptor->s0->BTIMER = 1<<BTIMER_START | CFG_BTIMER_IN_US;
    //set slope of block trigger
    device_descriptor->s0->BFLAGS |= 1<<BFLAG_BSLOPE;
    //// >>>> reset_dma_counters
    device_descriptor->s0->DMAS_PER_INTERRUPT |= (1<<DMA_COUNTER_RESET);
    memory_barrier();
    device_descriptor->s0->DMAS_PER_INTERRUPT &= ~(1<<DMA_COUNTER_RESET);

    //reset the internal block counter - is not BLOCKINDEX!
    device_descriptor->s0->DMA_BUF_SIZE_IN_SCANS |= (1<<BLOCK_COUNTER_RESET);
    memory_barrier();
    device_descriptor->s0->DMA_BUF_SIZE_IN_SCANS &= ~(1<<BLOCK_COUNTER_RESET);

    // reset block counter
    device_descriptor->s0->BLOCK_INDEX |= (1<<BLOCK_INDEX_RESET);
    memory_barrier();
    device_descriptor->s0->BLOCK_INDEX &= ~(1<<BLOCK_INDEX_RESET);

    //set Block end stops timer:
    //when SCANINDEX reaches NOS, the timer is stopped by hardware.
    device_descriptor->s0->PCIEFLAGS |= (1<<PCIE_EN_RS_TIMER_HW);
    ////<<<< reset all counters

    // >> SetIntFFTrig
    device_descriptor->s0->XCK.dword &= ~(1<<XCKMSB_EXT_TRIGGER);
    return;
}

void Lsc::startMeasurement()
{
    //set measure on
    device_descriptor->s0->PCIEFLAGS |= 1<<PCIEFLAG_MEASUREON;
    for (uint32_t blk_cnt = 0; blk_cnt < device_descriptor->control->number_of_blocks; blk_cnt++)
    {
        //block trigger
        if(!(device_descriptor->s0->CTRLA & 1<<CTRLA_TSTART))
            while(!(device_descriptor->s0->CTRLA & 1<<CTRLA_TSTART));
        //make pulse for blockindex counter
        device_descriptor->s0->PCIEFLAGS |= 1<<PCIEFLAG_BLOCKTRIG;
        memory_barrier();
        device_descriptor->s0->PCIEFLAGS &= ~(1<<PCIEFLAG_BLOCKTRIG);
        // reset scan counter
        device_descriptor->s0->SCAN_INDEX |= (1<<SCAN_INDEX_RESET);
        memory_barrier();
        device_descriptor->s0->SCAN_INDEX &= ~(1<<SCAN_INDEX_RESET);
        //set block on
        device_descriptor->s0->PCIEFLAGS |= 1<<PCIEFLAG_BLOCKON;
        //start Stimer
        device_descriptor->s0->XCK.dword = (device_descriptor->s0->XCK.dword & ~XCK_EC_MASK) | (CFG_STIMER_IN_US & XCK_EC_MASK) | (1<<XCK_RS);
        //software trigger
        device_descriptor->s0->BTRIGREG |= 1<<FREQ_REG_SW_TRIG;
        memory_barrier();
        device_descriptor->s0->BTRIGREG &= ~(1<<FREQ_REG_SW_TRIG);
        //wait for end of readout
        int result;
        do
            result = device_descriptor->s0->XCK.dword & (1<<XCK_RS);
        while (result);
        //reset block on
        device_descriptor->s0->PCIEFLAGS &= ~(1<<PCIEFLAG_BLOCKON);
    }
    //stop stimer
    device_descriptor->s0->XCK.dword &= ~(1<<XCK_RS);
    //stop btimer
    device_descriptor->s0->BTIMER &= ~(1<<BTIMER_START);
    //reset measure on
    device_descriptor->s0->PCIEFLAGS &= ~(1<<PCIEFLAG_MEASUREON);
    return;
}

void Lsc::returnFrame(uint32_t board, uint32_t sample, uint32_t block, uint16_t camera, uint16_t *pdest, uint32_t length)
{
    int n = device_descriptor->control->number_of_pixels;
    int nob = device_descriptor->control->number_of_blocks;
    int nos = device_descriptor->control->number_of_scans;
    int camcnt = device_descriptor->control->number_of_cameras;
    int offset = camera * n + sample * camcnt * n + block * nos * camcnt * n;
    memcpy( pdest, &((uint16_t*)device_descriptor->mapped_buffer)[offset], length * sizeof( uint16_t ) );
    return;
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
    for (int i = 0; i < number_of_registers; i++) {
        lscpcie_read_s0_32(0, i * 4, &data);
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
    for (int i = 0; i < number_of_registers; i++) {
        lscpcie_read_dma_32(0, i * 4, &data);
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
    lscpcie_read_config32(0, PCIeAddr_devCap, &data);
    data &= 0x7;
    stream  << "PAY_LOAD Supported:\t\t0x"
            << std::hex << data
            << '\n';
    lscpcie_read_config32(0, PCIeAddr_devStatCtrl, &data);
    uint32_t actpayload = (data >> 5) & 0x07;
    stream << "PAY_LOAD:\t\t\t0x"
           << std::hex << actpayload
           << '\n';
    lscpcie_read_config32(0, PCIeAddr_devStatCtrl, &data);
    data >>= 12;
    data &= 0x7;
    stream << "MAX_READ_REQUEST_SIZE:\t0x"
           << std::hex << data
           << '\n'
           << "Number of pixels:\t\t"
           << std::dec << device_descriptor[0].control->number_of_pixels
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
    lscpcie_read_dma_32(0, DmaAddr_WDMATLPS, &data);
    stream << "TLPS in DMAReg is:\t\t"
           << std::dec << data
           << "\n";
    data = (device_descriptor[0].control->number_of_pixels - 1) / (data * 2) + 1;
    stream << "number of TLPs should be:\t"
           << std::dec << data
           << "\n";
    lscpcie_read_dma_32(0, DmaAddr_WDMATLPC, &data);
    stream << "number of TLPs is:\t\t"
           << std::dec << data
           << "\n";
    return stream.str();
}
