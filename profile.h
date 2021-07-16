#ifndef _PROFILE_H
#define _PROFILE_H

#ifdef _PROFILE
#pragma once

using std::fstream;

class ProfileInfo {
	typedef struct tagSFuncInfo
	{
		char *name;
		unsigned int calls;
		float stime, ftime, ltime, mtime;
	} SFUNCINFO;
private:
	CSimpleList <SFUNCINFO> *funcs;
	ProfileInfo::SFUNCINFO *ProcurarFunc(const char *name);
	CInlineStack <SFUNCINFO> *stack;
public:
	ProfileInfo();
	~ProfileInfo();

	void Begin(const char *name);
	void End(const char *name);
};

extern ProfileInfo *g_ProfileInfo;

#define PROFILE_START g_ProfileInfo->Begin(__FUNCTION__)
#define PROFILE_END g_ProfileInfo->End(__FUNCTION__)

#else
	#define PROFILE_START
	#define PROFILE_END
#endif

#endif