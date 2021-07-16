#ifndef _3DTEXTURE_H
#define _3DTEXTURE_H

#include "ExternalResource.h"
#include "VideoResource.h"

#pragma once

class CTexture : public ExternalResource, public VideoResource
{
protected:
	LPDIRECT3DTEXTURE9 t;
	unsigned int w, h;
	CTexture();
	~CTexture();
public:
	static CTexture *Create(UINT width, UINT height, UINT miplevels, DWORD usage);
	static CTexture *Load(const Smart::String &filename);
	inline const LPDIRECT3DTEXTURE9 Get() const
	{ return this->t; }
	inline unsigned int GetWidth() const
	{ return this->w; }
	inline unsigned int GetHeight() const
	{ return this->h; }
	HRESULT OnLostDevice();
	HRESULT OnResetDevice();
	//HRESULT LoadAndMaskTexture(LPDIRECT3DDEVICE9 d3dDevice, const char *filename, const unsigned char r, const unsigned char g, const unsigned char b);
};

#endif