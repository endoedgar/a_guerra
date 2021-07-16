#include "stdafx.h"
#include "3DSprite.h"
#include "GameGlobals.h"
#include "profile.h"

CSprite::CSprite():su(NULL)
{
}

CSprite *CSprite::Load(const Smart::String &filename)
{
	CSprite *spr = NULL;
	if(this)
		spr = this;
	else
		spr = new CSprite();

	spr->Cleanup();
	spr->su = new SparkUnit;
	spr->su->blendflags = 3;

	if(spr->su) {
		spr->su->textura = spr->su->textura->Load(filename);
		if(spr->su->textura) {
			return spr;
		}
	}

	SafeDelete(spr);
	return NULL;
}
HRESULT CSprite::Render()
{
	PROFILE_START;
	D3DXMATRIX mWorld;
	Vector pos;
	d3dDevice->GetTransform(D3DTS_WORLD, &mWorld);
	pos.x = mWorld._41;
	pos.y = mWorld._42;
	pos.z = mWorld._43;
	ps->CreateParticle(&pos, &Vector(0, 0, 0), 255, 0, 0, 1, 0, this->su, UpdateSprite);
	PROFILE_END;
	return S_OK;
}

HRESULT CSprite::Texture(CTexture *tex)
{
	this->su->textura = tex;
	tex->AddRef();

	return S_OK;
}

HRESULT CSprite::Release()
{
	delete this;
	return S_OK;
}

void CSprite::Cleanup(void)
{
	if(this->su) {
		SafeRelease(this->su->textura);
	}
	SafeDelete(this->su);
}

CSprite::~CSprite()
{
	this->Cleanup();
}