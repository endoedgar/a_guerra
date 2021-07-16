#ifndef _CRASHHANDLER_H
#define _CRASHHANDLER_H

#pragma once

const char s_miniDumpFileName[MAX_PATH] = "crash.dmp";
const char s_reportFileName[MAX_PATH] = "crash.txt";

namespace StackTrace
{
#if defined (_MSC_VER)
	typedef const void*	Address;
#else
#	error "StackTrace not implemented for this platform!"
#endif

	bool InitSymbols();

	int GetCallStack(Address* callStack, int maxDepth, int entriesToSkip = 0);
	int GetCallStack(void* context, Address* callStack, int maxDepth, 
		int entriesToSkip = 0);
	// Faster than other versions, but may be less reliable (no FPO).
	int GetCallStack_Fast(Address* callStack, int maxDepth, int entriesToSkip = 0);

	// @return	Number of chars taken by symbol info.
	int GetSymbolInfo(Address address, char* symbol, int maxSymbolLen);
	// Retrieves whole callstack (using given context), optionally with 4 first function
	// arguments, in readable form.
	// @pre	context != 0
	void GetCallStack(void* context, bool includeArguments, char* callStackStr, int maxBufferLen);
}

struct MemoryStatus
{
	size_t	totalFree;
	size_t	largestFree;
	size_t	totalReserved;
	size_t	totalCommited;

	MemoryStatus::MemoryStatus()
	:	totalFree(0),
		largestFree(0),
		totalReserved(0),
		totalCommited(0)
	{
	}

	static MemoryStatus GetCurrent();
};

extern volatile unsigned long s_inFilter;

LONG WINAPI MyExceptionFilter(EXCEPTION_POINTERS* exceptionPtrs);
void WriteProcessName(FILE* f);
void WriteSystemInfo(FILE* f);
void WriteProcessorInfo(FILE* f);
void WriteDateTime(FILE* f);
void WriteHeader(FILE* f);
const char* GetExceptionString(DWORD exc);
void WriteExceptionInfo(FILE* f, EXCEPTION_POINTERS* exceptionPtrs);
void WriteEnvironmentInfo(FILE* f);
bool WriteMiniDump(EXCEPTION_POINTERS* exceptionPtrs, const char* fileName);
void WriteMemoryStatus(FILE* f, const MemoryStatus& status);
void WriteRegisters(FILE* f, EXCEPTION_POINTERS* exceptionPtrs);
void WriteCallStack(FILE* f, PCONTEXT context);
void InitCrashHandler();


#endif