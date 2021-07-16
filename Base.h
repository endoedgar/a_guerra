#ifndef _BASE_H
#define _BASE_H

class CEquipe;
#include "3DFunctions.h"
#include "Bandeira.h"
#include "CTerrain.h"
#include "GameGlobals.h"
#include "Equipe.h"
#include "NamedEntity.h"

class CObjeto;
class DadosDaUnidade;
extern CMesh *MeshCubo;

class CBaseUnidade {
	const DadosDaUnidade *unidade;
	real probabilidade;
public:
	CBaseUnidade()
	{ }
	CBaseUnidade(const DadosDaUnidade *unidade):unidade(unidade)
	{ }
	CBaseUnidade(const DadosDaUnidade *unidade, const real probabilidade):unidade(unidade), probabilidade(probabilidade)
	{ }

	const DadosDaUnidade *getUnidade() const
	{ return this->unidade; }

	void setUnidade(const DadosDaUnidade *unidade)
	{ this->unidade = unidade; }

	real getProbabilidade() const
	{ return this->probabilidade; }

	void setProbabilidade(const real probabilidade)
	{ this->probabilidade = probabilidade; }
};


class CBaseTipoDados : public NamedEntity {
protected:
	Smart::LinkedList<CBaseUnidade> unidades;
	real rinfluencia, rinfluenciasq, vconquista, maiorprobabilidade;
	size_t mingen, maxgen, gentime;
	real repairAmount;

public:
	CBaseTipoDados(): maiorprobabilidade(0)
	{ }
	inline size_t getNUnidades(void) const
	{
		return this->unidades.GetCapacity();
	}

	inline void setRepairAmount(const real repairAmount) 
	{ this->repairAmount = repairAmount; }

	inline real getRepairAmount(void) const
	{ return this->repairAmount; }

	inline void setRaioDeInfluencia(const real raio)
	{
		this->rinfluencia = raio;
		this->rinfluenciasq = this->rinfluencia;
		this->rinfluenciasq *= this->rinfluenciasq;
	}

	inline real getRaioDeInfluencia(void) const
	{ return this->rinfluencia; }

	inline real getRaioDeInfluenciaSq(void) const
	{ return this->rinfluenciasq; }

	inline void setVelDeConquista(const real vel)
	{ this->vconquista = vel; }

	inline real getVelDeConquista() const
	{ return this->vconquista; }

	inline void Gen(const size_t mingen, const size_t maxgen, const size_t gentime)
	{
		this->mingen = mingen;
		this->maxgen = maxgen;
		this->gentime = gentime;
	}

	inline size_t MinGen() const
	{ return this->mingen; }
	
	inline size_t MaxGen() const
	{ return this->maxgen; }

	inline size_t GenTime() const
	{ return this->gentime; }


	void InserirUnidade(const DadosDaUnidade *ud, const real probabilidade)
	{
		this->unidades.RInsert(new CBaseUnidade(ud, probabilidade));
		//if(probabilidade > this->maiorprobabilidade)
		this->maiorprobabilidade += probabilidade;
	}

	const DadosDaUnidade *ReceberUmaUnidade() const
	{
		Smart::LinkedList<CBaseUnidade>::LinkedNode *node = this->unidades.GetFirst();
		real soma = 0;
		while(node) {
			soma += (*node)->getProbabilidade();
			if(randomf(1) <= soma)
				return (*node)->getUnidade();
			node = node->GetNext();
		}
		return this->unidades.GetLast()->GetData().getUnidade();
	}

	void NormalizarProbabilidades()
	{
		Smart::LinkedList<CBaseUnidade>::LinkedNode *node = this->unidades.GetFirst();
		while(node) {
			(*node)->setProbabilidade((*node)->getProbabilidade()/this->maiorprobabilidade);
			node = node->GetNext();
		}
	}

	friend class CBaseDados;
};

class CBaseDados : public NamedEntity {
	const CEquipe *donainicial;
	const CBaseTipoDados *tipo;
	Vector pos;
public:
	CBaseDados():tipo(NULL), donainicial(NULL)
	{ }

	inline void setFaccao(const CEquipe *faccao)
	{ this->donainicial = faccao; }

	inline const CEquipe *getFaccao() const
	{ return this->donainicial; }

	inline void setTipo(const CBaseTipoDados *tipo)
	{ this->tipo = tipo; }

	inline const CBaseTipoDados *getTipo() const
	{ return this->tipo; }

	inline void setPosicao(const Vector &pos)
	{ this->pos = pos; }

	inline void setPosicao(const real x, const real y, const real z)
	{
		this->pos.x = x;
		this->pos.y = y;
		this->pos.z = z;
	}

	inline const Vector &getPosicao() const
	{ return this->pos; }

	inline const DadosDaUnidade *ReceberUmaUnidade() const
	{
		return this->getTipo()->ReceberUmaUnidade();
	}
};

extern CObjectManager<CObjeto> *g_Unidades;	// Vector of objects

class CBase : public Entity
{
	Entity flag;
	real influencia;
	const CEquipe *dona;
	const CBaseDados *dados;
	size_t nDefesas;
	int nInstabilidade;
public:
	CBase():Entity(), nInstabilidade(0), nDefesas(0)
	{ }

	inline const CBaseDados *Dados() const
	{ return this->dados; }

	inline const CEquipe *Dona() const
	{ return this->dona; }

	void Setup(const CBaseDados *dados, const Vector &hotspot)
	{
		const Vector tmp = dados->getPosicao()+hotspot;
		this->dados = dados;
		this->Position(&tmp);
		this->PositionY(terrain->GetTerrainHeight(this->GetPosition()));
		flag.SetupPhysics();
		this->Scale(5, 5, 5);
		flag.Parent(this);
		flag.Position(0, 0, 0);

		this->Reset();
	}

	void Reset()
	{
		this->dona = this->dados->getFaccao();
		if(this->dona) { 
			this->influencia = 1;
		} else
			this->influencia = 0;
		this->nInstabilidade = this->nDefesas = 0;
	}

	void Update(const real dt)
	{
		this->flag.Position(0, 8*this->influencia, 0);
	}

	HRESULT Render();

	void Reinforcements();

	void Dominar(const CEquipe *dominador);

	inline void Defender()
	{
		if(this->nDefesas < GameTicks)
			this->nDefesas = GameTicks;
		this->nDefesas++;
	}

	inline bool Defendida() const
	{
		return (nDefesas > GameTicks);
	}

	inline int Ameaca() const
	{
		return (this->nInstabilidade-static_cast<int>(GameTicks));
	}

	inline int NecessidadeDeDefesas() const
	{
		return ((this->Ameaca()/60)-this->nDefesas);
	}

	inline real Influencia() const
	{ return this->influencia; }
};

#endif