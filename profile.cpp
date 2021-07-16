#include "stdafx.h"
#include "profile.h"
#include "GameGlobals.h"
#include "globalheader.h"

#ifdef _PROFILE
ProfileInfo *g_ProfileInfo = NULL;

ProfileInfo::ProfileInfo()
{
	this->funcs = new CSimpleList<SFUNCINFO>;
	this->stack = new CInlineStack<SFUNCINFO>;
}

ProfileInfo::SFUNCINFO *ProfileInfo::ProcurarFunc(const char *name)
{
	CSimpleNode<SFUNCINFO> *func = this->funcs->GetFirst();
	while(func)
	{
		SFUNCINFO *funcao = func->GetPtr();
		if(strcmp(funcao->name, name) == 0)
			return funcao;
		func = func->GetNext();
	}
	return NULL;
}

void ProfileInfo::Begin(const char *name)
{
	SFUNCINFO *ptr = this->ProcurarFunc(name);
	if(!ptr)
	{
		ptr = new SFUNCINFO;
		ptr->name = new char[strlen(name)+1];
		ptr->ftime = 0x00FFFFFF;
		ptr->ltime = 0;
		ptr->mtime = 0;
		ptr->calls = 0;
		strcpy(ptr->name, name);
		this->funcs->Add(ptr);
	}
	++ptr->calls;
	ptr->stime = GetTime();
	this->stack->Push(ptr);
}

void ProfileInfo::End(const char *name)
{
	SFUNCINFO *ptr = this->stack->Get();
	const float end = GetTime()-ptr->stime;
	if(end < ptr->ftime)
		ptr->ftime = end;
	if(end > ptr->ltime)
		ptr->ltime = end;
	if(end)
		ptr->mtime += ptr->ltime - ptr->ftime;
	this->stack->Pop();
}

const char htmlstart[] = " \
	<html> \
	<head> \
	<title>A Guerra - Profile Info</title> \
	</head> \
	<body>";

const char htmlend[] = " \
	</body> \
	</html>";

ProfileInfo::~ProfileInfo()
{
	fstream file;
	CSimpleNode<SFUNCINFO> *func = this->funcs->GetFirst();

	SYSTEM_INFO sinfo;
	MEMORYSTATUS minfo;
	DWORD osver;

	GetSystemInfo(&sinfo);
	GlobalMemoryStatus(&minfo);
	osver = GetVersion();

	std::stringstream fname;
	tm timemark;
	__time64_t tempo = _time64(NULL);
	_localtime64_s(&timemark, &tempo);
	fname << "profiles/profiledata" << timemark.tm_mday << timemark.tm_mon << timemark.tm_year << "-" << timemark.tm_hour << "." << timemark.tm_min << "." << timemark.tm_sec << ".htm";
	file.open(fname.str().c_str(), std::ios_base::out);
	if(file.is_open())
	{
		file << htmlstart;

		file << "<b>Software:</b><br>" << std::endl;
		file << "Windows(r) " << static_cast<int>(LOBYTE(LOWORD(osver))) << "." << static_cast<int>(HIBYTE(LOWORD(osver))) << " (Compilação " << HIWORD(osver) << ")<br>" << std::endl;
		file << "<hr>" << std::endl;
		file << "<b>Hardware:</b><br>" << std::endl;
		file << "<b>Número de Processadores:</b> " << sinfo.dwNumberOfProcessors << "<br>" << std::endl;
		file << "<b>Tipo do Processador:</b> " << sinfo.dwProcessorType << "<br>" << std::endl;
		file << "<b>Arquitetura do Processador:</b> " << sinfo.wProcessorArchitecture << "<br>" << std::endl; 
		file << "<b>Nível do Processador:</b> " << sinfo.wProcessorLevel << "<br>" << std::endl;
		file << "<b>Revisão do Processador:</b> " << sinfo.wProcessorRevision << "<br>" << std::endl;
		file << "<hr>" << std::endl;
		file << "<b>Memórias</b><br>" << std::endl;
		file << "<b>Memória RAM:</b><br>" << std::endl;
		file << "<b>Livre:</b> " <<  minfo.dwAvailPhys/1024.0f/1024.0f << " MBs <b>Usada:</b> " << (minfo.dwTotalPhys-minfo.dwAvailPhys)/1024.0f/1024.0f << " MBs <b>Total:</b> " << minfo.dwTotalPhys/1024.0f/1024.0f << " MBs<br>" << std::endl;
		file << "<b>Memória Páginada:</b> " << minfo.dwTotalPageFile/1024.0f/1024.0f << " MBs<br>" << std::endl;
		file << "<hr>" << std::endl;

		while(func)
		{
			SFUNCINFO *funcao = func->GetPtr();

			const float lagrate = ((static_cast<float>(funcao->mtime)/funcao->calls)/funcao->ltime)*100;
			char *cor = "#008800";

			file << "<b>Função \"" << funcao->name << "\"</b><br>" << std::endl;
			file << "<b>Tempo Mais Rápido:</b> " << funcao->ftime*1000 << " ms<br>" << std::endl;
			file << "<b>Tempo Mais Longo:</b> " << funcao->ltime*1000 << " ms<br>" << std::endl;
			if(lagrate >= 50)
				cor = "#880000";
			else if(lagrate >= 25)
				cor = "#888800";
			file << "<font color = \"" << cor << "\"><b>Lag:</b> " << lagrate << "%</font><br>" << std::endl;
			file << "<b>Chamadas:</b> " << funcao->calls << "<br>" << std::endl;
			file << "<hr>" << std::endl;
			func = func->GetNext();
		}
		file << htmlend;
		file.flush();
		file.close();
	}

	func = this->funcs->GetFirst();
	while(func)
	{
		SFUNCINFO *funcao = func->GetPtr();

		SafeDeleteArray(funcao->name);

		func = func->GetNext();
	}
	SafeDelete(this->funcs);
	SafeDelete(this->stack);
}

#endif