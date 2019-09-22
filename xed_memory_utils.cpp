#include "pch.h"

#include "xed_memory_utils.hpp"

xed_bool_t xed_read_process_memory(__in HANDLE process_handle, xed_uint64_t runtime_address, xed_uint8_t* buf, xed_uint_t read_bytes)
{
	return ReadProcessMemory(process_handle, (LPCVOID)runtime_address, (LPVOID)buf, (SIZE_T)read_bytes, NULL);
}

xed_bool_t xed_write_process_memory(__in HANDLE process_handle, xed_uint64_t runtime_address, xed_uint8_t* buf, xed_uint_t write_bytes)
{
	DWORD flProtect;
	LPVOID lpTarget;

	lpTarget = (LPVOID)runtime_address;

	if (!VirtualProtectEx(process_handle, lpTarget, write_bytes, PAGE_EXECUTE_READWRITE, &flProtect))
	{
		return FALSE;
	}

	if (!WriteProcessMemory(process_handle, lpTarget, (LPCVOID)buf, write_bytes, NULL))
	{
		VirtualProtectEx(process_handle, lpTarget, write_bytes, flProtect, &flProtect);
		return FALSE;
	}

	if (!VirtualProtectEx(process_handle, lpTarget, write_bytes, flProtect, &flProtect))
	{
		return FALSE;
	}

	return TRUE;
}

xed_bool_t xed_get_thread_context(__in HANDLE thread_handle, __inout LPCONTEXT context)
{
	return GetThreadContext(thread_handle, context);
}

xed_bool_t xed_set_thread_context(__in HANDLE thread_handle, __in LPCONTEXT context)
{
	return SetThreadContext(thread_handle, context);
}

xed_uint32_t xed_get_last_error()
{
	return (xed_uint32_t)GetLastError();
}