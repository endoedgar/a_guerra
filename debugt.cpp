#include "stdafx.h"
#include "GameGlobals.h"
#include "debugt.h"
#include "defines.h"
#include "globalheader.h"
#include "GenericFunctions.h"


#ifdef _BETA

/* my_assert:
 *  Raises an assert (uses ASCII strings).
 */
void my_assert(const char *file, const int line)
{
	static bool asserted = false;
#ifdef UNICODE
	std::wostringstream ss, ss2;
#else
	std::ostringstream ss, ss2;
#endif

	if(asserted)
		return;

	/* todo, some day: use snprintf (C99) */
	ss << "Assert failed at line " << line << " of " << file;

	ss2 << "\nAssert Failed: " << ss.str().c_str() << "\n\n";
	//TRACE(ss2.str().c_str());
	MessageBox(NULL, ss.str().c_str(), Smart::String("Assert Failed").c_str(), MB_OK | MB_ICONERROR);

	asserted = true;
	if(hWnd)
		SendMessage(hWnd, WM_CLOSE, NULL, NULL);
	else {
		unexpected();
		terminate();
	}
}


/* my_trace:
 *  Outputs a trace message (uses ASCII strings).
 */
void my_traceA(const char *msg, ...)
{
	static char buf[2048];
	ASSERT(msg)
	ASSERT(strlen(msg) <= sizeof(buf))

	/* todo, some day: use vsnprintf (C99) */
	va_list ap;
	va_start(ap, msg);
	vsnprintf(buf, sizeof(buf), msg, ap);
	va_end(ap);

	if(!t_output)
		t_output = new std::wofstream();

	if(!t_output->is_open())
		t_output->open("tracedebug.log", std::ios::out);

	if(t_output->is_open())
	{
		*t_output << buf;
		t_output->flush();
	}

	#ifdef _DEBUG
	OutputDebugStringA(buf);
	#endif
}

/* my_trace:
 *  Outputs a trace message (uses ASCII strings).
 */
void my_traceW(const wchar_t *msg, ...)
{
	static wchar_t buf[2048];
	ASSERT(msg)
	ASSERT(wcslen(msg)*sizeof(wchar_t) <= sizeof(buf))

	/* todo, some day: use vsnprintf (C99) */
	//va_list ap;
	//va_start(ap, msg);
	//vswprintf(buf, sizeof(buf)/sizeof(wchar_t), msg, ap);
	//va_end(ap);

	if(!t_output)
		t_output = new std::wofstream();

	if(!t_output->is_open())
		t_output->open("tracedebug.log", std::ios::out);

	if(t_output->is_open())
	{
		*t_output << buf;
		t_output->flush();
	}

	#ifdef _DEBUG
	OutputDebugStringW(buf);
	#endif
}

#endif