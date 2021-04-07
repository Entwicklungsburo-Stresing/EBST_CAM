#pragma once

typedef enum
{
	es_no_error = 0,
	es_device_not_found,
	es_driver_init_failed,
	es_debug_init_failed,
	es_setting_driver_name_failed,
	es_invalid_pixel_count = 5,
	es_invalid_driver_number,
	es_getting_device_info_failed,
	es_open_device_failed,
	es_invalid_driver_handle,
	es_register_read_failed = 10,
	es_register_write_failed,
	es_no_space0,
	es_allocating_memory_failed,
	es_not_enough_ram,
	es_parameter_out_of_range = 15,
	es_unknown_error,
	es_enabling_interrupts_failed,
	es_getting_dma_buffer_failed,
	es_unlocking_dma_failed,
	es_camera_not_found,
} es_status_codes;

char* ConvertErrorCodeToMsg(es_status_codes status);
