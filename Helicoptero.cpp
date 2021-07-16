#include "stdafx.h"
#include "Helicoptero.h"
#include "Jogador.h"
#include "Base.h"
#include "3DParticles.h"

void AlinharEstrutura(CObjeto *obj);

void CHelicoptero::CPUMove(CObjeto *obj) {
	obj->DefCPUMove();
}

void CHelicoptero::Update(CObjeto *obj) {
	const real drag = static_cast<real>(0.02);
	Vector forcacontraria;
	//obj->setVelocityDamping(0.98, 0.98, 0.98);
	//obj->setAngularDamping(static_cast<real>(0.05));
	
	obj->Move(0, obj->speed*obj->dados->getMassa(), obj->speed*obj->dados->getMassa());
	
	forcacontraria = -drag**obj->GetVelocity()*obj->getSpeed()*obj->dados->getMassa();
	obj->Translate(&forcacontraria);
	obj->Translate(g_Gravidade*obj->getMassa());
	AlinharEstrutura(obj);

	if(obj->getLife() <= 0) {
		if(obj->dados->ExplodMorrendo)
			if(GameTicks % 2 == 0)
				ps->CriarExplod(obj->GetPosition(), obj->dados->ExplodMorrendo);
		obj->setDamage(0.2f);
		//obj->y_vel -= 0.06f;
		//obj->Translate(0, obj->y_vel, 0);
	} else {
		if(obj->dados->arma)
			if(obj->shootdelay < obj->dados->stock*obj->dados->arma->shootdelay)
				++obj->shootdelay;
	}
}

void CHelicoptero::OnSpeed(CObjeto *obj) {
	if(obj->speed < obj->dados->macel)
		obj->speed += obj->dados->acel;
}

void CHelicoptero::OnBreak(CObjeto *obj) {
	if(obj->speed > obj->dados->macel*0.02)
		obj->speed -= obj->dados->acel;
}

void CHelicoptero::OnLeft(CObjeto *obj) {
	obj->Turn(0, -obj->dados->hv*obj->dados->getInertiaTensor(), 0);
}

void CHelicoptero::OnRight(CObjeto *obj) {
	obj->Turn(0, obj->dados->hv*obj->dados->getInertiaTensor(), 0);
}

void CHelicoptero::move(CObjeto *obj, const CEixo *eixo) {
	CEixo novo = *eixo;//
	novo.limitar(20);
	obj->Turn(-obj->dados->vv*novo.getY()*obj->dados->getInertiaTensor(), 0, obj->dados->hv*novo.getX()*obj->dados->getInertiaTensor());
}