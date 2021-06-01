/* pcie.h
 *
 * Copyright 2020 Entwicklungsbuero Stresing (http://www.stresing.de/)
 * Copyright 2020 Bernhard Lang, University of Geneva
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _pcie_h_
#define _pcie_h_

#include <linux/pci.h>

int probe_lscpcie(struct pci_dev *dev, const struct pci_device_id *id);
void remove_lscpcie(struct pci_dev *dev);

#endif	/* _pcie_h_ */
