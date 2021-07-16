#include "stdafx.h"
#include "ManterDir.h"

CManterDir::CManterDir(void)
{
	olddir.Reserve(1024);
	GetCurrentDirectory(1024, olddir.GetWritableBuffer());
	olddir.UpdateLength();
}

CManterDir::~CManterDir(void)
{
	SetCurrentDirectory(olddir.c_str());
}
