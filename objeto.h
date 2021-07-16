#ifndef _OBJETO_H
#define _OBJETO_H

#include "loader.h"
#include "3DEntity.h"
#include "InlineList.h"
#include "GameGlobals.h"
#include "radio.h"
#include "Geometria/Geometry.h"
#include "Geometria/Sphere.h"
#include "ObjetoLiderado.h"
#include "3DFunctions.h"
#include "3DParticles.h"
#include "MeshColisao.h"

class CJogador;

enum FSM_STATES {
	FSM_IDLE,
	FSM_FOLLOW,
	FSM_PATROL,
	FSM_FIXPOSITION,
	FSM_CAPTURE,
	FSM_GUARD,
	FSM_ATTACK,
	FSM_SEARCH,
	FSM_REPAIR
};

enum CONSULTA_OBJ {
	CONSULTAR_NOVO_ALVO,
	CONSULTAR_NOVO_LIDER
};

enum CONSULTA_BASE {
	CONSULTAR_BASE_INIMIGA,
	CONSULTAR_BASE_ALIADA_MAIS_AMEACADA,
	CONSULTAR_BASE_ALIADA_MAIS_PROXIMA
};

class Waypoint : public Vector
{
public:
	inline bool reachedSq(const Vector *position, const real dist = 200) const
	{
		return DistanceSq(this, position) <= dist;
	}

	inline bool reached(const Vector *position, const real dist = 200) const
	{
		return DistanceSq(this, position) <= dist*dist;
	}

	inline void gotoTo(const Vector *position, const real radius = 100)
	{
		this->x = position->x - radius + randomf(radius*2);
		this->y = position->y - radius + randomf(radius*2);
		this->z = position->z - radius + randomf(radius*2);
	}
	inline Vector &operator=(const Vector &right)
	{
		*((Vector*)this) = right;
		return *this;
	}
};

class FSM {
public:
	typedef unsigned int uint;
private:
	FSM_STATES fsm;
	uint time;
public:
	inline void reset() {
		this->fsm = FSM_IDLE;
		this->time = GameTicks;
	}

	inline uint getTime() const
	{ return this->time; }

	inline FSM_STATES get() const
	{ return this->fsm; }

	inline void set(const FSM_STATES fsm, const uint time)
	{
		this->fsm = fsm;
		this->time += time;
	}

	inline void setTimeToFinish(const uint timeToFinish) {
		this->time = GameTicks+timeToFinish;
	}

	inline uint timeToFinish() {
		if(!finished())
			return this->time-GameTicks;
		return 0;
	}

	inline bool finished() const
	{ return this->time < GameTicks; }
};

class AIVars
{
public:
	typedef unsigned int uint;
private:
	uint timeout;
public:
	FSM state;
	real aimvar;
	Waypoint wpt;

	inline void reset()
	{
		this->state.reset();
	}

	inline void setTimeout(const uint timeout)
	{ this->timeout = timeout; }

	inline bool squadTimedOut() const
	{ return this->timeout < GameTicks; }

	inline void delayTimeout(const uint amount)
	{ this->timeout += amount; }
};

#include "Equipe.h"

class CBase;

class CSuspensao : public Entity
{
	Vector force;
	real suspensionK, suspensionD;
	unsigned int collidedFrame;
public:
	CSuspensao():force(0, 0, 0), collidedFrame(0), Entity()
	{ }

	inline void setSuspensionK(const real suspensionK)
	{ this->suspensionK = suspensionK; }
	inline void setSuspensionD(const real suspensionD)
	{ this->suspensionD = suspensionD; }
	void Update(const Vector &posicao);
	inline void collidedAtFrame(const unsigned int frame)
	{ this->collidedFrame = frame; }
	inline bool collided(void) const
	{ return this->collidedFrame >= GameTicks-1; }
};

class CObjeto : public Entity, public CInlineNode, public ObjetoLiderado
{
	real life;
	real damage;
	CObjeto **alvo;
	unsigned int nextShoot;
	unsigned int nLockOns;
	CSimpleListAP<Pergunta> *perguntas;
public:
	CSuspensao suspensao[4];
	bool freio;
	int dir;
protected:
	void ResponderPerguntas();

	virtual void PlayerMove();
public:
	MeshColisao mshCol;
	unsigned int hitfloor;
	unsigned int pergAjuda;

	inline void incNLockOns(void)
	{ ++this->nLockOns; }
	inline void decNLockOns(void)
	{ --this->nLockOns; }
	inline unsigned int getNLockOns(void) const
	{ return this->nLockOns; }
	inline real getLife() const
	{ return this->life; }
	inline void setLife(const real life)
	{ this->life = life; }

	inline void setTarget(CObjeto **alvo) {
		if(this->alvo)
			if(*this->alvo)
				(*this->alvo)->decNLockOns();

		this->lockOnTime = 0;
		this->alvo = alvo;

		if(this->alvo)
			if(*this->alvo) {
				if((*this->alvo)->j && explodRuim)
					ps->CriarExplod(this->GetPosition(), explodRuim, this->ptr->GetPtr());
				(*this->alvo)->incNLockOns();
			}
	}

	inline CObjeto **getTarget(void) const {
		return this->alvo;
	}

	void setDamage(const real damage, CObjeto **shooter = NULL);
	real speed, gravidade, y_vel;
	int shootdelay, flags, pontuacao, skills, morto, falou, importante, controle;
	int id, esquadrao, colidiu, lockOnTime;
	const CEquipe *team;
	class CGrupoObjetoMissao *ogm;
	real voz;
	
	CBase *basealvo;
	CSimpleNode<CObjeto*> *ptr;
	const DadosDaUnidade *dados;
	CJogador *j;
	AIVars ai;
	//Vector targetSpot;
	Entity *Turret;
	CSuspensao *side[4];

	void Atacar();
	void falarPara(CObjeto *para, PerguntaBot id, unsigned int expira, unsigned int responder) const;

	inline bool temAlgumaRelacaoCom(const CObjeto *obj) const {
		if(obj)
			if(obj->getLider() == this || this->getLider() == obj || this == obj)
				return true;
		return false;
	}
	inline bool podeLiderar(CObjeto **obj) const;

	inline bool podeAtacar(CObjeto *target) {
		if(this->dados->arma) {
			if(target) {
				if(this->noRadar(target))
					if(
						(target->dados->tipo == TIPO_AERONAVE && this->dados->arma->targetType & ALVO_AR) || 
						(target->dados->tipo == TIPO_VEICULO && this->dados->arma->targetType & ALVO_CHAO) ||
						(target->dados->tipo == TIPO_HELICOPTERO && this->dados->arma->targetType & ALVO_HELICOPTERO)
					  )
						return true;
			}
		}
		return false;
	}

	inline bool podeAtacar(CObjeto **target) {
		return (target) ? this->podeAtacar(*target) : false;
	}

	inline bool noRadar(const Vector *pos) const
	{ return this->getDistanceSq(pos) <= this->dados->getRadarSq(); }

	inline bool noRadar(const Vector &pos) const
	{ return this->noRadar(&pos); }

	inline bool noRadar(const Entity *entity) const
	{ return this->noRadar(entity->GetPosition(true)); }

	inline bool noRadar(const Entity &entity) const
	{ return this->noRadar(&entity); }

	inline bool IsValid() const
	{
		if(this)
			if(this->ptr)
				return true;
		return false;
	}

	inline CObjeto *GetTarget() const
	{
		if(this->alvo)
			return *this->alvo;
		return NULL;
	}

	inline void GostarDestaEquipe()
	{
		this->ai.setTimeout(GameTicks+1000);
	}

	inline void ReacaoAoMatarInimigo()
	{
		TocarSomRadio(RADIO_MATOU_INIMIGO, this);
		if(this->getSquadNum())
			ObjetoPerguntarPara(this, PERGUNTA_IMPRESSIONAR_GRUPO, NULL, 4000);
		else if(this->getLider())
			ObjetoPerguntarPara(this, PERGUNTA_IMPRESSIONAR_LIDER, this->getLider(), 5000);

		GostarDestaEquipe();
	}

	CObjeto();
	HRESULT Atirar();
	void Explodir();

	void IAAtacar();
	void IAVirarPara(const Vector *pos);
	void IAIrParaEManterDistancia(const Vector *pos, const real distancia);
	void IAIrPara(const Vector *pos);
	int UpdateObject();
	void RenderObject();
	void Criar(const Vector *position, const int controle, const CEquipe *team, const DadosDaUnidade *data);
	CObjeto **ConsultarObjetoMaisProximo(const CONSULTA_OBJ tipo);
	CBase *ConsultarBaseMaisProxima(const CONSULTA_BASE tipo);
	void PickARandomPlace(const float radius);
	void DefCPUMove();

	inline void setLider(CObjeto **lider) {
		ObjetoLiderado::setLider(lider);
		if(lider)
			ObjetoFalarSobreNovoLider(this);
	}

	virtual ~CObjeto();
};

class CEixo;

class CObjetoT
{
public:
	virtual void Update(CObjeto *obj) = 0;
	virtual void CPUMove(CObjeto *obj) = 0;

	virtual void OnSpeed(CObjeto *obj) = 0;
	virtual void OnBreak(CObjeto *obj) = 0;
	virtual void OnLeft(CObjeto *obj) = 0;
	virtual void OnRight(CObjeto *obj) = 0;

	virtual void move(CObjeto *obj, const CEixo *) = 0;
};

extern CObjectManager<CObjeto> *g_Unidades;	// Vector of objects
extern CSimpleListAP<CObjeto*> *g_UnidadesPtr;
extern ProjectileDataManager *pDataManager;

extern CObjetoT **g_Objtypes;

void ColisaoComOTerreno(CObjeto *obj);

#endif