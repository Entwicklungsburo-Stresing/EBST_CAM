#ifndef _lscpcie_examples_common_h_
#define _lscpcie_examples_common_h_


#include "to-library.h"


int init_7030(unsigned int dev_no);
int readout_init(int argc, char **argv, struct camera_info_struct *info);
int fetch_data_mapped(dev_descr_t *dev, uint8_t *data, size_t max);
void print_data(const struct camera_info_struct *info);


#endif /* _lscpcie_examples_common_h_ */
