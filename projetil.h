#ifndef _PROJETIL_H
#define _PROJETIL_H

#include "GameGlobals.h"
#include "3DParticles.h"
#include "3DEntity.h"
#include "loader.h"
#include "objeto.h"
#include "inlinelist.h"

class CProjetil : public Entity, public CInlineNode {
	unsigned int shoottime;
public:
	CProjectileData *tipo;
	CObjeto **alvo, **shooter;
	unsigned int *banco, gas, time;
	real speed, mspeed;
	bool mirvfilho;

	CProjetil();
	~CProjetil();

	void atirar();

	void BoomMirv();
	void Impacto();
	int UpdateProjetil();
	bool checarColisao();
};

extern CObjectManager<CProjetil> *g_Projeteis;

#endif