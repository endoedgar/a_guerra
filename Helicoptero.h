#ifndef _HELICOPTERO_H
#define _HELICOPTERO_H
#pragma once
#include "objeto.h"

class CHelicoptero :
	public CObjetoT
{
protected:
	virtual void Update(CObjeto *obj);
public:
	virtual void CPUMove(CObjeto *obj);
	virtual void OnSpeed(CObjeto *obj);
	virtual void OnBreak(CObjeto *obj);
	virtual void OnLeft(CObjeto *obj);
	virtual void OnRight(CObjeto *obj);

	virtual void move(CObjeto *obj, const CEixo *);
};
#endif