#ifndef _CVISIBLE_ENTITY
#define _CVISIBLE_ENTITY

#include "3DEntity.h"
#include "GerenciadorDeRecursos.h"
#include "VideoResource.h"

#pragma once

class CVisibleEntity: public VideoResource
{
public:
	CVisibleEntity()//:CResource(CLASSE_VIDEO)
	{ }

	virtual HRESULT Texture(CTexture *tex) = 0;
	virtual HRESULT Render() = 0;
	virtual HRESULT Release() = 0;
};



#endif