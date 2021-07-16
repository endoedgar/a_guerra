#include "stdafx.h"
#include "GameGlobals.h"
#include "MiniMapa.h"
#include "CTerrain.h"

MiniMapa::MiniMapa(const int x, const int y, const int w, const int h):x(x), y(y), w(w), h(h)
{
	this->textura = CTexture::Create(w, h, 1, D3DUSAGE_RENDERTARGET);
}

MiniMapa::~MiniMapa(void)
{
	SafeRelease(this->textura);
}

void MiniMapa::desenharMiniMapa(const Vector &pos)
{
	LPDIRECT3DSURFACE9 renderTargetOriginal;
	HRESULT hr;

	hr = d3dDevice->GetRenderTarget(0, &renderTargetOriginal);
	if(SUCCEEDED(hr)) {
		LPDIRECT3DSURFACE9 pSurface;
		hr = this->textura->Get()->GetSurfaceLevel(0, &pSurface);
		if(SUCCEEDED(hr)) {
			hr = d3dDevice->SetRenderTarget(0, pSurface);
			if(SUCCEEDED(hr)) {
				const real mapwidth = w/(pos.y/10000);
				const real mapheight = h/(pos.y/10000);
				gh->SetDrawMode(GameHUD::MODES::F_CENTER);
				gh->RenderQuad(((-pos.x*mapwidth)/terrain->GetScale()->x)+mapwidth/2, ((-pos.z*mapheight)/terrain->GetScale()->z)+mapheight/2, mapwidth, mapheight, terrain->getTexture());
				gh->Render();
			}
			pSurface->Release();
		}
		d3dDevice->SetRenderTarget(0, renderTargetOriginal);
		if(SUCCEEDED(hr)) {
			gh->SetDrawMode(GameHUD::MODES::F_ALPHABLENDTRANS);
			gh->RenderQuad(this->x, this->y, this->w, this->h, this->textura);
			gh->Render();
		}
	}
}