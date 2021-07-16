#include "stdafx.h"
#include "Carcaca.h"
#include "GameGlobals.h"
#include "CTerrain.h"

D3DMATERIAL9 queimado = {
	0x00000000,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0x00000000,
	50
};

Carcaca::Carcaca(void)
{
	this->setVelocityDamping(0.5, 1, 0.5);
}

bool Carcaca::Update() {
	if(this->getGravity())
		this->Translate(g_Gravidade);
	Entity::UpdatePhysics();
	if(GameTicks <= this->getLifetime() || (this->GetPositionY()+60 <= terrain->GetTerrainHeight(Entity::GetPosition())))
		return true;
	return false;
}

void Carcaca::Render() {
	this->UpdateMatrix();
	if(this->mesh) {
		d3dDevice->SetRenderState(D3DRS_SPECULARENABLE, TRUE);
		d3dDevice->SetMaterial(&queimado);
		d3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR, this->getCor()); 
		d3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE2X);
		d3dDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
		d3dDevice->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_TFACTOR);
		d3dDevice->SetTransform(D3DTS_WORLD, &this->mWorld);
		this->mesh->Render();
		d3dDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		d3dDevice->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
		d3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		d3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR, 0xFFFFFFFF);
		d3dDevice->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
	}
}

Carcaca::~Carcaca(void)
{
}
