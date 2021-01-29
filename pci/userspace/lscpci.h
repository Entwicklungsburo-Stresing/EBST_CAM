/* lscpci.c
 *
 * Copyright (C) 2010-2016 Bernhard Lang, University of Geneva
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _lscpci_h_
#define _lscpci_h_

#include <stdlib.h>
#include <stdint.h>
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

int lscpci_open(const char *name, int flags);
int lscpci_init(int handle);
int lscpci_set_params(int handle, const struct lscpci_param *);
int lscpci_get_params(int handle, struct lscpci_param *);
int lscpci_number_of_pixels(int handle);

ssize_t lscpci_readout(int handle, uint16_t *buffer, size_t bytes_to_read);
size_t lscpci_bytes_available(int handle);
size_t lscpci_bytes_free(int handle);

int lscpci_start(int handle);
int lscpci_idle_run(int handle);
int lscpci_stop(int handle);

int get_state(int handle);
int lscpci_fifo_overflow(int handle);

int lscpci_clear_fifo(int handle);
int lscpci_software_trigger(int handle);
int lscpci_reset_fifo(int handle);
int lscpci_hardware_present(int handle);

int lscpci_set_register(int handle, uint8_t address, uint8_t value);
int lscpci_get_irq_count(int handle);

int lscpci_fetch_registers(int handle, uint8_t *registers);
int lscpci_set_debug(int handle, uint16_t bits, uint16_t mask);
int lscpci_get_buffer_pointers(int handle, lscpci_buffer_ptr_t *);

#ifdef __cplusplus
}
#endif

#endif /* _lscpci_h_ */
