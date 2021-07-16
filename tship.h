#ifndef _TSHIP_H
#define _TSHIP_H

#include "objeto.h"

class CAirplane : public CObjetoT
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

/*void Acelerar(CObjeto *obj);
void Desacelerar(CObjeto *obj);
void TurnDown(CObjeto *obj);
void TurnUp(CObjeto *obj);
void RollLeft(CObjeto *obj);
void RollRight(CObjeto *obj);
void ControleAeronave(CObjeto *obj);*/


#endif