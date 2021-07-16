#ifndef _TESTRUTURA_H
#define _TESTRUTURA_H

#include "objeto.h"

class CEstrutura : public CObjetoT
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

void SpeedUp(CObjeto *obj);
void SpeedDown(CObjeto *obj);
void TUp(CObjeto *obj);
void TDown(CObjeto *obj);
void YawLeft(CObjeto *obj);
void YawRight(CObjeto *obj);
void ControleEstrutura(CObjeto *obj);

#endif