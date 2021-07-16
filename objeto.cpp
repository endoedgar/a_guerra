#include "stdafx.h"
#include "GameGlobals.h"
#include "objeto.h"
#include "tship.h"
#include "projetil.h"
#include "jogador.h"
#include "radio.h"
#include "3DFunctions.h"
#include "globalheader.h"
#include "CTerrain.h"
#include "missao.h"
#include "Carcaca.h"
#include "MeshColisao.h"
#include "Base.h"

CObjectManager<CObjeto> *g_Unidades = NULL;	// Vector of objects
CSimpleListAP<CObjeto*> *g_UnidadesPtr = NULL;
CObjetoT **g_Objtypes = NULL;

#define WHEELDIST (2)
#define SK (0.012)
#define SD (0.3)

void CObjeto::setDamage(const real damage, CObjeto **shooter) {
	this->setLife(this->getLife()-damage);
	this->damage = damage;
	if(this->life > 0) {
		if(shooter && *shooter) {
			if((*shooter)->j) {
				(*shooter)->j->MostrarTexto("Acertou o inimigo!", 320, D3DCOLOR_XRGB(255, 0, 0));
			}
		}
		//TocarSomRadio(RADIO_DANO, obj);
		if(!this->j) {
			if(!this->GetTarget() || (shooter && *shooter && (*shooter)->team != this->team && this->getDistanceSq(*this->alvo) > this->getDistanceSq(*shooter))) {
				if(shooter && *shooter) {
					this->alvo = shooter;
					this->ai.state.set(FSM_ATTACK, 600);
				}
			}
		} else {
			std::ostringstream sstemp;
			D3DCOLOR cor;
			if(damage > 0) {
				sstemp << "Dano: ";
				cor = D3DCOLOR_XRGB(255, 0, 0);
			} else {
				sstemp << "Reparo: ";
				cor = D3DCOLOR_XRGB(0, 255, 0);
			}
			sstemp << abs(this->damage);
			this->j->MostrarTexto(sstemp.str().c_str(), 320, cor);
		}
	} else if(shooter && *shooter) {
		(*shooter)->ReacaoAoMatarInimigo();
		if((*shooter)->j) {
			(*shooter)->j->mostrarInformacao(bomtiro);
			(*shooter)->j->MostrarTexto("Inimigo destruído!", 320, D3DCOLOR_XRGB(255, 0, 0));
		}
	}
}

void CSuspensao::Update(const Vector &whereshouldbe)
{
	real penetrationDepth;
	Vector normal, final;
	Vector velocity;

	Vector p = *this->GetPosition(true);
	p -= *this->GetParent()->GetPosition();

	this->GetParent()->velocityAtPoint(&velocity, &p);
	//TFormVector(&tmp, this->GetVelocity(), this->GetParent(), NULL);
	velocity += this->GetVelocity();

	

	terrain->GetPointInformation(this->GetPosition(true), penetrationDepth, normal);
	if(penetrationDepth > 0) {
		Vector force;
		normal = -normal;

		const real c = static_cast<real>(0.16), f = terrain->friction, k = static_cast<real>(0.03), b = 0;
		Vector frictionForce;

		force.clear();

		const real relativeSpeed = -normal.dot(velocity);

		if(relativeSpeed > 0) {
			force += normal * (c * relativeSpeed);
		}

		Vector tangentialVelocity = velocity + (normal * relativeSpeed);
		TFormVector(&frictionForce, &tangentialVelocity, NULL, this->GetParent());
		frictionForce *= -f;
		if(!static_cast<CObjeto*>(this->GetParent())->freio)
			frictionForce *= static_cast<const DadosDaUnidadeTerrestre*>(static_cast<CObjeto*>(this->GetParent())->dados)->friccaoVectorial;
		else
			frictionForce *= 2;
		TFormVector(&frictionForce, &frictionForce, this->GetParent(), NULL);
		force += frictionForce;	

		Vector penaltyForce = normal * (penetrationDepth * k);
		force += penaltyForce;

		Vector dampingForce = normal * (relativeSpeed * penetrationDepth * b);	
		force += dampingForce;

		//force.x *= -1;
		//force.z *= -1;

		this->Translate(&force);

		//force = -force;
		force *= this->pParent->getMassa();
		this->pParent->Impulse(&p, &force);

		this->collidedAtFrame(GameTicks);
	}

	// abrams 0.02
	final = ((whereshouldbe-*this->GetPosition())*this->suspensionK)-(this->suspensionD*(*this->GetVelocity()));
		
	//final.y -= 0.07;
	//if(abs(final.y) > 2)
	//	assert(1);
	this->Translate(&final);
	this->UpdatePhysics();
	this->UpdateMatrix();

	//final = -final;
	//final.y += static_cast<real>(0.30);
	//final *= static_cast<real>(0.005);
}


CObjeto::CObjeto()
{
	this->Turret = new Entity;
	if(this->Turret)
		this->Turret->Parent(this);
	for(register unsigned int a = 0; a < 4; ++a) {
		this->side[a] = new CSuspensao;
		if(this->side[a])
			this->side[a]->Parent(this);
	}
	this->perguntas = new CSimpleListAP<Pergunta>;
	TRACE("Objeto %p criado!\n", this);
}

HRESULT CObjeto::Atirar()
{
	if(this->dados->arma && this->nextShoot < GameTicks) {
		//vTemp2;
		//static D3DXQUATERNION qTemp;
		if(this->shootdelay >= this->dados->arma->shootdelay) {
			// TODO: Falar sobre tiro
			CProjetil *pProj = g_Projeteis->New();
			pProj->shooter = this->ptr->GetPtr();
			pProj->atirar();
			this->shootdelay -= this->dados->arma->shootdelay;
			this->nextShoot = GameTicks + this->dados->shootdelay;
		}
	}
	return S_OK;
}

void CObjeto::Atacar()
{
	PROFILE_START;
	if(this->dados->arma) {
		if(
			((this->dados->arma->subtipo == TIPO_MIRV || this->dados->arma->subtipo == TIPO_MISSEL) && this->lockOnTime >= static_cast<CHotSeekingProjectileData*>(this->dados->arma)->lockOnTime) ||
			(this->dados->arma->subtipo != TIPO_MIRV && this->dados->arma->subtipo != TIPO_MISSEL)
		  )
		this->Atirar();
	}
	PROFILE_END;
}

void CObjeto::IAAtacar() {
	this->alvo = this->ConsultarObjetoMaisProximo(CONSULTAR_NOVO_ALVO);
	if(this->alvo)
	{
		ObjetoLiderMandouAtacar(this);
		this->ai.state.set(FSM_ATTACK, 1000);
	} else if(this->pergAjuda <= GameTicks) {
		ObjetoPerguntarSobrePosicaoInimiga(this);
		this->PickARandomPlace(1000);
	}
}

void CObjeto::DefCPUMove()
{
	switch(this->ai.state.get())
	{
		case FSM_IDLE:
			if(!this->ai.state.finished())
			{
				this->dados->ctipo->OnBreak(this);
				if(this->getLider() && !this->GetTarget() && this->getLider()->GetTarget()) {
					this->alvo = this->getLider()->alvo;
					this->ai.state.set(FSM_ATTACK, 1000);
				}
				
				if(!this->basealvo && !this->dados->getImovel() && this->dados->tipo == TIPO_VEICULO)
					this->basealvo = this->ConsultarBaseMaisProxima(CONSULTAR_BASE_INIMIGA);
				CObjeto *alvo = this->GetTarget();
				if(!alvo) {
					this->alvo = this->ConsultarObjetoMaisProximo(CONSULTAR_NOVO_ALVO);
					alvo = this->GetTarget();
				}

				if(this->dados->tipo == TIPO_VEICULO && this->basealvo && this->basealvo->Dona() != this->team && (!alvo || (alvo && this->getDistanceSq(this->basealvo) < this->getDistanceSq(alvo))))
					this->ai.state.set(FSM_CAPTURE, 2000);
				else if(alvo) {
					if(this->getSquadNum())
						ObjetoLiderMandouAtacar(this);
					this->ai.state.set(FSM_ATTACK, 1000);
				}
			} else {
				if(this->dados->tipo == TIPO_VEICULO && !this->dados->getImovel()) {
					if(!this->basealvo || this->basealvo->Dona() != this->team) {
						this->basealvo = this->ConsultarBaseMaisProxima(CONSULTAR_BASE_ALIADA_MAIS_AMEACADA);
						PickARandomPlace(200);
					} else
						this->ai.state.set(FSM_GUARD, 3600);
				} else
					PickARandomPlace(200);
			}
			break;
		case FSM_GUARD:
			if(this->basealvo) {
				if(this->getDistanceSq(this->basealvo) >= this->basealvo->Dados()->getTipo()->getRaioDeInfluencia())
					this->IAIrPara(this->basealvo->GetPosition());
				else if(this->alvo && *this->alvo) {
					this->basealvo->Defender();
					CObjeto *mytarget = *this->alvo;
					this->dados->ctipo->OnBreak(this);
					if(mytarget) {
						Vector midpoint;
						TFormVector(&midpoint, mytarget->dados->midpoint, mytarget, NULL);
						midpoint += *mytarget->GetPosition();
						if(this->dados->tipo == TIPO_VEICULO) {
							if((this->dados->arma && this->dados->arma->subtipo == TIPO_PESADO) ||
							((fabs(DeltaPitch(this->Turret, &midpoint)) + fabs(DeltaYaw(this->Turret, &midpoint)))*0.5f <  this->dados->AILockOnAngle))
								this->Atacar();
						} else {
							const real pitch = fabs(DeltaPitch(this, &midpoint));
							const real yaw = fabs(DeltaYaw(this, &midpoint));
							const real res = (pitch+yaw)*r(0.5);
							if(res <  this->dados->AILockOnAngle)
								this->Atacar();
						}
		
						this->IAVirarPara(&midpoint);
						if(!this->noRadar(mytarget))
						{
							TocarSomRadio(RADIO_INIMIGO_FUGIU, this);
							if(!this->getLider()) {
								this->alvo = NULL;
							}
						}
					} else
						this->alvo = NULL;
				}
				if(!this->alvo)
					this->alvo = this->ConsultarObjetoMaisProximo(CONSULTAR_NOVO_ALVO);
				if(this->ai.state.finished() || (this->basealvo->Dona() != this->team)) {
					this->basealvo = NULL;
					if(!this->getLider()) {
						this->PickARandomPlace(1000);
					} else {
						this->ai.state.set(FSM_FOLLOW, 1000);
					}
				}
			} else {
				this->PickARandomPlace(1000);
				this->basealvo = NULL;
			}

			break;
		case FSM_ATTACK:
		  	PROFILE_START;
			if(this->alvo)
			{
				CObjeto *mytarget = *this->alvo;
				if(mytarget)
				{
					Vector midpoint;
					TFormVector(&midpoint, mytarget->dados->midpoint, mytarget, NULL);
					midpoint += *mytarget->GetPosition();
					if(this->dados->tipo == TIPO_VEICULO) {
						if((this->dados->arma && this->dados->arma->subtipo == TIPO_PESADO && fabs(DeltaYaw(this->Turret, &midpoint)) <  this->dados->AILockOnAngle) ||
						((fabs(DeltaPitch(this->Turret, &midpoint)) + fabs(DeltaYaw(this->Turret, &midpoint)))*0.5f <  this->dados->AILockOnAngle))
							this->Atacar();
					} else {
						const real pitch = fabs(DeltaPitch(this, &midpoint));
						const real yaw = fabs(DeltaYaw(this, &midpoint));
						const real res = (pitch+yaw)*r(0.5);
						if(res <  this->dados->AILockOnAngle)
							this->Atacar();
					}
					this->IAIrPara(&midpoint);
	
					if(!this->noRadar(mytarget))
					{
						TocarSomRadio(RADIO_INIMIGO_FUGIU, this, -2);
						if(!this->getLider()) {
							this->PickARandomPlace(1000);
							this->alvo = NULL;
						} else {
							this->ai.state.set(FSM_FOLLOW, 1000);
							this->alvo = NULL;
						}
					}
					else if(this->ai.state.finished()) {
						if(!this->getLider()) {
							this->PickARandomPlace(1000);
							this->alvo = NULL;
						} else {
							this->ai.state.set(FSM_FOLLOW, 1000);
							this->alvo = NULL;
						}
					} 
				}
			}
			else
			{
				this->PickARandomPlace(1000);
				this->alvo = NULL;
			}
			PROFILE_END;
			break;
		case FSM_FOLLOW:
			if(this->getLider()) {
				CObjeto *lider = this->getLider();
				if(lider->getLider() == this) {
					this->setLider(NULL);
					this->PickARandomPlace(1000);
					break;
				}
				if((lider->ai.state.get() == FSM_ATTACK && this->ai.state.timeToFinish() < 300) || (lider->j && (*lider->j->GetControl())(INPUT_ATIRAR) && lider->GetTarget())) {
					this->alvo = lider->alvo;
					this->ai.state.set(FSM_ATTACK, 1000);
				}
				if(lider->ai.state.get() == FSM_CAPTURE && !this->dados->getImovel()) {
					this->basealvo = lider->basealvo;
					this->ai.state.set(FSM_CAPTURE, lider->ai.state.getTime());
				}
				if(lider->ai.state.get() == FSM_GUARD) {
					this->basealvo = lider->basealvo;
					this->ai.state.set(FSM_GUARD, lider->ai.state.getTime());
				}
				this->IAIrParaEManterDistancia(lider->GetPosition(), (this->dados->getRadius()+lider->dados->getRadius())*5);
			} else {
				ObjetoFalarSobrePerdaDeLider(this);
				this->ai.state.set(FSM_IDLE, 25);
				this->ai.setTimeout(0);
			}
			break;
		case FSM_CAPTURE:
			if(this->basealvo && !this->dados->getImovel()) {
				if(this->getDistanceSq(this->basealvo) >= this->basealvo->Dados()->getTipo()->getRaioDeInfluenciaSq())
					this->IAIrPara(this->basealvo->GetPosition());
				else if(this->alvo && *this->alvo) {
					CObjeto *mytarget = *this->alvo;
					this->dados->ctipo->OnBreak(this);
					if(mytarget) {
						Vector midpoint;
						TFormVector(&midpoint, mytarget->dados->midpoint, mytarget, NULL);
						midpoint += *mytarget->GetPosition();
						if(this->dados->tipo == TIPO_VEICULO) {
							if((this->dados->arma && this->dados->arma->subtipo == TIPO_PESADO) ||
							((fabs(DeltaPitch(this->Turret, &midpoint)) + fabs(DeltaYaw(this->Turret, &midpoint)))*0.5f <  this->dados->AILockOnAngle))
								this->Atacar();
						} else {
							const real pitch = fabs(DeltaPitch(this, &midpoint));
							const real yaw = fabs(DeltaYaw(this, &midpoint));
							const real res = (pitch+yaw)*r(0.5);
							if(res <  this->dados->AILockOnAngle)
								this->Atacar();
						}
		
						this->IAVirarPara(&midpoint);
						if(!this->noRadar(mytarget))
						{
							TocarSomRadio(RADIO_INIMIGO_FUGIU, this, -2);
							if(!this->getLider()) {
								this->alvo = NULL;
							}
						}
					} else
						this->alvo = NULL;
				}
				if(!this->alvo)
					this->alvo = this->ConsultarObjetoMaisProximo(CONSULTAR_NOVO_ALVO);
				if(this->ai.state.finished() || (this->basealvo->Dona() == this->team && this->basealvo->Influencia() >= 1.0) || this->dados->getImovel()) {
					this->basealvo = NULL;
					if(!this->getLider()) {
						this->PickARandomPlace(1000);
					} else {
						this->ai.state.set(FSM_FOLLOW, 1000);
					}
				}
			} else {
				this->PickARandomPlace(1000);
				this->basealvo = NULL;
			}

			break;
		case FSM_PATROL:
			if(!this->ai.wpt.reachedSq(this->GetPosition(),this->dados->getRadiusSq()) && !this->ai.state.finished() && !this->dados->getImovel())
				this->IAIrPara(&this->ai.wpt);
			else {
				if(!this->basealvo) {
					this->basealvo = this->ConsultarBaseMaisProxima(CONSULTAR_BASE_INIMIGA);
				} else if(this->basealvo->Dona() == this->team)
					this->basealvo = NULL;

				if(!(this->dados->tipo == TIPO_AERONAVE && this->GetPositionY() < static_cast<const DadosDaUnidadeVoadora*>(this->dados)->getAISafeAlt())) {
					this->alvo = this->ConsultarObjetoMaisProximo(CONSULTAR_NOVO_ALVO);
					if(this->alvo) {
						if(this->basealvo && *this->alvo && this->dados->tipo == TIPO_VEICULO)
							if(this->getDistanceSq(this->basealvo) < this->getDistanceSq(*this->alvo)) {
								this->ai.state.set(FSM_CAPTURE, 3600);
								break;
							}
						ObjetoLiderMandouAtacar(this);
						this->ai.state.set(FSM_ATTACK, 1000);
						//if(obj->ai.ehlider)
						//	ObjetoPerguntarSobreAtaque(Obj)
					} else if(this->basealvo) {
						this->ai.wpt.gotoTo(this->basealvo->GetPosition());
					}
				}
				this->ai.state.set(FSM_IDLE, random(300));
			}
			break;
		case FSM_SEARCH:
			if(this->ai.state.timeToFinish() % 10 == 0) {
				this->alvo = this->ConsultarObjetoMaisProximo(CONSULTAR_NOVO_ALVO);
				if(this->alvo) {
					ObjetoLiderMandouAtacar(this);
					this->ai.state.set(FSM_ATTACK, 1000);
					//if(obj->ai.ehlider)
					//	ObjetoPerguntarSobreAtaque(Obj)
				}
			}
			if(!this->ai.wpt.reachedSq(this->GetPosition(), this->dados->getRadiusSq()*2) && !this->ai.state.finished()) 
				this->IAIrPara(&this->ai.wpt);
			else {
				this->ai.state.set(FSM_IDLE, 25);
			}
			break;
		default:
			this->ai.reset();
			break;
	}
}

void CObjeto::Criar(const Vector *position, const int controle, const CEquipe *team, const DadosDaUnidade *data)
{
	this->freio = false;
	this->team = team;
	this->esquadrao = rand()%10;
	this->dados = data;
	this->basealvo = NULL;
	this->alvo = NULL;
	this->nLockOns = 0;
	this->pergAjuda = 0;
	this->lockOnTime = 0;
	this->setCOM(this->dados->getCOM());

	this->mshCol = this->dados->meshCol;
	
	this->nextShoot = 0;
	this->SetupPhysics();
	this->ai.reset();
	ObjetoLiderado::ObjetoLiderado();
	this->ai.aimvar = r(0.22);
	this->Position(position);
	this->life = this->dados->armour;
	this->speed = 0;
	this->id = 30;
	this->morto = false;
	this->pontuacao = 0;
	this->alvo = NULL;
	this->j = NULL;
	this->y_vel = 0;
	this->ptr = g_UnidadesPtr->New();
	this->ptr->SetPtr(this);
	ASSERT(this->ptr);
	this->voz = r(r(0.8)+randomf(r(0.4)));
	this->setMassa(this->dados->getMassa());
	this->setInertiaTensor(this->dados->getInertiaTensor());

	if(this->dados->tipo != TIPO_AERONAVE) {
		for(register unsigned int a = 0; a < 4; ++a) {
			this->side[a]->SetupPhysics();
			this->side[a]->Position(&this->dados->p[a]);
			this->side[a]->UpdateMatrix();
			this->side[a]->setVelocityDamping(0, 0, 0);
			this->side[a]->setSuspensionD(this->dados->suspensionD);
			this->side[a]->setSuspensionK(this->dados->suspensionK);
			this->side[a]->setVelocityDamping(0.8, 0.8, 0.8);
		}
	}
	if(this->dados->tipo == TIPO_VEICULO) {
		this->Turret->SetupPhysics();
		this->Turret->Position(&static_cast<const DadosDaUnidadeTerrestre*>(this->dados)->turretpos);
		this->Turret->UpdateMatrix();

		for(register unsigned int a = 0; a < 4; ++a) {
			this->suspensao[a].setSuspensionD(SD);
			this->suspensao[a].setSuspensionK(SK);
			this->suspensao[a].Parent(this);
			this->suspensao[a].SetupPhysics();
			this->suspensao[a].setVelocityDamping(1, 1, 1);
		}
		this->suspensao[0].Position(&this->dados->meshCol.getVertice(0));
		this->suspensao[1].Position(&this->dados->meshCol.getVertice(1));
		this->suspensao[2].Position(&this->dados->meshCol.getVertice(4));
		this->suspensao[3].Position(&this->dados->meshCol.getVertice(5));
		for(register unsigned int a = 0; a < 4; ++a) {
			Vector vec = this->suspensao[a].GetPosition();
			vec.y -= this->dados->suspLength;
			this->suspensao[a].Position(&vec);
			this->suspensao[a].UpdatePhysics();
			this->suspensao[a].UpdateMatrix();
		}
	}
	this->setAngularDamping(real(0.1));
	if(this->dados->tipo == TIPO_HELICOPTERO || this->dados->tipo == TIPO_VEICULO) {
		this->setVelocityDamping(1, 1, 1);
		this->setAngularDamping(static_cast<real>(0.001));
		if(this->dados->getImovel()) {
			this->setAngularDamping(static_cast<real>(0.1));
		}
	}
	if(this->dados->tipo == TIPO_HELICOPTERO)
		this->setAngularDamping(static_cast<real>(0.1));
	if(this->dados->arma)
		this->shootdelay = this->dados->arma->shootdelay*this->dados->stock;
	//this->Rotate(D3DXToRadian(135), 0, 0);
	//this->Rotate(0, 0, 0);
}

void CObjeto::Explodir()
{
	Carcaca *tmp;
	TocarSomRadio(RADIO_MORREU, this, -1);
	this->setTarget(NULL);
	if(this->dados->ExplodDestruido)
		ps->CriarExplod(this->GetPosition(), this->dados->ExplodDestruido);
	if(this->getLider()) {
		if(this->getLider()->j) {
			this->getLider()->j->mostrarInformacao(menosesquadraoimg);
		}
		this->setLider(NULL);
	}
	if(this->ptr) {
		this->ptr->SetPtr(static_cast<CObjeto*>(NULL));
		g_UnidadesPtr->Delete(&this->ptr);
	}
	if(this->ogm)
		--(*this->ogm);
	tmp = g_Carcacas->New()->GetPtr();
	if(this->dados->tipo == TIPO_VEICULO)
		tmp->setMesh(static_cast<const DadosDaUnidadeTerrestre*>(this->dados)->Turret);
	else
		tmp->setMesh(this->dados->Mesh);
	tmp->SetupPhysics();
	tmp->setAccelerationDamping(static_cast<real>(0.99), 1, static_cast<real>(0.99));
	tmp->setTorqueDamping(0);
	tmp->setAngularDamping(1);
	const real fator = D3DX_PI/25;
	tmp->Turn(-(fator/2)+randomf(fator), -(fator/2)+randomf(fator), -(fator/2)+randomf(fator));
	tmp->Rotate(this->GetQuaternion());
	tmp->Position(this->GetPosition());
	tmp->Move(-1+randomf(2), 2, -1+randomf(2));
	tmp->setLifetime(600);
	tmp->setGravity(true);
	tmp->setCor(0x00000000);
	if(this->dados->ExplodCarcacaVoando)
		ps->CriarExplod(&Vector::zero(), this->dados->ExplodCarcacaVoando, NULL, tmp);
	if(this->dados->tipo == TIPO_VEICULO) {
		if(this->dados->ExplodFumacaCarcaca)
			ps->CriarExplod(this->GetPosition(), this->dados->ExplodFumacaCarcaca);
		tmp = g_Carcacas->New()->GetPtr();
		tmp->SetupPhysics();
		tmp->setCor(0x00000000);
		tmp->setMesh(this->dados->Mesh);
		tmp->Rotate(this->GetQuaternion());
		tmp->Position(this->GetPosition());
		tmp->setLifetime(600);
		tmp->setGravity(false);
	}

	for(register unsigned int a = 0; a < this->dados->qCarcaca; ++a) {
		tmp = g_Carcacas->New()->GetPtr();
		tmp->SetupPhysics();
		tmp->setCor(0x00000000);
		tmp->setMesh(NULL);
		tmp->setLifetime(600);
		tmp->Position(this->GetPosition());
		tmp->Rotate(&gIdentityQuaternion);
		tmp->setAccelerationDamping(static_cast<real>(0.99), 1, static_cast<real>(0.99));
		tmp->setTorqueDamping(0);
		tmp->setAngularDamping(1);
		if(this->dados->tipo == TIPO_VEICULO)
			tmp->Move(-4+randomf(8), 2+randomf(2), -4+randomf(8));
		else
			tmp->Move(-3+randomf(6), -3+randomf(6), -3+randomf(6));
		tmp->Turn(-(fator/2)+randomf(fator), -(fator/2)+randomf(fator), -(fator/2)+randomf(fator));
		if(this->dados->ExplodCarcacaVoando)
			ps->CriarExplod(&Vector::zero(), this->dados->ExplodCarcacaVoando, NULL, tmp);
		tmp->setGravity(true);

	}
	this->ptr = NULL;
	casualities += this->dados->crew;
}

CBase *CObjeto::ConsultarBaseMaisProxima(const CONSULTA_BASE tipo)
{
	CBase *minbase = NULL, *proxima;
	real mindist = this->dados->getRadarSq(), dist;

	switch(tipo) {
		case CONSULTAR_BASE_INIMIGA:
			for(register size_t i = 0; i < mymission->GetBases().GetCapacity(); ++i) {
				proxima = &mymission->GetBases()[i];
				if(proxima->Dona() != this->team) {
					dist = this->getDistanceSq(proxima);
					if(dist < mindist || proxima->Dados()->getFaccao() == this->team)
					{
						mindist = dist;
						minbase = proxima;
					}
				}
			}
			break;
		case CONSULTAR_BASE_ALIADA_MAIS_PROXIMA:
			for(register size_t i = 0; i < mymission->GetBases().GetCapacity(); ++i) {
				proxima = &mymission->GetBases()[i];
				if(proxima->Dona() == this->team) {
					dist = this->getDistanceSq(proxima);
					if(dist < mindist)
					{
						mindist = dist;
						minbase = proxima;
					}
				}
			}
			break;
		case CONSULTAR_BASE_ALIADA_MAIS_AMEACADA: {
			int MaiorAmeaca = 0;
			int MaiorNecessidade = 0;
			for(register size_t i = 0; i < mymission->GetBases().GetCapacity(); ++i) {
				proxima = &mymission->GetBases()[i];
				if(proxima->Dona() == this->team) {
					if(proxima->Ameaca() > MaiorAmeaca && proxima->NecessidadeDeDefesas() > MaiorNecessidade)
					{
						MaiorNecessidade = proxima->NecessidadeDeDefesas();
						MaiorAmeaca = proxima->Ameaca();
						minbase = proxima;
					}
				}
			}
			break;
		}
	}
	return minbase;
}

CObjeto **CObjeto::ConsultarObjetoMaisProximo(const CONSULTA_OBJ tipo)
{
	CObjeto **minobj = NULL, *proximo;//, *defato;
	real mindist = this->dados->getRadarSq(), dist;
	PROFILE_START;
	switch(tipo)
	{
		case CONSULTAR_NOVO_ALVO:
			proximo = g_Unidades->GetFirstActive();
			while(proximo)
			{
				if(proximo->team != this->team && proximo->IsValid() && this->podeAtacar(&proximo))
				{
					dist = this->getDistanceSq(proximo);
					if(dist < mindist)
					{
						mindist = dist;
						minobj = proximo->ptr->GetPtr();
					}
				}
				proximo = g_Unidades->GetNext(proximo);
			}
			break;
		case CONSULTAR_NOVO_LIDER:
			mindist = -1;
			proximo = g_Unidades->GetFirstActive();
			while(proximo)
			{
				if(proximo->IsValid() && proximo->podeLiderar(this->ptr->GetPtr())) {
					dist = this->getDistanceSq(proximo);
					if(mindist == -1 || dist < mindist) {
						mindist = dist;
						minobj = proximo->ptr->GetPtr();
					}
				}
				proximo = g_Unidades->GetNext(proximo);
			}
			break;
	}
	PROFILE_END;
	return minobj;
}

bool CObjeto::podeLiderar(CObjeto **obj) const {
		if(obj) {
			if(*obj) {
				CObjeto *defato = *obj;
				if(this->team == defato->team && defato != this && this->noRadar(defato))
					if(defato->dados->getRadar() >= this->dados->getRadar() && defato->IsValid() && !defato->getLider())
						if((this->dados->getImovel() && defato->dados->getImovel()) || (!this->dados->getImovel() && !defato->dados->getImovel()))
							if(defato->dados->arma && defato->dados->tipo == this->dados->tipo)
								return true;
			}
		}
		return false;
	}

void CObjeto::PickARandomPlace(const real radius)
{
	PROFILE_START;
	if(!this->dados->getImovel()) {
		CBase *base = this->ConsultarBaseMaisProxima(CONSULTAR_BASE_ALIADA_MAIS_PROXIMA);
		this->ai.state.set(FSM_PATROL, 300+random(100));
		Vector tmp;
		if(base) {
			tmp.x = base->GetPosition()->x-radius+static_cast<real>(random(static_cast<unsigned int>(radius*2)));
			tmp.z = base->GetPosition()->z-radius+static_cast<real>(random(static_cast<unsigned int>(radius*2)));
		} else {
			tmp.x = randomf(terrain->GetWidth()*terrain->GetScale()->x);//this->GetPositionX()-random(radius)+random(radius);
			tmp.z = randomf(terrain->GetDepth()*terrain->GetScale()->z);//this->GetPositionZ()-random(radius)+random(radius);
		}

		this->ai.wpt.gotoTo(&tmp);

		if(this->dados->tipo == TIPO_VEICULO)
			this->ai.wpt.y = terrain->GetTerrainHeight(&tmp);
		else
			this->ai.wpt.y = terrain->GetTerrainHeight(&tmp)+(static_cast<const DadosDaUnidadeVoadora*>(this->dados)->getAISafeAlt()*2)+randomf(radius);
	} else
		this->ai.state.set(FSM_IDLE, 300+random(100));
	PROFILE_END;
}

void CObjeto::falarPara(CObjeto *receptor, PerguntaBot id, unsigned int expira, unsigned int responder) const {
	Pergunta *p;
	if(receptor) {
		p = receptor->perguntas->New()->GetPtr();
	} else {
		p = Perguntas->New()->GetPtr();
	}
	p->expira = static_cast<unsigned int>(GetTime()+expira/1000);
	p->responder = static_cast<unsigned int>(GetTime()+responder/1000);
	p->asker = this->ptr->GetPtr();
	p->id = id;
}

void CObjeto::ResponderPerguntas()
{
	PROFILE_START;
	CSimpleNode<Pergunta> *node = this->perguntas->GetFirstActive();
	float mili;

	while(node) {
		Pergunta *p = node->GetPtr();
		if(!p->asker)
			this->perguntas->Delete(&node);
		else {
			if(!(*p->asker))
				this->perguntas->Delete(&node);
			else {
				mili = GetTime();
				if(p->expira <= mili)
					this->perguntas->Delete(&node);
				else if(p->responder <= mili) {
					switch(p->id) {
						//case PERGUNTA_STATUS:
						//	break;
						//case PERGUNTA_PROVOCACAO:
						//	break;
						case PERGUNTA_RESPOSTA_SOBRE_INIMIGOS:
							if(!this->alvo) {
								if((*p->asker)->alvo) {
									if((*(*p->asker)->alvo)) {
										ObjetoAgradecer(this, *p->asker);
										this->ai.state.set(FSM_SEARCH, static_cast<unsigned int>(Distance(this->GetPosition(), (*p->asker)->GetPosition())));
										this->ai.wpt.gotoTo((*(*p->asker)->alvo)->GetPosition());
									}
								}
							}
							this->perguntas->Delete(&node);
							break;
						case PERGUNTA_IMPRESSIONAR_LIDER:
							if((*p->asker)->getLider() == this)
								TocarSomRadio(RADIO_LIDER_PUXANDO_SACO, this, 5);
							this->perguntas->Delete(&node);
							break;
						default:
							node = node->GetNext();
							break;
					}
				} else
					node = node->GetNext();
			}
		}
	}

	node = Perguntas->GetFirstActive();
	while(node) {
		Pergunta *p = node->GetPtr();
		if(!p->asker)
			Perguntas->Delete(&node);
		else {
			if(!(*p->asker))
				Perguntas->Delete(&node);
			else {
				mili = GetTime();
				if(p->expira <= mili)
					Perguntas->Delete(&node);
				else if(p->responder <= mili) {
					if((*p->asker)->team == this->team) {
						switch(p->id)
						{
							case PERGUNTA_AJUDANTE:
								if(!this->getLider() && this->dados->arma && this->IsValid()) {
									if((*p->asker)->podeLiderar(this->ptr->GetPtr())) {
										ObjetoResponderPedidoDeAjuda(this, *p->asker);
										Perguntas->Delete(&node);
										(*p->asker)->pergAjuda = 0;
										break;
									}
								}
								node = node->GetNext();
								break;
							case PERGUNTA_SOBRE_INIMIGOS:
							{
								if(this->alvo && this != *p->asker) {
									if(*this->alvo) {
										ObjetoResponderSobrePosicaoInimiga(this, *p->asker);
										Perguntas->Delete(&node);
										break;
									}
								}
								node = node->GetNext();
								break;
							}
							case PERGUNTA_ATACAR:
								if(this->getLider() == *p->asker) {
									if(this->alvo != this->getLider()->alvo)
										ObjetoResponderSobreAtaque(this);
								}
								node = node->GetNext();
								break;
							//case PERGUNTA_FORMACAO:
							//	break;
							//case PERGUNTA_SEPARAR:
							//	break;
							case PERGUNTA_IMPRESSIONAR_GRUPO:
								if(this->getLiderPtr() == p->asker) {
									TocarSomRadio(RADIO_PUXANDO_SACO_DO_LIDER, this, 5);
									Perguntas->Delete(&node);
									break;
								}
								node = node->GetNext();
								break;
							default:
								node = node->GetNext();
								break;
						}
					} else 
						node = node->GetNext();
				} else
					node = node->GetNext();
			}
		}
	}
	PROFILE_END;
}

void CObjeto::PlayerMove()
{
	CControle *catual = this->j->GetControl();
	this->dados->ctipo->move(this, &catual->v.getEixo());

	if((*catual)(INPUT_ESQUERDA))
		this->dados->ctipo->OnLeft(this);
	if((*catual)(INPUT_DIREITA))
		this->dados->ctipo->OnRight(this);
	if((*catual)(INPUT_ACELERAR))
		this->dados->ctipo->OnSpeed(this);
	if((*catual)(INPUT_DESACELERAR))
		this->dados->ctipo->OnBreak(this);
	if((*catual)(INPUT_ATIRAR))
		this->Atirar();
	// TODO: INPUT_STEALTH
}

void CObjeto::IAVirarPara(const Vector *pos)
{
	real dpitch, droll;
	if(this->dados->tipo == TIPO_AERONAVE) {
		dpitch = DeltaPitch(this, pos);
		droll = DeltaYaw(g_piv, pos);

		if(droll < -this->dados->hv)
			this->dados->ctipo->OnRight(this);
		else if(droll > this->dados->hv)
			this->dados->ctipo->OnLeft(this);//RollRight(obj);

		CEixo tmp(0, 0);
		if(dpitch < -this->dados->vv)
			tmp.setY(1);
		else if(dpitch > this->dados->vv) {
			tmp.setY(-1);
		}
		this->dados->ctipo->move(this, &tmp);
	} else {
		real dist, dtyaw;

		if(this->dados->AIFiringRange == 0 && this->dados->arma) {
			dist = this->dados->getRadar();
		} else
			dist = static_cast<real>(this->dados->AIFiringRange);

		if(this->dados->arma && this->dados->arma->subtipo == TIPO_PESADO)
			dpitch = DeltaPitch(this->Turret, pos)-(D3DXToRadian(sqrt(Distance(this->GetPosition(), pos)))*this->ai.aimvar);
		else
			dpitch = DeltaPitch(this->Turret, pos);
		dtyaw = DeltaYaw(this->Turret, pos);
		droll = DeltaYaw(this, pos);

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
		this->dados->ctipo->move(this, &tmp);
	}
}

void CObjeto::IAIrParaEManterDistancia(const Vector *pos, const real distancia)
{
	real dpitch, droll;
	const real distanciaSq = distancia * distancia;
	PROFILE_START;

	g_piv->Position(this->GetPosition());
	g_piv->Rotate(this->GetQuaternion());
	g_piv->Turn(D3DXToRadian(-90), 0, 0);

	switch(this->dados->tipo) {
		case TIPO_AERONAVE: {
			dpitch = DeltaPitch(this, pos);
			droll = DeltaYaw(g_piv, pos);

			if(droll < -this->dados->hv)
				this->dados->ctipo->OnRight(this);
			else if(droll > this->dados->hv)
				this->dados->ctipo->OnLeft(this);//RollRight(obj);

			CEixo tmp(0, 0);
			if(dpitch < -this->dados->vv)
				tmp.setY(1);
			else if(dpitch > this->dados->vv) {
				tmp.setY(-1);
			}
			this->dados->ctipo->move(this, &tmp);

			if(fabs((DeltaPitch(this, pos)) + fabs(DeltaYaw(this, pos)))/2 < D3DXToRadian(20))
				this->dados->ctipo->OnSpeed(this);//Acelerar(obj);
			else
				this->dados->ctipo->OnBreak(this);//Desacelerar(obj);
		}
		break;
		case TIPO_HELICOPTERO: {
			IAIrPara(pos);
			break;
		}
		default: {
			if(this->dados->arma) {
				if(this->ai.state.get() == FSM_ATTACK) {
					if(this->getDistanceSq(pos) > distanciaSq)
						this->dados->ctipo->OnSpeed(this);
					else
						this->dados->ctipo->OnBreak(this);
				} else {
					if(this->getDistanceSq(pos) > distanciaSq)
						this->dados->ctipo->OnSpeed(this);
					else
						this->dados->ctipo->OnBreak(this);
				}
			}

			//if(this->dados->arma && this->dados->arma->subtipo == TIPO_PESADO)
			//	dpitch = DeltaPitch(this->Turret, pos)-(D3DXToRadian(sqrt(Distance(this->GetPosition(), pos)))*this->ai.aimvar);
			//else
			//	dpitch = DeltaPitch(this->Turret, pos);
			//dtyaw = DeltaYaw(this->Turret, pos);
			droll = DeltaYaw(this, pos);

			if(droll < -this->dados->hv)
				this->dados->ctipo->OnLeft(this);
			else if(droll > this->dados->hv)
				this->dados->ctipo->OnRight(this);//RollRight(obj);

			//CEixo tmp(0, 0);
			//tmp.setX(dtyaw*10);
			//tmp.setY(dpitch*10);
			/*if(dtyaw < -this->dados->hv)
				tmp.setX(-10);
			else if(dtyaw > this->dados->hv)
				tmp.setX(10);
			if(dpitch < -this->dados->vv)
				tmp.setY(-10);
			else if(dpitch > this->dados->vv) {
				tmp.setY(10);
			}*/
			//this->dados->ctipo->move(this, &tmp);
		}
	}
	PROFILE_END;
}

void CObjeto::IAIrPara(const Vector *pos)
{
	real dpitch, droll;
	PROFILE_START;

	g_piv->Position(this->GetPosition());
	g_piv->Rotate(this->GetQuaternion());
	g_piv->Turn(D3DXToRadian(-90), 0, 0);

	switch(this->dados->tipo) {
		case TIPO_AERONAVE: {
			dpitch = DeltaPitch(this, pos);
			droll = DeltaYaw(g_piv, pos);

			if(droll < -this->dados->hv)
				this->dados->ctipo->OnRight(this);
			else if(droll > this->dados->hv)
				this->dados->ctipo->OnLeft(this);//RollRight(obj);

			CEixo tmp(0, 0);
			if(dpitch < -this->dados->vv)
				tmp.setY(1);
			else if(dpitch > this->dados->vv) {
				tmp.setY(-1);
			}
			this->dados->ctipo->move(this, &tmp);

			if(fabs((DeltaPitch(this, pos)) + fabs(DeltaYaw(this, pos)))/2 < D3DXToRadian(20))
				this->dados->ctipo->OnSpeed(this);//Acelerar(obj);
			else
				this->dados->ctipo->OnBreak(this);//Desacelerar(obj);
			break;
		}
		case TIPO_HELICOPTERO: {
			real dyaw = DeltaYaw(this, pos);
			Vector up(this->GetPositionX(), this->GetPositionY()+1, this->GetPositionZ());
			const Vector dif(*pos-*this->GetPosition()); 
			Vector orientation;

			droll = DeltaYaw(g_piv, &up);
			
			TFormPoint(&orientation, pos, NULL, this);
			real s = sqrt(r(this->dados->AIFiringRange));

			if(this->getDistanceSq(pos) >= this->dados->AIFiringRange*0.5 || (orientation.z > 0 || orientation.z < -s*0.5)) {
				if(dyaw < -this->dados->hv)
					this->dados->ctipo->OnLeft(this);
				else if(dyaw > this->dados->hv)
					this->dados->ctipo->OnRight(this);
			}

			if(equal(orientation.z, 0))
				dpitch = 0;
			else if(orientation.z > 0)
				dpitch = D3DXToRadian(35)-this->pitch();
			else
				dpitch = -D3DXToRadian(35)-this->pitch();

			CEixo tmp(0, 0);
			if(dpitch < -this->dados->vv)
				tmp.setY(1);
			else if(dpitch > this->dados->vv)
				tmp.setY(-1);
			if(droll < -this->dados->hv)
				tmp.setX(1);
			else if(droll > this->dados->hv)
				tmp.setX(-1);
			this->dados->ctipo->move(this, &tmp);

			if(orientation.z >= -s && this->GetPositionY() < terrain->GetTerrainHeight(this->GetPosition()) + static_cast<const DadosDaUnidadeVoadora*>(this->dados)->getAISafeAlt() && dif.y > -300)
				this->dados->ctipo->OnSpeed(this);
			else
				this->dados->ctipo->OnBreak(this);

			/*if(fabs((dpitch) + fabs(DeltaYaw(this, pos)))/2 < D3DXToRadian(20) && this->GetPositionY() <= 600)
				this->dados->ctipo->OnSpeed(this);
			else
				this->dados->ctipo->OnBreak(this);*/
			break;
		}
		default: {
			real dist;

			if(this->dados->AIFiringRange == 0 && this->dados->arma) {
				dist = this->dados->getRadarSq();
			} else
				dist = static_cast<real>(this->dados->AIFiringRange);

			if(this->dados->arma) {
				if(this->ai.state.get() == FSM_ATTACK) {
					if(this->getDistanceSq(pos) > dist)
						this->dados->ctipo->OnSpeed(this);
					else
						this->dados->ctipo->OnBreak(this);
				} else {
					if(this->getDistanceSq(pos) > this->dados->getRadiusSq())
						this->dados->ctipo->OnSpeed(this);
					else
						this->dados->ctipo->OnBreak(this);
				}
			}

			//if(this->dados->arma && this->dados->arma->subtipo == TIPO_PESADO)
			//	dpitch = DeltaPitch(this->Turret, pos)-(D3DXToRadian(sqrt(Distance(this->GetPosition(), pos)))*this->ai.aimvar);
			//else
			//	dpitch = DeltaPitch(this->Turret, pos);
			//dtyaw = DeltaYaw(this->Turret, pos);
			droll = DeltaYaw(this, pos);

			if(droll < -this->dados->hv)
				this->dados->ctipo->OnLeft(this);
			else if(droll > this->dados->hv)
				this->dados->ctipo->OnRight(this);//RollRight(obj);

			//CEixo tmp(0, 0);
			//tmp.setX(dtyaw*10);
			//tmp.setY(dpitch*10);
			/*if(dtyaw < -this->dados->hv)
				tmp.setX(-10);
			else if(dtyaw > this->dados->hv)
				tmp.setX(10);
			if(dpitch < -this->dados->vv)
				tmp.setY(-10);
			else if(dpitch > this->dados->vv) {
				tmp.setY(10);
			}*/
			//this->dados->ctipo->move(this, &tmp);
		}
	}
	PROFILE_END;
}


int CObjeto::UpdateObject()
{
	PROFILE_START;
	this->freio = false;
	this->dir = 0;
	if(this->morto)
	{
		if(this->j)
			this->j->obj = NULL;
		PROFILE_END;
		return 1;
	}
	if(this->life <= -100)
	{
		this->Explodir();
		if(this->j)
			this->j->obj = NULL;
		PROFILE_END;
		return 1;
	} else if(life > 0) {
		if(this->j)
			this->PlayerMove();
		else {
			if(this->getLider() && (this->getLider()->team != this->team) || !this->getLider()->IsValid())
				this->setLider(NULL);
			if(this->alvo)
				if((*this->alvo) && ((*this->alvo)->team == this->team) || !(*this->alvo)->IsValid())
					this->alvo = NULL;

			if(!this->getSquadNum() && this->ai.squadTimedOut()) {
				if(this->getLider() && random(100) <= 2) {
					ObjetoFalarSobreIrEmbora(this);
					this->ai.setTimeout(GameTicks + 20);
				} else {
					this->setLider(this->ConsultarObjetoMaisProximo(CONSULTAR_NOVO_LIDER));
					if(this->getLider()) {
						this->ai.setTimeout(GameTicks + 2000);
						if(this->getLider()->j)
							this->ai.delayTimeout(100000);
					} else
						this->ai.setTimeout(GameTicks + 20);
				}
			}
			this->dados->ctipo->CPUMove(this);
		}
	}
	this->dados->ctipo->Update(this);
	if(this->getLife() > 0)
		for(register size_t i = 0; i < mymission->GetBases().GetCapacity(); ++i) {
			if(this->getDistanceSq(mymission->GetBases()[i]) <= mymission->GetBases()[i].Dados()->getTipo()->getRaioDeInfluenciaSq()+this->dados->getRadiusSq()) {
				CBase *atual = &mymission->GetBases()[i];
				atual->Dominar(this->team);
				if(this->life < this->dados->armour)
					if(this->team == atual->Dona())
						if(equal(this->speed, 0)) {
							this->setDamage(-atual->Dados()->getTipo()->getRepairAmount()*atual->Influencia());
							this->setLife(Clamp(this->getLife(), 0, this->dados->armour));
							if(this->dados->ExplodArmourUp && GameTicks % 10 == 0)
								ps->CriarExplod(this->GetPosition(), this->dados->ExplodArmourUp, this->ptr->GetPtr());
						}
			}
		}
	
		
	/*switch(this->dados->tipo)
	{
		case TIPO_AERONAVE:
			ControleAeronave(this);
	};*/
	if(this->dados->tipo == TIPO_VEICULO)  {
		Vector vec = this->dados->meshCol.getVertice(0);
		vec.y -= WHEELDIST;
		this->suspensao[0].Update(vec);
		vec = this->dados->meshCol.getVertice(1);
		vec.y -= WHEELDIST;
		this->suspensao[1].Update(vec);
		vec = this->dados->meshCol.getVertice(4);
		vec.y -= WHEELDIST;
		this->suspensao[2].Update(vec);
		vec = this->dados->meshCol.getVertice(5);
		vec.y -= WHEELDIST;
		this->suspensao[3].Update(vec);
	}
	this->ResponderPerguntas();
	this->UpdatePhysics();
	//this->dados->meshCol.transformed(this->mWorld, *this->GetQuaternion(), this->mshCol);
	if(this->GetTarget() == NULL)
		if(this->getTarget())
			this->setTarget(NULL);
	if(this->dados->arma && (this->dados->arma->subtipo == TIPO_MISSEL || this->dados->arma->subtipo == TIPO_MIRV)) {
		if(this->getTarget() && *this->getTarget()) {
			CObjeto *tgt = *this->getTarget();
			const CHotSeekingProjectileData *dadosarma = static_cast<CHotSeekingProjectileData *>(this->dados->arma);
			if(this->getDistanceSq(tgt) <= dadosarma->disthoming) {
				Vector midpoint;
				real tx, ty;

				TFormPoint(&midpoint, tgt->dados->midpoint, tgt, NULL);

				if(this->dados->tipo == TIPO_VEICULO) {
					tx = DeltaPitch(this->Turret, &midpoint);
					ty = DeltaYaw(this->Turret, &midpoint);
				} else {
					tx = DeltaPitch(this, &midpoint);
					ty = DeltaYaw(this, &midpoint);
				}
				if(tx*tx + ty*ty <= dadosarma->getAngleHomingSq()) {
					if(this->lockOnTime < dadosarma->lockOnTime)
						++this->lockOnTime;
				} else
					this->lockOnTime = 0;
			} else
				this->lockOnTime = 0;
		} else
			this->lockOnTime = 0;
	}
	PROFILE_END;
	return 0;
}

void CObjeto::RenderObject()
{
	CTexture *tex;
	d3dDevice->SetTransform( D3DTS_WORLD, &this->mWorld );
	if(this->team && this->team->getID() >= 0 && this->team->getID() < eDM->getSize())
		tex = this->dados->meshTextures[this->team->getID()+1];
	else
		tex = this->dados->meshTextures[0];
	if(tex)
		d3dDevice->SetTexture(0, tex->Get());
	else
		d3dDevice->SetTexture(0, NULL);
	if(this->dados->Mesh)
		this->dados->Mesh->Render();
	if(this->dados->tipo == TIPO_VEICULO && static_cast<const DadosDaUnidadeTerrestre*>(this->dados)->Turret) {
		this->Turret->UpdateMatrix();
		d3dDevice->SetTransform( D3DTS_WORLD, &this->Turret->mWorld );
		static_cast<const DadosDaUnidadeTerrestre*>(this->dados)->Turret->Render();
	}
}

CObjeto::~CObjeto()
{
	SafeDelete(this->Turret);
	for(register unsigned int a = 0; a < 4; ++a) {
		SafeDelete(this->side[a]);
	}
	SafeDelete(this->perguntas);
	TRACE("Objeto %p removido!\n", this);
}