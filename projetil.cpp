#include "stdafx.h"
#include "projetil.h"
#include "3DFunctions.h"
#include "CTerrain.h"
#include "radio.h"
#include "globalheader.h"
#include "Jogador.h"

CObjectManager<CProjetil> *g_Projeteis = NULL;

CProjetil::CProjetil()
{
	TRACE("Projétil %p criado!\n", this);
}

void CProjetil::atirar() {
	CObjeto *atirador = (*this->shooter);
	Vector vTemp;

	this->shoottime = GameTicks;
	this->setTorqueDamping(static_cast<real>(0.5));
	this->SetupPhysics();
	this->tipo = atirador->dados->arma;
	this->setMassa(this->tipo->getMassa());
	this->setInertiaTensor(this->tipo->getInertiaTensor());
	this->alvo = NULL;
			
	this->speed = atirador->speed;
	this->mspeed = this->speed + this->tipo->macel;
	this->gas = this->tipo->gas;
	if(this->tipo->subtipo == TIPO_MIRV || this->tipo->subtipo == TIPO_MISSEL) {
		if(atirador->lockOnTime >= static_cast<CHotSeekingProjectileData*>(this->tipo)->lockOnTime) {
			this->time = static_cast<unsigned int>(GameTicks + static_cast<CHotSeekingProjectileData*>(this->tipo)->homingtime);
			if(atirador->podeAtacar(atirador->getTarget()))
				this->alvo = atirador->getTarget();
		}
	}
	this->mirvfilho = false;

	switch(atirador->dados->tipo)
	{
		case TIPO_VEICULO: {
			atirador->Turret->Move(0, 0, -this->tipo->tforca);
			CQuaternion qTmp = *atirador->Turret->GetQuaternion();
			qTmp *= *atirador->GetQuaternion();
			vTemp = *atirador->Turret->GetPosition(true);
			atirador->Turret->UpdateMatrix();
			//D3DXMatrixDecompose(&vTemp2, &qTemp, &vTemp, &this->Turret.mWorld);
			//D3DXVec3TransformCoord(&vTemp, this->dados->shootspot, &this->Turret->mWorld);
			vTemp += atirador->dados->shootspot->Transformed(&atirador->Turret->mWorld);
			//vTemp += *this->GetPosition();

			this->Position(&vTemp);
			this->Rotate(&qTmp);
			this->Rotate(this->pitch()-atirador->dados->recoil+randomf(atirador->dados->recoil*2), this->yaw()-atirador->dados->recoil+randomf(atirador->dados->recoil*2), 0);
			Vector v(*atirador->dados->shootspot), vf(0, 0, -this->tipo->tforca);
					
			TFormPoint(&v, &v, atirador->Turret, NULL);
			TFormPoint(&v, &v, NULL, atirador);
			TFormPoint(&vf, &vf, atirador->Turret, NULL);
			TFormPoint(&vf, &vf, NULL, atirador);
			vf *= this->getMassa();
			atirador->ImpulseRelative(&v, &vf);
			break;
		}
		default:
			Vector v(*atirador->dados->shootspot);
			TFormPoint(&v, &v, atirador, NULL);
			this->Position(&v);
			this->Rotate(atirador->GetQuaternion());
			this->Rotate(this->pitch()-atirador->dados->recoil+randomf(atirador->dados->recoil*2), this->yaw()-atirador->dados->recoil+randomf(atirador->dados->recoil*2), 0);
	}
	if(atirador->dados->arma->subtipo == TIPO_PESADO) {
		this->Move(0, 0, atirador->speed+this->tipo->macel*this->getMassa());
		this->setVelocityDamping(1, 1, 1);
	}
	if(this->tipo->SLaunch)
	{
		atirador->dados->arma->SLaunch->Play3D(this, 0, 0, 0, *this->tipo->SLaunchT);
	}
	
	if(this->tipo->ExplodDisparo)
		ps->CriarExplod(this->GetPosition(), this->tipo->ExplodDisparo);
	this->copyStateToOldState();
}

void CProjetil::Impacto()
{
	if(this->tipo->ExplodImpacto)
		ps->CriarExplod(this->GetPosition(), this->tipo->ExplodImpacto);
}

void CProjetil::BoomMirv()
{
	const real angle = r(5);
	this->UpdateMatrix();
	if(this->tipo->SLaunch)
		this->tipo->SLaunch->Play3D(this, 0, 0, 0, *this->tipo->SLaunchT);
	for(register int c = 0; c < static_cast<CHotSeekingProjectileData*>(this->tipo)->mirvno; ++c) {
		CProjetil *pProj = g_Projeteis->New();
		pProj->speed = this->speed;
		pProj->mspeed = this->mspeed;
		pProj->tipo = this->tipo;
		pProj->alvo = this->alvo;
		pProj->shooter = this->shooter;
		pProj->gas = pProj->tipo->gas;
		pProj->mirvfilho = true;
		pProj->Position(this->GetPosition(true));
		pProj->Rotate(this->GetQuaternion());
		pProj->Turn(D3DXToRadian(randomf(angle)-angle/2), D3DXToRadian(randomf(angle)-angle/2), 0);
	}
	this->mirvfilho = true;
}

bool CProjetil::checarColisao() {
	bool retorna = false;
	CObjeto *obj = g_Unidades->GetFirstActive();
	while(obj)
	{
		if(!this->shooter || (*this->shooter && obj->team != (*this->shooter)->team))
		{
			//const float csize = max(obj->dados->Radius, this->tipo->size);
			const real howmuchf = DistanceSq(&this->getOldState().getPosition(), this->GetPosition());

			if(this->getDistanceSq(obj) <= howmuchf+obj->dados->getRadiusSq()+this->tipo->sizesq) {
				bool colidiu = false;
				const real howmuch = sqrtf(howmuchf);
				Vector postransformed;
				TFormPoint(&postransformed, obj->GetPosition(true), NULL, this);
				const real objradius = obj->dados->getRadius();
				const real col = max(this->tipo->size, objradius);
				colidiu = abs(postransformed.x) <= this->tipo->size+objradius;				
				colidiu &= abs(postransformed.y) <= this->tipo->size+objradius;
				colidiu &= postransformed.z >= -howmuch;
				colidiu &= postransformed.z <= col;

				if(colidiu) {
					Vector wher;
					Vector forca(0, 0, this->getMassa());
					this->GetQuaternion()->apply(&forca, &forca);
					forca *= this->getSpeed();
					//forca *= obj->dados->impactabsorb;

					wher = *this->GetPosition(true);
					wher -= *obj->GetPosition();

					//forca /= 1000;
					obj->Impulse(&wher, &forca);
					obj->setDamage(this->tipo->damage, this->shooter);
					if(obj->j) {
						obj->j->showDamage(this->GetPosition(true));
					}
					//obj->life -= this->tipo->damage;
					
					if(this->tipo->SImpact)
						this->tipo->SImpact->Play3D(obj, 0, 0, 0, *this->tipo->SImpactT);

					retorna = true;
				}
			}
		}
		obj = g_Unidades->GetNext(obj);
	}
	if(retorna)
		this->Impacto();
	return retorna;
}

int CProjetil::UpdateProjetil()
{
	if(this->speed < this->mspeed)
		this->speed += this->tipo->acel;
	if((this->tipo->subtipo == TIPO_MISSEL || this->tipo->subtipo == TIPO_MIRV) && this->alvo && static_cast<CHotSeekingProjectileData*>(this->tipo)->trate != 0 && this->time <= GameTicks) {
		if(!(*this->alvo))
			this->alvo = NULL;
		else
		{
			Vector midpoint;
			TFormVector(&midpoint, (*this->alvo)->dados->midpoint, (*this->alvo), NULL);
			midpoint += *(*this->alvo)->GetPosition();
			const real distance = this->getDistanceSq(midpoint);
			if(distance <= static_cast<CHotSeekingProjectileData*>(this->tipo)->disthoming) {
				Vector TFormed;
				real tx, ty;
				TFormVector(&TFormed, &Vector(0, 0, this->speed), this, 0);
				tx = DeltaPitch(this, &midpoint);
				ty = DeltaYaw(this, &midpoint);
				if(tx*tx + ty*ty <= static_cast<CHotSeekingProjectileData*>(this->tipo)->getAngleHomingSq()) {
					const real turno = static_cast<CHotSeekingProjectileData*>(this->tipo)->tvalue*this->getMassa();
					//const float ca = distance;

					if(tx < -turno)
						this->Turn(-turno, 0, 0);
					else if(tx > turno)
						this->Turn(turno, 0, 0);

					if(ty < -turno)
						this->Turn(0, -turno, 0);
					else if(ty > turno)
						this->Turn(0, turno, 0);
				}

				if(this->tipo->subtipo == TIPO_MIRV && !this->mirvfilho && distance < static_cast<CHotSeekingProjectileData*>(this->tipo)->disthoming/2)
					this->BoomMirv();
			}
		}
	}

	if(this->tipo->subtipo != TIPO_PESADO)
		this->Move(0, 0, this->speed*this->getMassa());
	switch(this->tipo->subtipo)
	{
		// TODO: Laser
		case TIPO_PESADO:
			this->Translate(0, static_cast<real>(-0.1*this->getMassa()), 0);
		default:
			this->UpdatePhysics();
			if(this->checarColisao())
				return 1;
			// TODO: Checagem de colisão
			const real ty = terrain->GetTerrainHeight(this->GetPosition())+this->tipo->size;
			if(this->GetPositionY() <= ty)
			{
				if(this->shooter && *this->shooter) {
					if((*this->shooter)->ai.state.get() == FSM_ATTACK && (*this->shooter)->GetTarget() && (*this->shooter)->dados->tipo == TIPO_VEICULO && ((!(*this->shooter)->dados->getImovel() && this->tipo->subtipo != TIPO_PESADO) || this->tipo->subtipo == TIPO_PESADO)) {
						if(this->tipo->subtipo != TIPO_PESADO) {
							(*this->shooter)->ai.state.set(FSM_FIXPOSITION, 60);
						} else {
							Vector tmp = *(*this->shooter)->GetTarget()->GetPosition()-(*this->GetPosition());
							TFormVector(&tmp, &tmp, NULL, this);
							tmp.normalize();
							(*this->shooter)->ai.aimvar += static_cast<real>(0.01*tmp.z);
							if((*this->shooter)->ai.aimvar < 0)
								(*this->shooter)->ai.aimvar = 0;
						}
					}
				}
				this->Position(this->GetPositionX(), ty, this->GetPositionZ());
				this->Impacto();
				if(this->tipo->SImpact)
					this->tipo->SImpact->Play3D(this, 0, 0, 0, *this->tipo->SImpactT);
				return 1;
			}
	}

	this->gas--;

	if(!this->gas) {
		if(this->tipo->subtipo == TIPO_MIRV)
			if(!this->mirvfilho)
				this->BoomMirv();

		if(this->tipo->ExplodFim)
			ps->CriarExplod(this->GetPosition(), this->tipo->ExplodFim);
		return 1;
	}

	if(this->GetPositionY() <= 0)
	{
		if(this->shooter && *this->shooter) {
			if((*this->shooter)->ai.state.get() == FSM_ATTACK && (*this->shooter)->GetTarget() && (*this->shooter)->dados->tipo == TIPO_VEICULO && ((!(*this->shooter)->dados->getImovel() && this->tipo->subtipo != TIPO_PESADO) || this->tipo->subtipo == TIPO_PESADO)) {
				if(this->tipo->subtipo != TIPO_PESADO) {
					(*this->shooter)->ai.state.set(FSM_FIXPOSITION, 60);
				} else {
					Vector tmp = *(*this->shooter)->GetTarget()->GetPosition()-(*this->GetPosition());
					TFormVector(&tmp, &tmp, NULL, this);
					tmp.normalize();
					(*this->shooter)->ai.aimvar += static_cast<real>(0.1*tmp.z);
					if((*this->shooter)->ai.aimvar < 0)
						(*this->shooter)->ai.aimvar = 0;
				}
			}
		}
		this->Impacto();
		if(this->tipo->SImpact)
			this->tipo->SImpact->Play3D(this, 0, 0, 0, *this->tipo->SImpactT);
		return 1;
	}

	if(this->tipo->ExplodRastro) {
		ps->CriarExplod(this->GetPosition(), this->tipo->ExplodRastro);
	}
	if(this->tipo->subtipo == TIPO_PESADO)
		this->Rotate(this->GetVelocity()->pitch(), this->yaw(), this->pitch());
	return 0;
}

CProjetil::~CProjetil()
{
	TRACE("Projétil %p liberado!\n", this);
}