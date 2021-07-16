#ifndef _GENERIC_FUNCTIONS_H
#define _GENERIC_FUNCTIONS_H

#include "3DMath.h"
#include "INI Parser.h"
using namespace Matematica;
#pragma once

/*bool SetIniString(const Smart::String &, const Smart::String &, const Smart::String &, const Smart::String &);
Smart::String &GetIniString(const Smart::String &, const Smart::String &, const Smart::String &, const Smart::String & = Smart::StringW(""));
int GetIniInt(const Smart::String &, const Smart::String &, const Smart::String &, const int = 0);
real GetIniReal(const Smart::String &, const Smart::String &, const Smart::String &, const real = 0);
bool GetIniBool(const Smart::String &, const Smart::String &, const Smart::String &, const bool DefaultValue = false);*/

unsigned int GetNSubs(const Smart::String &str);
Smart::String GetSubString(const Smart::String &str, const unsigned int pos);

void RuntimeError(const wchar_t *msg, ...);
void RuntimeError(const char *msg, ...);

extern Smart::Ptr<std::wofstream> t_output;

template<class Type>
inline Type GetSub(const Smart::String &str, const unsigned int pos)
{ 
#ifdef UNICODE
	std::wstringstream ss;
#else
	std::stringstream ss;
#endif
	Type r;
	ss << GetSubString(str, pos).c_str();
	ss >> r;
	return r;
}

Vector *GetVector(Vector *vec, const INIParser::INIFile &ini, const Smart::String &section, const Smart::String &item, const Vector &def = Vector::zero());

int for_each_file_ex(const Smart::String &name, int in_attrib, int out_attrib, int (*callback)(const Smart::String &filename, int attrib, void *param), void *param);

Smart::String IntToStr(const int i);

/* emulate the FA_* flags for platforms that don't already have them */
#ifndef FA_RDONLY
   #define FA_RDONLY       1
   #define FA_HIDDEN       2
   #define FA_SYSTEM       4
   #define FA_LABEL        8
   #define FA_DIREC        16
   #define FA_ARCH         32
#endif
   #define FA_NONE         0
   #define FA_ALL          (~FA_NONE)

extern Smart::String GlobalIniBuffer;

#endif