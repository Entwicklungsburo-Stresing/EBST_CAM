read-me file for lscpcie driver

compile with

$ make

load with

$ sudo /sbin/insmod ./lscpcie.ko

make the driver file accessible for normal user (will go at some point into the
driver)

$ sudo chmod 666 /dev/lscpcie0

####

When loading the module, the driver is registered for the vendor id of the
camera. All corresponding boards are associated with a separate instance of the
driver. For each instance a couple of special files are created

/dev/lscpcie<n> the file directly associated with the device. Read and write
access to the dma buffer, ioctl, mmap system calls

/proc/lscpcie<n> blocks reading when dma buffer is empty, can be used for
waiting on data without polling (not yet implemented)

/proc/lscpcie<n>_registers shows register contents in hex

/proc/lscpcie<n>_pci_io dumps register contents in binary format


The special file /proc/lscpcie exists as long as the module is loaded, whether
a board was found or not. Upon read it returns a single line containing two
ascii digits. The first is the number of board the second the number of
simulating debug devices.

The command

$ echo 512:2 > /proc/lscpcie

adds a simulating device for debugging purposes with to cameras and 512 pixels
each. Accessing registers is of course not possible. Corresponding system calls
(ioctl, mmap) will return -ENODEV.
