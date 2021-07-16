#include "stdafx.h"
#include "GameGlobals.h"
#include "tship.h"
#include "CTerrain.h"
#include "Jogador.h"
#include "3DFunctions.h"
#include "radio.h"
#include "globalheader.h"
#include "3DParticles.h"
#include "3DPlane.h"
#include "Base.h"

void CAirplane::OnSpeed(CObjeto *obj)
{
	if(obj->speed < obj->dados->macel)
		obj->speed += obj->dados->acel;
}

void CAirplane::OnBreak(CObjeto *obj)
{
	if(obj->speed > 0)
		obj->speed -= obj->dados->acel/6;
}

/*void CAirplane::OnUp(CObjeto *obj)
{
	obj->Turn(obj->dados->vv, 0, 0);
}

void CAirplane::OnDown(CObjeto *obj)
{
	obj->Turn(-obj->dados->vv, 0, 0);
}*/

void CAirplane::OnLeft(CObjeto *obj)
{
	obj->Turn(0, 0, obj->dados->hv*obj->dados->getInertiaTensor());
}

void CAirplane::OnRight(CObjeto *obj)
{
	obj->Turn(0, 0, -obj->dados->hv*obj->dados->getInertiaTensor());
}

void CAirplane::CPUMove(CObjeto *obj)
{
	real tmp = terrain->GetTerrainHeight(obj->GetPosition())+static_cast<const DadosDaUnidadeVoadora*>(obj->dados)->getAISafeAlt();
	if(obj->GetPositionY() < tmp) {
		Vector front(0, 0, 1);
		obj->GetQuaternion()->apply(&front, &front);
		obj->ai.wpt.gotoTo(&Vector(obj->GetPositionX()+front.x*8, tmp*2, obj->GetPositionZ()+front.z*8), 0);
		obj->ai.state.set(FSM_PATROL, 1);
	}
	switch(obj->ai.state.get()) {
		case FSM_PATROL:
			if(obj->ai.state.timeToFinish() > 100)
				obj->ai.state.setTimeToFinish(100);
			if(!obj->ai.state.finished()) {
				//if(obj->ai.wpt.reachedSq(obj->GetPosition(), obj->dados->getRadiusSq()))
					obj->IAIrPara(&obj->ai.wpt);
				/*else {
					obj->ai.wpt = Vector(1, 0, 1)*obj->dados->getRadius()*20;
					TFormPoint(&obj->ai.wpt, &obj->ai.wpt, obj, NULL);
					obj->ai.wpt.y = obj->GetPositionY();
				}*/
			} else {
				obj->setTarget(obj->ConsultarObjetoMaisProximo(CONSULTAR_NOVO_ALVO));
				if(obj->getTarget()) {
					ObjetoLiderMandouAtacar(obj);
					obj->ai.state.set(FSM_ATTACK, 1000);
					//if(obj->ai.ehlider)
					//	ObjetoPerguntarSobreAtaque(Obj)
				}
				obj->ai.state.set(FSM_IDLE, random(300));
			}
			break;
		case FSM_GUARD:
		case FSM_CAPTURE:
				obj->dados->ctipo->OnBreak(obj);
				if(obj->getLider() && !obj->GetTarget() && obj->getLider()->GetTarget() && obj->podeAtacar(obj->getLider()->getTarget())) {
					obj->setTarget(obj->getLider()->getTarget());
					obj->ai.state.set(FSM_ATTACK, 1000);
				} else if(!obj->getTarget()) {
					obj->IAAtacar();
				} else {
					ObjetoLiderMandouAtacar(obj);
					obj->ai.state.set(FSM_ATTACK, 1000);
				}
			break;
		default:
			obj->DefCPUMove();
	}
}

void CAirplane::Update(CObjeto *obj)
{
	PROFILE_START;	
	//obj->ImpulseRelative(&Vector(0,0,-1), &(Vector(0, 1, 0)*obj->getMassa()));

	//ColisaoComOTerreno(obj);
	if(obj->getLife() <= 0) {
		obj->setLife(-1000);
		if(obj->dados->ExplodMorrendo)
			if(GameTicks % 2 == 0)
				ps->CriarExplod(obj->GetPosition(), obj->dados->ExplodMorrendo);
		obj->y_vel -= 0.06f;
		obj->Translate(0, obj->y_vel, 0);
	} else {
		if(obj->dados->arma)
			if(obj->shootdelay < obj->dados->stock*obj->dados->arma->shootdelay)
				++obj->shootdelay;
	}

	if(obj->speed < obj->dados->macel*0.3f)
		obj->speed = obj->dados->macel*0.3f;
	obj->Move(0, 0, obj->speed*obj->getMassa());

	const real ty = terrain->GetTerrainHeight(obj->GetPosition());
	if(obj->GetPositionY() <= ty)
	{
		obj->Explodir();
		obj->morto = true;
		obj->Position(obj->GetPositionX(), ty, obj->GetPositionZ());
	}
	if(obj->GetPositionY() <= mar->GetPositionY()) {
		obj->Explodir();
		obj->morto = true;
	}

	PROFILE_END;
}

void CAirplane::move(CObjeto *obj, const CEixo *eixo) {
	CEixo novo = *eixo;//
	novo.limitar(1);
	obj->Turn(obj->dados->vv*-novo.getY()*obj->dados->getInertiaTensor(), 0, obj->dados->hv*novo.getX()*obj->dados->getInertiaTensor());
}

/*void CAirplane::OnTLeft(CObjeto *obj)
{
	this->OnLeft(obj);
}

void CAirplane::OnTRight(CObjeto *obj)
{
	this->OnRight(obj);
}*/