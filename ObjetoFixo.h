#ifndef _OBJETOFIXO_H
#define _OBJETOFIXO_H

#pragma once

#include "3DEntity.h"
#include "CMesh.h"
#include "3DTexture.h"

class ObjetoFixo : public Entity
{
	CTexture *textura;
	CMesh *mesh;
public:
	ObjetoFixo(void);
	void Render();
	~ObjetoFixo(void);
};

#endif