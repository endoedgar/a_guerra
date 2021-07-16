#include "stdafx.h"
#include "GameGlobals.h"
#include "testrutura.h"
#include "CTerrain.h"
#include "Jogador.h"
#include "3DFunctions.h"
#include "radio.h"
#include "globalheader.h"
#include "3DParticles.h"
#include "3DPlane.h"
#include "Base.h"

// TODO: Melhorar a física

void CEstrutura::OnSpeed(CObjeto *obj)
{
	//if(obj->hitfloor) {
		if(obj->speed < obj->dados->macel)
			obj->speed += obj->dados->acel;
	//}
}

void CEstrutura::OnBreak(CObjeto *obj)
{
	obj->freio = true;
}

/*void CEstrutura::OnTLeft(CObjeto *obj)
{
	obj->Turret->Turn(0, -D3DXToRadian(6), 0);
}

void CEstrutura::OnTRight(CObjeto *obj)
{
	obj->Turret->Turn(0, D3DXToRadian(6), 0);
}*/

void CEstrutura::OnLeft(CObjeto *obj)
{
	obj->dir = -1;
	//obj->Turn(0, -obj->dados->hv*obj->dados->getInertiaTensor(), 0);
}

void CEstrutura::OnRight(CObjeto *obj)
{
	obj->dir = 1;//obj->Turn(0, obj->dados->hv*obj->dados->getInertiaTensor(), 0);
}

/*void CEstrutura::OnDown(CObjeto *obj)
{
	obj->Turret->Turn(obj->dados->vv, 0, 0);
}

void CEstrutura::OnUp(CObjeto *obj)
{
	//if(obj->j)
	//	TRACE("%f %f %f %f\n", obj->Turret.GetQuaternion()->x, obj->Turret.GetQuaternion()->y, obj->Turret.GetQuaternion()->z, obj->Turret.GetQuaternion()->w);
	obj->Turret->Turn(-obj->dados->vv, 0, 0);
}*/

void CEstrutura::move(CObjeto *obj, const CEixo *eixo) {
	CEixo novo = *eixo;//
	const DadosDaUnidadeTerrestre *du = static_cast<const DadosDaUnidadeTerrestre *>(obj->dados);
	novo.limitar(1);
	obj->Turret->Turn(du->tmove*novo.getY(), du->tmove*novo.getX(), 0);
}

void AlinharEstrutura(CObjeto *obj)
{
	PROFILE_START;
	// passo1
	/*for(register unsigned int a = 0; a < 4; ++a) {
		Entity *side = obj->side[a];
		side->setVelocityDamping(0, obj->dados->suspensionDamping, 0);
		real theight = terrain->GetTerrainHeight(side->GetPosition(true));
		if(side->GetPositionY(true) <= theight) {
			side->tempf = ((theight-side->GetPositionY(true))-side->GetVelocity()->y);
			side->Translate(0, side->tempf, 0);
		}// else
		//	side->Translate(0, -0.005, 0);
	}*/
	// passo2

	//static Vector front, back, left, right;
	//front = *obj->side[0]->GetPosition(true);
	//back = *obj->side[1]->GetPosition(true);
	//left = *obj->side[2]->GetPosition(true);
	//right = *obj->side[3]->GetPosition(true);
	//static const Vector &fivetop = *obj->dados->p1;
	//static const Vector &fivebottom = *obj->dados->p2;
	//static const Vector &fiveleft = *obj->dados->p3;
	//static const Vector &fiveright = *obj->dados->p4;

	//TFormPoint(&front, &fivetop, obj, NULL);
	//TFormPoint(&back, &fivebottom, obj, NULL);
	//TFormPoint(&left, &fiveleft, obj, NULL);
	//TFormPoint(&right, &fiveright, obj, NULL);

	//real fronty = terrain->GetTerrainHeight(&front);
	//real backy = terrain->GetTerrainHeight(&back);
	//real lefty = terrain->GetTerrainHeight(&left);
	//real righty = terrain->GetTerrainHeight(&right);

	//for(register unsigned int a = 0; a < 4; ++a) {
		//tmp *= obj->side[a]->tempf;
	//	obj->Impulse(&obj->dados->p[a], &Vector(0,obj->side[a]->tempf*0.1,0));
	//}

	//if(front.y < fronty || back.y < backy) 
	/*{
		const real hyp = back.y - front.y;
		const real line = Distance(&back, &front);
		real hypsline = hyp/line;
		if(hypsline > 1.0f)
			hypsline = fmod(hypsline, 1);
		else if(hypsline < -1.0f)
			hypsline = fmod(hypsline, -1);
		const real angle = asin(hypsline);
		obj->Rotate(angle, obj->Yaw(), 0);
	//
	}
	//if(left.y < lefty || right.y < righty) 
		{
		const real hyp2 = right.y - left.y;
		const real line2 = Distance(&left, &right);
		real hypsline = hyp2/line2;
		if(hypsline > 1.0f)
			hypsline = fmod(hypsline, 1);
		else if(hypsline < -1.0f)
			hypsline = fmod(hypsline, -1);
		const real angle = asin(hypsline);
		obj->Rotate(obj->Pitch(), obj->Yaw(), angle);
	//
		}*/

	// passo 3
	for(register unsigned int a = 0; a < 4; ++a) {
		//obj->side[a]->Update(obj->dados->p[a]);
		/*Vector whereshouldbe;
		Vector final;
		Entity *side = obj->side[a];

		side->setVelocityDamping(0, obj->dados->suspensionDamping, 0);
		real theight = terrain->GetTerrainHeight(side->GetPosition(true));
		if(side->GetPositionY(true) <= theight) {
			side->tempf = ((theight-side->GetPositionY(true))-side->GetVelocity()->y)+obj->GetVelocity()->y;
			side->Translate(0, side->tempf, 0);
		}

		whereshouldbe = obj->dados->p[a];

		// abrams 0.02
		final = ((whereshouldbe-*side->GetPosition())*obj->dados->suspensionK);
		
		//final.y -= 0.07;
		//if(abs(final.y) > 2)
		//	assert(1);
		side->Translate(&final);
		side->UpdatePhysics();
		side->UpdateMatrix();

		final = -final;
		final *= obj->getMassa();
		final *= obj->dados->suspensionEL;
		obj->Impulse(&obj->dados->p[a], &final);*/
	}

	PROFILE_END;
}

void ColisaoParaOPonto(CObjeto *obj, const Vector &p) {
	real penetrationDepth;
	Vector normal;

	terrain->GetPointInformation(&p, penetrationDepth, normal);
	normal = -normal;
	if(penetrationDepth > 0) {
		Vector velocity;
		Vector force;

		//obj->GetQuaternion()->apply(&vFront, &vFront);
		//vFront.y = 0;
		//vFront.normalize();
		//f = 0.08
		const real c = 0.16, f = terrain->friction, k = 0.03, b = 0;
		const Vector pontoRelativo = p-(*obj->GetPosition(true));

		force.clear();
		obj->velocityAtPoint(&velocity, &pontoRelativo);

		const real relativeSpeed = -normal.dot(velocity);

		if(relativeSpeed > 0) {
			force += normal * (c * relativeSpeed);// - colB * normal * (normal.dot(velocity));
			//force *= obj->getMassa();	
			//obj->Impulse(&pontoRelativo, &force);
			if(obj->dados->ExplodColisao) {
				ps->CriarExplod(&p, obj->dados->ExplodColisao, obj->ptr->GetPtr());
			}
		}

		Vector tangentialVelocity = velocity + (normal * relativeSpeed);
		Vector frictionForce = -tangentialVelocity * f;
		force += frictionForce;	

		Vector penaltyForce = normal * (penetrationDepth * k);
		force += penaltyForce;	

		Vector dampingForce = normal * (relativeSpeed * penetrationDepth * b);	
		force += dampingForce;

		obj->setDamage(force.lengthsq()*gc->getRazaoDeDanoDeTerreno());

		force *= obj->getMassa();
		obj->Impulse(&pontoRelativo, &force);
	}
}

void ColisaoComOTerreno(CObjeto *obj)
{
	for(register unsigned int i = 0; i < 8; ++i) {
		const Vector verticeTFormed;
		*TFormPoint(const_cast<Vector*>(&verticeTFormed), &obj->mshCol.getVertice(i), obj, NULL);

		ColisaoParaOPonto(obj, verticeTFormed);
	}
}

void reagirAColisao(CObjeto *obj, CObjeto *collider, const Vector &p, const real penetrationDepth, const Vector &normal);

void CEstrutura::Update(CObjeto *obj)
{
	const DadosDaUnidadeTerrestre *dadosunidade = static_cast<const DadosDaUnidadeTerrestre*>(obj->dados);
	obj->Translate(g_Gravidade*obj->getMassa());
	ColisaoComOTerreno(obj);
	AlinharEstrutura(obj);
	obj->hitfloor = obj->suspensao[0].collided() + obj->suspensao[1].collided() + obj->suspensao[2].collided() + obj->suspensao[3].collided();

	if(obj->hitfloor >= 2) {
	//Vector tmp(&obj->mshCol.getVertice(0));
	//tmp = TFormPoint(&tmp, &tmp, obj, NULL); 
	//if(tmp.y <= terrain->GetTerrainHeight(&tmp)+1) {
		Vector fRes;
		Vector speed(0, 0, obj->speed);
		obj->GetQuaternion()->apply(&speed, &speed);
		fRes.clear();

		fRes = -static_cast<const DadosDaUnidadeTerrestre*>(obj->dados)->resistenciaAr**obj->GetVelocity()*obj->getSpeed(); // drag

		//if(obj->hitfloor) {
		//	fRes += -1**obj->GetVelocity(); // rolling drag
			//obj->setAngularDamping(static_cast<real>(0.8));
		//} 
	
		fRes += speed;
		fRes *= obj->getMassa();
		obj->Translate(fRes);
		obj->Turn(0, obj->dir*obj->dados->hv*obj->dados->getInertiaTensor(), 0);
	//} else {
		//obj->setAngularDamping(static_cast<real>(0.1));
	}

	obj->speed /= 2;

	if(obj->getLife() <= 0) {
		if(obj->dados->ExplodMorrendo)
			if(GameTicks % 2 == 0)
				ps->CriarExplod(obj->GetPosition(), obj->dados->ExplodMorrendo);
		obj->setDamage(0.2f);
	}
	
	//real groundlevel = terrain->GetTerrainHeight(obj->GetPosition());

	//if(obj->GetPositionY() > groundlevel) {
	//	obj->y_vel -= 0.06f;
	//} else {		
		//obj->Position(obj->GetPositionX(), groundlevel, obj->GetPositionZ());
	//	obj->y_vel = 0;
	//}

	if(obj->GetPositionY() <= mar->GetPositionY()) {
		obj->setDamage(9999);
		obj->Explodir();
		obj->morto = true;
	}

	if(obj->dados->arma)
		if(obj->shootdelay < obj->dados->stock*obj->dados->arma->shootdelay)
			++obj->shootdelay;

	//obj->Move(0, 0, obj->speed*obj->getMassa());
	//obj->Translate(0, obj->y_vel*obj->getMassa(), 0);
	obj->Turret->Translate((dadosunidade->turretpos.x-obj->Turret->GetPositionX())*dadosunidade->tforce, (dadosunidade->turretpos.y-obj->Turret->GetPositionY())*dadosunidade->tforce, (dadosunidade->turretpos.z-obj->Turret->GetPositionZ())*dadosunidade->tforce);
	obj->Turret->UpdatePhysics();

	const real turretPitch = obj->Turret->pitch();
	const real turretYaw = obj->Turret->yaw();
	if(turretPitch > dadosunidade->tmaxpitch)
		obj->Turret->Rotate(dadosunidade->tmaxpitch, turretYaw, 0);
	else if(turretPitch < dadosunidade->tminpitch)
		obj->Turret->Rotate(dadosunidade->tminpitch, turretYaw, 0);

	CObjeto *pObj = g_Unidades->GetNext(obj);
	while(pObj)
	{
		
		if(obj != pObj) 
		{
			real dist = obj->getDistanceSq(pObj);
			real sraio = obj->dados->getRadiusSq()+pObj->dados->getRadiusSq();
			if(dist <= sraio) {
				Vector normal = (*pObj->GetPosition()-*obj->GetPosition());
				Vector tmp; // Velocidade balanceada
				normal.normalize();
				normal = -normal;
				
				const Vector p = (normal*obj->dados->getRadius())+(*obj->GetPosition(true));
				const real penetrationDepth = sqrtf(sraio-dist);

				reagirAColisao(obj, pObj, p, penetrationDepth, normal);
				reagirAColisao(pObj, obj, p, penetrationDepth, -normal);

				//this->Translate(&force);

				//force = -force;
				//force *= this->pParent->getMassa();
				//this->pParent->Impulse(&p, &force);

				//this->collidedAtFrame(GameTicks);


				/*Vector w, f, fp, pontocolisao;
				sraio = sqrt(sraio);

				pontocolisao = *pObj->GetPosition();
				pontocolisao -= *obj->GetPosition();

				pontocolisao.getNormalized(fp);

				fp = -fp;
				fp /= 800;

				w = pObj->GetPosition(true);
				//f = *pObj->GetVelocity();
				TFormPoint(&w, &w, NULL, obj);
				TFormVector(&fp, &fp, NULL, obj);
				//TFormVector(&f, &f, pObj, NULL);
				//TFormVector(&f, &f, NULL, obj);
				//f += obj->GetVelocity();
				f = fp;
				f *= pObj->getMassa();*/
				//Vector dCollision = (*pObj->GetPosition()-*obj->GetPosition())*-0.25;
				//dCollision *= obj->getMassa();
				//obj->Impulse(&w, &f);
				//obj->Translate(&dCollision);
				//pObj->Translate(&dCollision);
			}
		}
		pObj = g_Unidades->GetNext(pObj);
	}
	obj->Turret->Rotate(obj->Turret->pitch(), turretYaw, 0);
	PROFILE_END;
}

void reagirAColisao(CObjeto *obj, CObjeto *collider, const Vector &p, const real penetrationDepth, const Vector &normal) {
	Vector force(0,0,0);
	const real c = 0.16, k = 0.03;
	const Vector pontoRelativo = p-(*obj->GetPosition(true));
	Vector velocity;

	collider->velocityAtPoint(&velocity, &pontoRelativo);

	const real relativeSpeed = -normal.dot(velocity);

	if(relativeSpeed > 0) {
		force += normal * (c * relativeSpeed);
	}

	Vector penaltyForce = normal * (penetrationDepth * k);
	force += penaltyForce;

	force *= collider->getMassa();

	obj->Impulse(&pontoRelativo, &force);
}

void MoveTurret(CObjeto *obj, const Vector *pos) {
	real dpitch, dtyaw;

	if(obj->dados->arma && obj->dados->arma->subtipo == TIPO_PESADO)
		dpitch = DeltaPitch(obj->Turret, pos)-(D3DXToRadian(sqrt(Distance(obj->GetPosition(), pos)))*obj->ai.aimvar);
	else
		dpitch = DeltaPitch(obj->Turret, pos);
	dtyaw = DeltaYaw(obj->Turret, pos);

	CEixo tmp(0, 0);
	tmp.setX(dtyaw*10);
	tmp.setY(dpitch*10);
	/*if(dtyaw < -this->dados->hv)
		tmp.setX(-10);
	else if(dtyaw > this->dados->hv)
		tmp.setX(10);
	if(dpitch < -this->dados->vv)
		tmp.setY(-10);
	else if(dpitch > this->dados->vv) {
		tmp.setY(10);
	}*/
	obj->dados->ctipo->move(obj, &tmp);
}

void IrPara(CObjeto *obj, const Vector *pos, const real distanciasq, const Vector *targetpos) {
	real dpitch, dtyaw, droll;

	if(obj->getDistanceSq(pos) > distanciasq)
		obj->dados->ctipo->OnSpeed(obj);
	else
		obj->dados->ctipo->OnBreak(obj);

	if(obj->dados->arma && obj->dados->arma->subtipo == TIPO_PESADO)
		dpitch = DeltaPitch(obj->Turret, targetpos)-(D3DXToRadian(sqrt(Distance(obj->GetPosition(), targetpos)))*obj->ai.aimvar);
	else
		dpitch = DeltaPitch(obj->Turret, targetpos);
	dtyaw = DeltaYaw(obj->Turret, targetpos);
	droll = DeltaYaw(obj, pos);

	if(droll < -obj->dados->hv)
		obj->dados->ctipo->OnLeft(obj);
	else if(droll > obj->dados->hv)
		obj->dados->ctipo->OnRight(obj);//RollRight(obj);

	CEixo tmp(0, 0);
	tmp.setX(dtyaw*10);
	tmp.setY(dpitch*10);
	/*if(dtyaw < -this->dados->hv)
		tmp.setX(-10);
	else if(dtyaw > this->dados->hv)
		tmp.setX(10);
	if(dpitch < -this->dados->vv)
		tmp.setY(-10);
	else if(dpitch > this->dados->vv) {
		tmp.setY(10);
	}*/
	obj->dados->ctipo->move(obj, &tmp);
}

void TurretMove(CObjeto *obj)
{
	if(obj->getTarget()) {
		if(obj->podeAtacar(obj->getTarget())) {
			CObjeto *alvo = *obj->getTarget();
			Vector midpoint;
			TFormVector(&midpoint, alvo->dados->midpoint, alvo, NULL);
			midpoint += *alvo->GetPosition();

			MoveTurret(obj, &midpoint);

			if((obj->dados->arma && obj->dados->arma->subtipo == TIPO_PESADO) ||
				((fabs(DeltaPitch(obj->Turret, &midpoint)) + fabs(DeltaYaw(obj->Turret, &midpoint)))*0.5f <  obj->dados->AILockOnAngle))
					obj->Atacar();
		} else
			obj->setTarget(NULL);
	} else {
		CObjeto *lider = obj->getLider();
		if(lider && lider->GetTarget() && obj->podeAtacar(lider->getTarget()))
			obj->setTarget(lider->getTarget());
		else
			obj->setTarget(obj->ConsultarObjetoMaisProximo(CONSULTAR_NOVO_ALVO));
	}
}

void CEstrutura::CPUMove(CObjeto *obj)
{
	//return;
	//CObjeto *lider = obj->getLider();

	TurretMove(obj);
	/*if(lider) {
		if(obj->GetTarget() != lider->GetTarget()) {
			obj->alvo = lider->alvo;
		} else if(obj->GetTarget()) {
			IrPara(obj, lider->GetPosition(), lider->dados->Radius*20, obj->GetTarget()->GetPosition()); 
			obj->Atacar();
			return;
		}
	}*/

	switch(obj->ai.state.get())
	{
		case FSM_FIXPOSITION:
			if(obj->getTarget())	{
				CObjeto *mytarget = *obj->getTarget();
				if(mytarget)
					obj->IAIrPara(mytarget->GetPosition());
				if(obj->ai.state.finished()) {
					obj->ai.state.set(FSM_ATTACK, 1000);
				}
			} else {
				obj->PickARandomPlace(200);
			}
			return;
		case FSM_REPAIR:
			if(obj->basealvo->Dona() == obj->team && obj->getLife() < obj->dados->armour) {
				if(obj->basealvo->getDistanceSq(obj->ai.wpt) > obj->basealvo->Dados()->getTipo()->getRaioDeInfluenciaSq())
					obj->ai.wpt.gotoTo(obj->basealvo->GetPosition(), obj->basealvo->Dados()->getTipo()->getRaioDeInfluencia());
				else
					obj->IAIrParaEManterDistancia(&obj->ai.wpt, obj->basealvo->Dados()->getTipo()->getRaioDeInfluencia());					
			} else {
				obj->ai.state.set(FSM_IDLE, 10);
				obj->basealvo = NULL;
			}
			return;
		default:
			if(obj->ai.state.finished()) {
				if(!obj->getLider() && obj->getNLockOns() > 1 && obj->getSquadNum() < obj->getNLockOns() && obj->pergAjuda <= GameTicks)
					PedirAjuda(obj);
			}
			if(obj->ai.state.get() != FSM_FOLLOW && obj->ai.state.get() != FSM_REPAIR && obj->getLider())
				obj->ai.state.set(FSM_FOLLOW, 100);
			if(!obj->dados->getImovel() && obj->getLife() <= obj->dados->armour/2 && obj->ai.state.get() != FSM_REPAIR) {
				obj->basealvo = obj->ConsultarBaseMaisProxima(CONSULTAR_BASE_ALIADA_MAIS_PROXIMA);
				if(obj->basealvo) {
					obj->ai.state.set(FSM_REPAIR, 1);
					obj->ai.wpt.gotoTo(obj->basealvo->GetPosition(), obj->basealvo->Dados()->getTipo()->getRaioDeInfluencia());
					return;
				}
			}
			break;
	}

	obj->DefCPUMove();
}