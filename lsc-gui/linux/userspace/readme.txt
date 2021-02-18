readme file for lscpcie userspace code

lscpcie.c

general camera interface

* lscpcie_driver_init determines the number of boards (and debug instances) being
present in the system and initiates for each of them a separate structure
of type dev_struct_t containing all necessary pointers and flags. The other
functions address the device by its number (0..n where n+1 devices are present).
Returns the number of devices being present or a negative error code.

* lscpcie_open opens /dev/lscpcie<n>, mapps the memory areas and performs some
basic device initialisation (number of pixels over fiber link, CTRL registers,
number of pixels and cameras in pcie registers). Returns the file handle to
/dev/lscpcie<n> or a negative error code.