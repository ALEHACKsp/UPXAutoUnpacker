#pragma once

#include <string>
#include <vector>
#include <mutex>

class xed_instruction;

class CDebugger
{
public:
	CDebugger(std::string commandLine);
	CDebugger(std::wstring commandLine);
	~CDebugger();

	BOOL AttachProcess();
	BOOL AttachProcess(DWORD dwProcessId);
	void Run();

	BOOL SetBreakPoint(LPVOID lpTarget, BOOL bWow64);
	BOOL SetBreakPointWX86(LPVOID lpTarget);
	BOOL SetBreakPointWX64(LPVOID lpTarget);

	BOOL DeleteBreakPoint(LPVOID lpTarget);
	BOOL HitBreakPointWX86(HANDLE hThread, LPVOID lpTarget);
	BOOL HitBreakPointWX64(HANDLE hThread, LPVOID lpTarget);

	BOOL SingleStepWX86(HANDLE hThread);
	BOOL SingleStepWX64(HANDLE hThread);

private:
	BOOL RunDebug();

	// debug events
	void CreateProcessDebugEvent();
	void CreateThreadDebugEvent();
	void ExceptionDebugEvent();
	void ExitProcessDebugEvent();
	void ExitThreadDebugEvent();
	void LoadDllDebugEvent();
	void OutputDebugStringEvent();
	void RIP_Event();
	void UnloadDllDebugEvent();

private:
	std::wstring		m_wsCommandLine;
	STARTUPINFOW		m_siTarget;
	PROCESS_INFORMATION m_piTarget;
	DEBUG_EVENT			m_deTarget;
	BOOL				m_bWow64;
	BOOL				m_bDebug;
	std::mutex			m_lock;
	std::vector<xed_instruction> m_breakpoints;
	std::vector<xed_instruction> m_instructions;
};
