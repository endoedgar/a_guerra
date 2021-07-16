#include "stdafx.h"
#include "ObjetoLiderado.h"
#include "objeto.h"
#include "radio.h"

ObjetoLiderado::ObjetoLiderado(void):lider(NULL), squads(0)
{
}

ObjetoLiderado::~ObjetoLiderado(void)
{
}

void ObjetoLiderado::setLider(CObjeto ** lider)
{
	CObjeto *liderantigo = this->getLider();
	if(liderantigo) {
		liderantigo->squads--;
	}

	this->lider = lider;

	if(lider) 
		(*this->lider)->squads++;
}