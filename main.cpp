// virtualized_asm.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include "pch.h"

#include "xed_lib.hpp"
#include "debugger.hpp"

int _tmain(int argc, _TCHAR* argv[])
{
	xed_tables_init();

	CDebugger *debugProcess;
	if (argc < 2)
	{
		debugProcess = new CDebugger(L"C:\\Test\\CodeVirtualizedTest.exe");
	}
	else
	{
		debugProcess = new CDebugger(argv[1]);
	}

	if (debugProcess->AttachProcess())
	{
		debugProcess->Run();
	}

	delete debugProcess;

	getchar();

	return 0;
}
