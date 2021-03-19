#include "local-config.h"
#include "common.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <memory.h>

/* Performs a camera read with polling.
   After intitialising and starting the cameras, the read and write pointers
   in the control memory mapped from the driver are checked repeatedly and
   buffer contents are copied upon pointer change.
   The block trigger flag in CRTLA is polled ẗo detect block ends and start
   new block readings until the number of bytes copied from the mapped DMA
   buffer reaches the goal defined by the two command line arguments
   <number of scans> <number of blocks>.
*/

int lscpcie_acquire_block_fs(dev_descr_t *dev, uint8_t *data, size_t n_scans,
			int camera_file_handle) {
	int result, bytes_read = 0;
	size_t block_size =
		dev->control->number_of_pixels * dev->control->number_of_cameras
		* sizeof(pixel_t) * n_scans;

	result = lscpcie_start_block_soft(dev);
	if (result < 0)
		return result;

	do {
		result = read(camera_file_handle, data + bytes_read,
			block_size - bytes_read);

		bytes_read += result;
		fprintf(stderr, "got %d bytes of data, having now %d\n",
			result, bytes_read);
	} while (bytes_read < block_size);

        result = lscpcie_end_block(dev);
	if (result < 0)
		return result;

	return bytes_read;
}

int main(int argc, char **argv)
{
	int result, bytes_read;
	struct camera_info_struct info;

	result = readout_init(argc, argv, &info);
	bytes_read = 0;

	do {
		// wait for trigger signal
		if (info.dev->s0->CTRLA & (1 << CTRLA_TSTART))
			continue;

		result = lscpcie_acquire_block_fs(info.dev,
						(uint8_t *) info.data,
						2, info.dev->handle);
		if (result) {
			fprintf(stderr, "error %d when acquiring block\n",
				result);
			goto out;
		}
	} while (bytes_read < info.mem_size);

	fprintf(stderr, "finished measurement\n");

	result = lscpcie_end_acquire(info.dev);
	if (result)
		fprintf(stderr, "error %d when finishing acquisition\n", result);
	else
		print_data(&info);

      out:
	if (info.data)
		free(info.data);
	lscpcie_close(0);

	return result;
}
