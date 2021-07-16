#include "stdafx.h"
#include "base.h"
#include "loader.h"
#include "objeto.h"
#include "jogador.h"

void CBase::Reinforcements() {
	if(this->Dona()) {
		if(g_Unidades->GetNumActive() < gc->GetMaxUnits()) {
			const real inf = this->dados->getTipo()->getRaioDeInfluencia();
			Vector pos;
			pos.x = this->GetPositionX()-randomf(inf/2)+randomf(inf);
			pos.y = this->GetPositionY()-randomf(inf/2)+randomf(inf);
			pos.z = this->GetPositionZ()-randomf(inf/2)+randomf(inf);
			pos.y = terrain->GetTerrainHeight(&pos);
			const DadosDaUnidade * dados = this->dados->ReceberUmaUnidade();
			if(dados) {
				CObjeto *c = g_Unidades->New();
				if(dados->tipo == TIPO_AERONAVE)
					pos.y += 2000;
				else if(dados->tipo == TIPO_HELICOPTERO)
					pos.y += 500;
				pos.y += 30;
				c->Criar(&pos, 0, this->dona, dados);
				c->ogm = NULL;
			}
		}
	}
}

void CBase::Dominar(const CEquipe *dominador) 
{
	if(this->dona) {
		if(dominador == this->dona) {
			if(this->influencia < 1)
				this->influencia += r(0.001);
		} else {
			this->influencia -= r(0.001);
			if(this->nDefesas)
				this->nDefesas = 0;
			this->nInstabilidade += 300;
		}
		if(this->influencia <= -r(0.10)) {
			if(this->nInstabilidade < static_cast<signed>(GameTicks))
				this->nInstabilidade = GameTicks;
			this->nInstabilidade += 60*120;
			if(g_Jogador->team)
				if(g_Jogador->team == this->dona)
					g_Jogador->mostrarInformacao(baseperdimg);
			this->dona = NULL;
		}
	} else {
		if(this->influencia < 1)
			this->influencia += r(0.001);
		if(this->influencia >= 0) {
			if(g_Jogador->team) {
				if(g_Jogador->team == dominador)
					g_Jogador->mostrarInformacao(basecptimg);
			}
			this->dona = dominador;
		}
	}
}

HRESULT CBase::Render()
{
	this->UpdateMatrix();
	d3dDevice->SetTransform(D3DTS_WORLD, &this->mWorld);
	MeshCubo->Render();
	if(this->dona) {
		flag.UpdateMatrix();
		d3dDevice->SetTransform(D3DTS_WORLD, &flag.mWorld);
		d3dDevice->SetTexture(0, this->dona->getFlag()->Get());
		bmesh->Render();
	}
	return S_OK;
}