#include "../crossPlattformBoard_ll.h"
#include "../../lsc-gui/linux/userspace/lscpcie.h"

es_status_codes setBitS0(uint32_t drvno, uint32_t bitnumber, uint16_t address)
{
    struct dev_descr *dev = lscpcie_get_descriptor(drvno);
    if(lscpcie_set_s0_bit(dev, address, bitnumber) < 0)
        return es_register_write_failed;
    else
        return es_no_error;
}

es_status_codes resetBitS0(uint32_t drvno, uint32_t bitnumber, uint16_t address)
{
    struct dev_descr *dev = lscpcie_get_descriptor(drvno);
    if(lscpcie_reset_s0_bit(dev, address, bitnumber) < 0)
        return es_register_write_failed;
    else
        return es_no_error;
}