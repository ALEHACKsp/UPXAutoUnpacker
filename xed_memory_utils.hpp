#pragma once

#include "xed_lib.hpp"

xed_bool_t xed_read_process_memory(__in HANDLE process_handle, xed_uint64_t runtime_address, xed_uint8_t* buf, xed_uint_t read_bytes);
xed_bool_t xed_write_process_memory(__in HANDLE process_handle, xed_uint64_t runtime_address, xed_uint8_t* buf, xed_uint_t write_bytes);

xed_bool_t xed_get_thread_context(__in HANDLE thread_handle, __inout LPCONTEXT context);
xed_bool_t xed_set_thread_context(__in HANDLE thread_handle, __in LPCONTEXT context);

xed_uint32_t xed_get_last_error();