#ifndef _3DSPRITE_H
#define _3DSPRITE_H

#pragma once

#include "loader.h"
#include "3DParticles.h"
#include "CVisibleEntity.h"

class CSprite: public CVisibleEntity {
	SparkUnit *su;
	void Cleanup();
	CSprite();
	~CSprite();
public:

	CSprite *Load(const Smart::String &);
	virtual HRESULT Render();
	virtual HRESULT Texture(CTexture *tex);
	virtual HRESULT Release();

	virtual HRESULT OnLostDevice() {
		return S_OK;
	}
	virtual HRESULT OnResetDevice() {
		return S_OK;
	}
};

#endif