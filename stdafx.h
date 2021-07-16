#ifndef _STDAFX_H
#define _STDAFX_H

// Arquivo usado para agilizar o tempo de compilação
// Inclua TODAS as bibliotecas utilizadas pelo jogo aqui

#define DIRECTINPUT_VERSION 0x0800

#if defined(_DEBUG) | defined(DEBUG)
	#ifndef STRICT
		#define STRICT
	#endif
	#ifndef D3D_DEBUG_INFO
		#define D3D_DEBUG_INFO
	#endif
#endif

#define SafeRelease(pInterface) if(pInterface != NULL) {ASSERT(SUCCEEDED(pInterface->Release())); pInterface=NULL;}
#define SafeDelete(pObject) if(pObject != NULL) {delete pObject; pObject=NULL;}
#define SafeDeleteArray(pArray) { if(pArray) { delete [] (pArray);   (pArray)=NULL; } }


#include <windows.h>
#include <sstream>
#include <d3d9.h>
#include <d3dx9.h>
#include <time.h>
#include <vector>
#include <stack>
#include <dsound.h>
#include <mmsystem.h>
#include <fstream>
#include <io.h>
#include <process.h>
#include <signal.h>
#include <crtdbg.h>
#include <dinput.h>
#include <dbghelp.h>
#include "SmartLib.h"
#include "defines.h"
#include "InlineList.h"
#include "profile.h"
#include "globalheader.h"
#include "3DMath.h"
#include "Vector.h"
#include "journal.h"

#endif