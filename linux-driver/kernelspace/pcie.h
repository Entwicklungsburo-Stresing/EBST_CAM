/*****************************************************************//**
 * @file		pcie.h
 * @author		Bernhard Lang
 * @date		05.11.2020
 * @copyright	Copyright 2020-2021 Bernhard Lang, University of Geneva, Entwicklungsbuero Stresing (http://www.stresing.de/) This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License version 2 as published by the Free Software Foundation.
 *********************************************************************/

#ifndef _pcie_h_
#define _pcie_h_

#include <linux/pci.h>

int probe_lscpcie(struct pci_dev *dev, const struct pci_device_id *id);
void remove_lscpcie(struct pci_dev *dev);

#endif	/* _pcie_h_ */
