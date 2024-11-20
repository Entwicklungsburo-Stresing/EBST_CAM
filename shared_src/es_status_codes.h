#pragma once

#ifdef __cplusplus
extern "C" {
#endif
typedef enum
{
	es_no_error = 0,
	es_device_not_found = 1,
	es_driver_init_failed = 2,
	es_debug_init_failed = 3,
	es_setting_driver_name_failed = 4,
	es_invalid_pixel_count = 5,
	es_invalid_driver_number = 6,
	es_getting_device_info_failed = 7,
	es_open_device_failed = 8,
	es_invalid_driver_handle = 9,
	es_register_read_failed = 10,
	es_register_write_failed = 11,
	es_no_space0 = 12,
	es_allocating_memory_failed = 13,
	es_not_enough_ram = 14,
	es_parameter_out_of_range = 15,
	es_unknown_error = 16,
	es_enabling_interrupts_failed = 17,
	es_getting_dma_buffer_failed = 18,
	es_unlocking_dma_failed = 19,
	es_camera_not_found = 20,
	es_abortion = 21,
	es_creating_thread_failed = 22,
	es_setting_thread_priority_failed = 23,
	es_already_running = 24,
	es_disabling_interrupt_failed = 25,
	es_memory_not_initialized = 26,
	es_create_file_failed = 27,
	es_first_measurement_not_done = 28,
	es_measurement_running = 29,
	es_invalid_pointer = 30,
} es_status_codes;

char* ConvertErrorCodeToMsg(es_status_codes status);
#ifdef __cplusplus
}
#endif
