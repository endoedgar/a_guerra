#include "stdafx.h"
#include "3DTexture.h"
#include "defines.h"
#include "GerenciadorDeRecursos.h"
#include "GameGlobals.h"
#include "globalheader.h"
#include "D3DObject.h"

CTexture::CTexture():t(NULL), w(0), h(0), ExternalResource(CLASSE_VIDEO)
{
}

HRESULT CTexture::OnLostDevice()
{
	SafeRelease(this->t);
	return S_OK;
}

HRESULT CTexture::OnResetDevice()
{
	HRESULT hr;
	hr = D3DXCreateTextureFromFileEx(d3dDevice, this->getFilename().c_str(), D3DX_DEFAULT, D3DX_DEFAULT, 0, 0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DTEXF_LINEAR, D3DTEXF_LINEAR, 0, NULL, NULL, &this->t);
	if(SUCCEEDED(hr)) {
		this->t->GenerateMipSubLevels();
	}
	return hr;
}

CTexture *CTexture::Create(UINT width, UINT height, UINT miplevels, DWORD usage)
{
	CTexture *tmp = new CTexture();
	HRESULT hr = D3DXCreateTexture(d3dDevice, width, height, miplevels, usage, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &tmp->t);
	if(SUCCEEDED(hr)) {
		tmp->w = width;
		tmp->h = height;
	} else
		SafeRelease(tmp);
	return tmp;
}

CTexture *CTexture::Load(const Smart::String &filename)
{
	Smart::String filepath = filename;

	fixslashs(filepath);
	CTexture *mytex = static_cast<CTexture*>(mywrapper->FindResource(filepath, TR_TEXTURA));
	if(mytex) {
		mytex->AddRef();
		return mytex;
	} else {
		HRESULT hr;
		D3DXIMAGE_INFO image_info;
		CTexture *tmp = new CTexture();
		hr = D3DXCreateTextureFromFileEx(d3dDevice, filepath.c_str(), D3DX_DEFAULT, D3DX_DEFAULT, 0, 0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DTEXF_LINEAR, D3DTEXF_LINEAR, 0, &image_info, NULL, &tmp->t);
		if(SUCCEEDED(hr)) {
			tmp->t->GenerateMipSubLevels();
			tmp->w = image_info.Width;
			tmp->h = image_info.Height;
			tmp->Catalog(filepath, TR_TEXTURA);
			TRACE("Memória de vídeo disponível %d...\n", d3dDevice->GetAvailableTextureMem());
		} else {
			SafeRelease(tmp);
			TRACE("ERRO: Textura não criada! (código %d)\n", HRESULT_CODE(hr));
		}
		return tmp;
	}
}

CTexture::~CTexture()
{
	TRACE("%p::~CTexture() called!\n", this);
	SafeRelease(this->t);
}