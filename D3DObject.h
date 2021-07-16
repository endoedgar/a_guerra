#ifndef _D3DOBJECT_H
#define _D3DOBJECT_H

#pragma once

#include "GerenciadorDeRecursos.h"
#include "VideoResource.h"
#include "ExternalResource.h"
#include "Vector.h"

using Matematica::Vector;

void ROnResetDevice();
void ROnLostDevice();
extern LPD3DXSPRITE fontsprlayer;

class D3DWrapper {
	LPDIRECT3D9			  d3dObject;
	LPDIRECT3DDEVICE9	  d3dDevice;
	D3DCOLOR clearcolor, ambientcolor, amb;
	HWND hWnd;
	_D3DZBUFFERTYPE zbuffer;

	std::vector<CResource*> Resources;
public:
	unsigned char AmbientLightR, AmbientLightG, AmbientLightB;
	D3DPRESENT_PARAMETERS	d3dpp;

	class CLight
	{
	public:
		Matematica::Vector vecDir;
		D3DLIGHT9 *light;

		CLight::CLight(LPDIRECT3DDEVICE9 d3dDevice, D3DLIGHTTYPE type);

		CLight::~CLight()
		{
			SafeDelete(this->light);
		}

	};

	CLight *light;

	D3DWrapper();

	HRESULT InitObject();

	inline LPDIRECT3DDEVICE9 GetDevice()
	{ return this->d3dDevice; }

	inline const LPDIRECT3DDEVICE9 GetDevice() const
	{ return this->d3dDevice; }

	bool IsResourceAllocated(const CResource *t);

	void RemoveResource(CResource *t);

	HRESULT SetupStates();
	D3DCOLOR getClearColor(void) const
	{ return this->clearcolor; }
	void SetClearColor(const D3DCOLOR color)
	{ this->clearcolor = color; }
	D3DCOLOR GetAmbientColor(void) const
	{ return this->ambientcolor; }
	void SetAmbientColor(const D3DCOLOR color)
	{
		this->ambientcolor = color;
		this->AmbientLightR = (color >> 16) & 0xFF;
		this->AmbientLightG = (color >> 8) & 0xFF;
		this->AmbientLightB = (color) & 0xFF;
	}

	void SetAmbColor(const D3DCOLOR color)
	{ this->amb = color; }

	void SetLight(const D3DCOLOR lightcolor, const Vector &lightdir);

	HRESULT SetGFXMode(HWND hWnd, const bool UseZBuffer, const bool UseZ32Bits, const D3DSWAPEFFECT SwapEffect);
	ExternalResource *FindResource(const Smart::String &filename, const CRECURSO_TIPO tipo);
	void AddResource(CResource *r);

	HRESULT OnLostDevice();
	HRESULT OnResetDevice();
	HRESULT SizeChange();
	HRESULT Reset();

	~D3DWrapper();
};

#endif