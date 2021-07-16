#ifndef _DEFINES_H
#define _DEFINES_H

#pragma once

#define FPS 60
#define APP_WIDTH  640
#define APP_HEIGHT 480



#ifdef _BETA
	#include "debugt.h"

	#ifdef _DEBUG
		#define ASSERT(condition)		{ if (!(condition)) my_assert(__FILE__, __LINE__); }
		#define TRACE					my_traceA
		#define TRACEW					my_traceW
	#else
		#define ASSERT(condition)
		#define TRACE					my_traceA
		#define TRACEW					my_traceW
	#endif
#else
	#define ASSERT(condition)
	#define TRACE
	#define TRACEW
#endif

#include "SmartPtr.h"
const Smart::String APP_TITLE("A Guerra - Versão C++");


#endif