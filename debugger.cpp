#include "pch.h"

#include "debugger.hpp"

#include "xed_memory_utils.hpp"
#include "xed_instruction_utils.hpp"

#include <thread>

CDebugger::CDebugger(std::string sCommandLine)
{
	this->m_wsCommandLine = std::wstring(sCommandLine.begin(), sCommandLine.end());
	this->m_bWow64 = FALSE;
	this->m_bDebug = FALSE;
}

CDebugger::CDebugger(std::wstring wsCommandLine)
{
	this->m_wsCommandLine = wsCommandLine;
	this->m_bWow64 = FALSE;
	this->m_bDebug = FALSE;
}

CDebugger::~CDebugger()
{
}

BOOL CDebugger::AttachProcess()
{
	memset(&this->m_piTarget, 0, sizeof(PROCESS_INFORMATION));
	memset(&this->m_siTarget, 0, sizeof(STARTUPINFOW));

	this->m_siTarget.cb = sizeof(STARTUPINFOW);
	BOOL status = CreateProcessW(NULL, (LPWSTR)this->m_wsCommandLine.c_str(), NULL, NULL, FALSE,
		DEBUG_PROCESS | CREATE_SUSPENDED, NULL, NULL, &this->m_siTarget, &this->m_piTarget);

	if (!status)
		return FALSE; 
	
	BOOL bWow64Process = FALSE;
	if (IsWow64Process(this->m_piTarget.hProcess, &bWow64Process) == FALSE)
		return FALSE;

	printf("Target PID:%d\n", this->m_piTarget.dwProcessId);

	this->m_bWow64 = bWow64Process ? TRUE : FALSE;
	return TRUE;
}

BOOL CDebugger::AttachProcess(DWORD dwProcessId)
{
	BOOL status = DebugActiveProcess(dwProcessId);

	if (!status)
		return FALSE;

	BOOL bWow64Process = FALSE;
	if (IsWow64Process(this->m_piTarget.hProcess, &bWow64Process) == FALSE)
		return FALSE;

	printf("Target PID:%d\n", this->m_piTarget.dwProcessId);

	this->m_bWow64 = bWow64Process ? TRUE : FALSE;
	return TRUE;
}

void CDebugger::Run()
{	
	ResumeThread(this->m_piTarget.hThread);
	RunDebug();
}

BOOL CDebugger::SetBreakPoint(LPVOID lpTarget, BOOL bWow64)
{
	if (bWow64)
		return SetBreakPointWX86(lpTarget);
	else
		return SetBreakPointWX64(lpTarget);
}

BOOL CDebugger::SetBreakPointWX86(LPVOID lpTarget)
{
	xed_uint64_t runtime_address = (xed_uint64_t)lpTarget;

	xed_uint8_t buf[XED_MAX_INSTRUCTION_BYTES];

	if (!xed_read_process_memory(this->m_piTarget.hProcess, runtime_address, buf, sizeof(buf)))
	{
		printf("xed_read_process_memory GLE=%08X\n", GetLastError());
		return FALSE;
	}

	xed_instruction instruction;

	xed_error_enum_t xed_error = instruction.decode(buf, sizeof(buf), XED_MACHINE_MODE_LEGACY_32, XED_ADDRESS_WIDTH_32b);
	if (xed_error != XED_ERROR_NONE)
	{
		printf("xed_decode GLE=%08X\n", xed_error);
		return FALSE;
	}

	xed_uint8_t write_bytes[1] = { 0xCC };

	if (!xed_write_process_memory(this->m_piTarget.hProcess, runtime_address, write_bytes, sizeof(write_bytes)))
	{
		printf("xed_write_process_memory GLE=%08X\n", GetLastError());
		return FALSE;
	}

	instruction.set_runtime_address(runtime_address);
	this->m_breakpoints.push_back(instruction);

	return TRUE;
}

BOOL CDebugger::SetBreakPointWX64(LPVOID lpTarget)
{
	xed_uint64_t runtime_address = (xed_uint64_t)lpTarget;

	xed_uint8_t buf[XED_MAX_INSTRUCTION_BYTES];

	if (!xed_read_process_memory(this->m_piTarget.hProcess, runtime_address, buf, sizeof(buf)))
	{
		printf("xed_read_process_memory GLE=%08X\n", GetLastError());
		return FALSE;
	}

	xed_instruction instruction;

	xed_error_enum_t xed_error = instruction.decode(buf, sizeof(buf), XED_MACHINE_MODE_LONG_64, XED_ADDRESS_WIDTH_64b);
	if (xed_error != XED_ERROR_NONE)
	{
		printf("xed_decode GLE=%08X\n", xed_error);
		return FALSE;
	}

	xed_uint8_t write_bytes[1] = { 0xCC };

	if (!xed_write_process_memory(this->m_piTarget.hProcess, runtime_address, write_bytes, sizeof(write_bytes)))
	{
		printf("xed_write_process_memory GLE=%08X\n", GetLastError());
		return FALSE;
	}

	instruction.set_runtime_address(runtime_address);
	this->m_breakpoints.push_back(instruction);

	return TRUE;
}

BOOL CDebugger::DeleteBreakPoint(LPVOID lpTarget)
{
	auto it = std::find_if(this->m_breakpoints.begin(), this->m_breakpoints.end(), 
		xed_instruction::find_address((xed_uint64_t)lpTarget));

	if (it != this->m_breakpoints.end())
	{
		xed_uint8_t write_bytes[1] = { it->get_byte() };

		if (xed_write_process_memory(this->m_piTarget.hProcess, it->get_runtime_address(), write_bytes, sizeof(write_bytes)))
		{
			printf("Break: %llX - %s\n", lpTarget, it->dump_intel_format().c_str());

			this->m_breakpoints.erase(it);
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CDebugger::HitBreakPointWX86(HANDLE hThread, LPVOID lpTarget)
{
	if (!DeleteBreakPoint(lpTarget))	
	{
		return FALSE;
	}

	CONTEXT ctx;
	memset(&ctx, 0, sizeof(CONTEXT));
	ctx.ContextFlags = CONTEXT_FULL;

	if (!GetThreadContext(hThread, &ctx))
	{
		printf("GetThreadContext GLE=%08X\n", GetLastError());
		return FALSE;
	}

	ctx.EFlags |= 0x00000100;
	ctx.Rip--;

	xed_uint8_t buf[XED_MAX_INSTRUCTION_BYTES];
	if (!xed_read_process_memory(this->m_piTarget.hProcess, ctx.Rip, buf, sizeof(buf)))
	{
		printf("xed_read_process_memory GLE=%08X\n", GetLastError());
		return FALSE;
	}

	xed_instruction instruction;
	xed_error_enum_t xed_error = instruction.decode(buf, sizeof(buf), XED_MACHINE_MODE_LEGACY_32, XED_ADDRESS_WIDTH_32b);
	if (xed_error != XED_ERROR_NONE)
	{
		printf("xed_error GLE=%08X\n", xed_error);
		return FALSE;
	}

	// skip sti instruction
	if (instruction.get_iclass() == XED_ICLASS_STI)
		ctx.Rip += instruction.get_length();

	if (!SetThreadContext(hThread, &ctx))
	{
		printf("SetThreadContext GLE=%08X\n", GetLastError());
		return FALSE;
	}

	return TRUE;
}

BOOL CDebugger::HitBreakPointWX64(HANDLE hThread, LPVOID lpTarget)
{
	if (!DeleteBreakPoint(lpTarget))
	{
		return FALSE;
	}

	CONTEXT ctx;
	memset(&ctx, 0, sizeof(CONTEXT));
	ctx.ContextFlags = CONTEXT_FULL;

	if (!GetThreadContext(hThread, &ctx))
	{
		printf("GetThreadContext GLE=%08X\n", GetLastError());
		return FALSE;
	}

	ctx.EFlags |= 0x00000100;
	ctx.Rip--;

	xed_uint8_t buf[XED_MAX_INSTRUCTION_BYTES];
	if (!xed_read_process_memory(this->m_piTarget.hProcess, ctx.Rip, buf, sizeof(buf)))
	{
		printf("xed_read_process_memory GLE=%08X\n", GetLastError());
		return FALSE;
	}

	xed_instruction instruction;
	xed_error_enum_t xed_error = instruction.decode(buf, sizeof(buf), XED_MACHINE_MODE_LONG_64, XED_ADDRESS_WIDTH_64b);
	if (xed_error != XED_ERROR_NONE)
	{
		printf("xed_error GLE=%08X\n", xed_error);
		return FALSE;
	}

	// skip sti instruction
	if (instruction.get_iclass() == XED_ICLASS_STI)
		ctx.Rip += instruction.get_length();

	if (!SetThreadContext(hThread, &ctx))
	{
		printf("SetThreadContext GLE=%08X\n", GetLastError());
		return FALSE;
	}

	return TRUE;
}

BOOL CDebugger::SingleStepWX86(HANDLE hThread)
{
	CONTEXT ctx;

	memset(&ctx, 0, sizeof(CONTEXT));
	ctx.ContextFlags = CONTEXT_FULL;

	if (!GetThreadContext(hThread, &ctx))
	{
		printf("GetThreadContext GLE=%08X\n", GetLastError());
		return FALSE;
	}

	std::vector<xed_instruction> instructions;

	xed_uint8_t buf[0x1000 + XED_MAX_INSTRUCTION_BYTES];
	xed_uint_t bufsize = sizeof(buf) - XED_MAX_INSTRUCTION_BYTES;

	if (!xed_read_process_memory(this->m_piTarget.hProcess, (xed_uint64_t)ctx.Rip, buf, bufsize))
	{
		printf("xed_read_process_memory GLE=%08X\n", GetLastError());
		return FALSE;
	}

	xed_uint_t rest_bytes = bufsize;
	xed_uint_t offset = 0;

	while (rest_bytes > 0)
	{
		xed_instruction instruction;
		xed_error_enum_t xed_error = instruction.decode((xed_uint64_t)buf + offset, XED_MACHINE_MODE_LEGACY_32, XED_ADDRESS_WIDTH_32b);
		if (xed_error != XED_ERROR_NONE)
		{
			printf("xed_decode GLE=%08X\n", xed_error);
			break;
		}

		// add [eax], al
		auto array_byte = instruction.get_bytes();
		if (array_byte[0] == 0x00 &&
			array_byte[1] == 0x00)
		{
			break;
		}

		// real instruction address
		instruction.set_runtime_address(ctx.Rip + offset);

		// add instruction
		instructions.push_back(instruction);

		offset += instruction.get_length();
		rest_bytes -= instruction.get_length();
	}

	for (auto it = instructions.begin(); it != instructions.end(); it++)
	{
		// push x
		// cmp eap, reg
		// jne x
		// sub esp, x
		// jmp oep
		if (it->get_iclass() == XED_ICLASS_PUSH)
		{
			auto operands = it->get_operands();
			if (operands.size() == 4)
			{
				// push x
				auto next = std::next(it);
				if (next == instructions.end())
					continue;

				auto next_operands = next->get_operands();
				if (next->get_iclass() == XED_ICLASS_CMP
					&& next_operands.size() == 3
					&& next_operands[0].is_register() && next->get_register(next_operands[0].get_name()) == XED_REG_ESP
					&& next_operands[1].is_register())
				{
					// cmp esp, reg
					auto next_next = std::next(next);
					if (next_next == instructions.end())
						continue;

					auto next_next_operands = next_next->get_operands();
					if (next_next->get_iclass() == XED_ICLASS_JNZ
						&& next_next_operands.size() == 3)
					{
						// jne x
						auto next_next_next = std::next(next_next);
						if (next_next_next == instructions.end())
							continue;

						auto next_next_next_operands = next_next_next->get_operands();
						if (next_next_next->get_iclass() == XED_ICLASS_SUB
							&& next_next_next_operands[0].is_register() && next_next_next->get_register(next_next_next_operands[0].get_name()) == XED_REG_ESP)
						{
							// sub esp, x
							auto next_next_next_next = std::next(next_next_next);
							if (next_next_next_next == instructions.end())
								continue;

							if (next_next_next_next->get_iclass() == XED_ICLASS_JMP)
							{
								xed_instruction *inst = &(*next_next_next_next);
								xed_uint64_t address = inst->get_runtime_address() + inst->get_length() + inst->get_branch_displacement();

								printf("OEP: %llX\n", address);

								ctx.Dr6 = 0x0;
								ctx.Dr7 = 0x0;
								SetThreadContext(hThread, &ctx);

								return TRUE;
							}
						}
					}
				}
			}
		}
	}

	ctx.EFlags |= 0x00000100;
	if (!SetThreadContext(hThread, &ctx))
	{
		printf("SetThreadContext GLE=%08X\n", GetLastError());
		return FALSE;
	}

	return TRUE;
}

BOOL CDebugger::SingleStepWX64(HANDLE hThread)
{
	CONTEXT ctx;

	memset(&ctx, 0, sizeof(CONTEXT));
	ctx.ContextFlags = CONTEXT_FULL;

	if (!GetThreadContext(hThread, &ctx))
	{
		printf("GetThreadContext GLE=%08X\n", GetLastError());
		return FALSE;
	}

	std::vector<xed_instruction> instructions;

	xed_uint8_t buf[0x1000 + XED_MAX_INSTRUCTION_BYTES];
	xed_uint_t bufsize = sizeof(buf) - XED_MAX_INSTRUCTION_BYTES;

	if (!xed_read_process_memory(this->m_piTarget.hProcess, (xed_uint64_t)ctx.Rip, buf, bufsize))
	{
		printf("xed_read_process_memory GLE=%08X\n", GetLastError());
		return FALSE;
	}

	xed_uint_t rest_bytes = bufsize;
	xed_uint_t offset = 0;

	while (rest_bytes > 0)
	{
		xed_instruction instruction;
		xed_error_enum_t xed_error = instruction.decode((xed_uint64_t)buf + offset, XED_MACHINE_MODE_LONG_64, XED_ADDRESS_WIDTH_64b);
		if (xed_error != XED_ERROR_NONE)
		{
			printf("xed_decode GLE=%08X\n", xed_error);
			break;
		}

		// add [eax], al
		auto array_byte = instruction.get_bytes();
		if (array_byte[0] == 0x00 &&
			array_byte[1] == 0x00)
		{
			break;
		}

		// real instruction address
		instruction.set_runtime_address(ctx.Rip + offset);

		// add instruction
		instructions.push_back(instruction);

		offset += instruction.get_length();
		rest_bytes -= instruction.get_length();
	}

	for (auto it = instructions.begin(); it != instructions.end(); it++)
	{
		// push x
		// cmp rsp, reg
		// jne x
		// sub rsp, x
		// jmp oep
		if (it->get_iclass() == XED_ICLASS_PUSH)
		{
			auto operands = it->get_operands();
			if (operands.size() == 4)
			{
				// push x
				auto next = std::next(it);
				if (next == instructions.end())
					continue;

				auto next_operands = next->get_operands();
				if (next->get_iclass() == XED_ICLASS_CMP
					&& next_operands.size() == 3
					&& next_operands[0].is_register() && next->get_register(next_operands[0].get_name()) == XED_REG_RSP
					&& next_operands[1].is_register())
				{
					// cmp esp, reg
					auto next_next = std::next(next);
					if (next_next == instructions.end())
						continue;

					auto next_next_operands = next_next->get_operands();
					if (next_next->get_iclass() == XED_ICLASS_JNZ
						&& next_next_operands.size() == 3)
					{
						// jne x
						auto next_next_next = std::next(next_next);
						if (next_next_next == instructions.end())
							continue;

						auto next_next_next_operands = next_next_next->get_operands();
						if (next_next_next->get_iclass() == XED_ICLASS_SUB
							&& next_next_next_operands[0].is_register() && next_next_next->get_register(next_next_next_operands[0].get_name()) == XED_REG_RSP)
						{
							// sub esp, x
							auto next_next_next_next = std::next(next_next_next);
							if (next_next_next_next == instructions.end())
								continue;

							if (next_next_next_next->get_iclass() == XED_ICLASS_JMP)
							{
								xed_instruction *inst = &(*next_next_next_next);
								xed_uint64_t address = inst->get_runtime_address() + inst->get_length() + inst->get_branch_displacement();

								printf("OEP: %llX\n", address);

								ctx.Dr6 = 0x0;
								ctx.Dr7 = 0x0;
								SetThreadContext(hThread, &ctx);

								return TRUE;
							}
						}
					}
				}
			}
		}
	}

	ctx.EFlags |= 0x00000100;
	if (!SetThreadContext(hThread, &ctx))
	{
		printf("SetThreadContext GLE=%08X\n", GetLastError());
		return FALSE;
	}

	return TRUE;
}

BOOL CDebugger::RunDebug()
{
	this->m_bDebug = TRUE;

	while (TRUE)
	{
		if (!WaitForDebugEvent(&this->m_deTarget, INFINITE))		
			return FALSE;		

		switch (this->m_deTarget.dwDebugEventCode)
		{
			case CREATE_PROCESS_DEBUG_EVENT: 
				CreateProcessDebugEvent();
				break;
			case CREATE_THREAD_DEBUG_EVENT:
				CreateThreadDebugEvent();
				break;
			case EXCEPTION_DEBUG_EVENT:      
				ExceptionDebugEvent();
				break;
			case EXIT_PROCESS_DEBUG_EVENT:   
				ExitProcessDebugEvent();
				return FALSE;
			case EXIT_THREAD_DEBUG_EVENT:    
				ExitThreadDebugEvent();
				break;
			case LOAD_DLL_DEBUG_EVENT:
				LoadDllDebugEvent();
				break;
			case OUTPUT_DEBUG_STRING_EVENT:  
				OutputDebugStringEvent();
				break;
			case RIP_EVENT:
				RIP_Event();
				break;
			case UNLOAD_DLL_DEBUG_EVENT:    
				UnloadDllDebugEvent();
				break;
		}

		if (!ContinueDebugEvent(this->m_deTarget.dwProcessId, this->m_deTarget.dwThreadId, DBG_CONTINUE))		
			return FALSE;		
	}

	this->m_bDebug = FALSE;

	return TRUE;
}

void CDebugger::CreateProcessDebugEvent()
{
	SetBreakPoint(this->m_deTarget.u.CreateProcessInfo.lpStartAddress, this->m_bWow64);
}

void CDebugger::CreateThreadDebugEvent()
{
}

void CDebugger::ExitThreadDebugEvent()
{
}

void CDebugger::ExceptionDebugEvent()
{
	DWORD dwThreadId;
	HANDLE hThread;

	dwThreadId = this->m_deTarget.dwThreadId;	

	if (this->m_piTarget.dwThreadId != dwThreadId)
	{
		hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, dwThreadId);
		if (hThread == NULL)
		{
			printf("OpenThread GLE=%08X \n", GetLastError());
			return;
		}
	}
	else
		hThread = this->m_piTarget.hThread;

	{
		DWORD dwExceptionCode = this->m_deTarget.u.Exception.ExceptionRecord.ExceptionCode;
		PVOID pvExceptionAddr = this->m_deTarget.u.Exception.ExceptionRecord.ExceptionAddress;

		switch (dwExceptionCode)
		{
			case STATUS_BREAKPOINT:
			{
				HitBreakPointWX64(hThread, pvExceptionAddr);
				break;
			}
			case STATUS_WX86_BREAKPOINT:
			{
				HitBreakPointWX86(hThread, pvExceptionAddr);
				break;
			}
			case STATUS_SINGLE_STEP:
			{
				SingleStepWX64(hThread);
				break;
			}
			case STATUS_WX86_SINGLE_STEP:
			{
				SingleStepWX86(hThread);
				break;
			}
			// Executing an instruction not allowed in current machine mode.
			case STATUS_PRIVILEGED_INSTRUCTION:
			{
				printf("%llX = STATUS_PRIVILEGED_INSTRUCTION \n", pvExceptionAddr);
				MessageBox(0, 0, 0, 0);
				break;
			}
			default:
			{
				printf("%llX = %08X\n", pvExceptionAddr, dwExceptionCode);
				break;
			}
		}
	}
}

void CDebugger::ExitProcessDebugEvent()
{
}

void CDebugger::LoadDllDebugEvent()
{
}

void CDebugger::OutputDebugStringEvent()
{
}

void CDebugger::RIP_Event()
{
}

void CDebugger::UnloadDllDebugEvent()
{
}

