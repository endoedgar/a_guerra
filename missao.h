#ifndef _MISSAO_H
#define _MISSAO_H

#include "base.h"
class CEquipe;
class CBaseTipoDados;
class CBaseDados;
class Ambiente;
class CBase;
class DadosDaUnidade;

#pragma once

enum TIPO_TAREFA {
	TAREFA_DESTRUIR,
	TAREFA_PROTEGER,
	TAREFA_ESPERAR,
	TAREFA_DOMINACAO,
	TAREFA_CAPTURAR,
	TAREFA_DEFENDER_BASE
};

class UnidadeMissao : public NamedEntity {
	Vector posicao;
	DadosDaUnidade *ud;
	real life;
public:
	UnidadeMissao():posicao(Vector::zero()), ud(NULL), life(0)
	{ }

	UnidadeMissao(const Vector &posicao, DadosDaUnidade *ud, const real life):posicao(posicao), ud(ud), life(life)
	{ }

	inline void setPosicao(const real x, const real y, const real z)
	{ 
		Vector tmp(x, y, z);
		this->setPosicao(tmp);
	}

	inline void setPosicao(const Vector &posicao)
	{ this->posicao = posicao; }

	inline const Vector &getPosicao(void) const
	{ return this->posicao; }

	inline void setUD(DadosDaUnidade *ud)
	{ this->ud = ud; }

	inline DadosDaUnidade *getUD(void) const
	{ return this->ud; }

	inline void setLife(const real life)
	{ this->life = life; }

	inline real getLife(void) const
	{ return this->life; }
};

class GrupoObjetoMissaoDados : public NamedEntity {
public:
	size_t nUnidades, index;
	Smart::Array<UnidadeMissao> unidades;
	const CEquipe *team;
	unsigned int objetivo;

	GrupoObjetoMissaoDados()
	{ }
};

class CTarefaDados : public NamedEntity {
	TIPO_TAREFA tipo;

	GrupoObjetoMissaoDados *objgrupo;
	size_t base;
public:
	void SetType(const TIPO_TAREFA tipo)
	{ this->tipo = tipo; }

	TIPO_TAREFA GetType() const
	{ return this->tipo; }

	GrupoObjetoMissaoDados *GetObjGroup() const
	{ return this->objgrupo; }

	size_t getBase(void) const
	{ return this->base; }

	void setBase(const size_t base)
	{ this->base = base; }

	void SetObjGroup(GrupoObjetoMissaoDados *gom)
	{ this->objgrupo = gom; }

	CTarefaDados() : tipo(TAREFA_DESTRUIR), objgrupo(NULL)
	{ }
};

class CObjetivo : public NamedEntity {
public:
	Smart::Array<CTarefaDados> pTarefas;
	unsigned int maxSec;
	Smart::String szMusic;
	bool admv;

public:
	CObjetivo() {
	}

	~CObjetivo()
	{ }

	void setNome(const Smart::String &nome);

	void SetNTask(const size_t i)
	{
		this->pTarefas.SetSize(i);
	}

	bool arquivoDeMusicaValido(void) const
	{ return this->admv; }

	size_t GetSize() const
	{ return this->pTarefas.GetCapacity(); }

	const CTarefaDados *operator[](const size_t i) const
	{
		return &pTarefas[i];
	}

	CTarefaDados *operator()(const size_t i)
	{
		return &pTarefas[i];
	}
};

class ErrorInfo {
	Smart::String info;
public:
	ErrorInfo()
	{ this->clear(); }

	inline void set(const Smart::String &info) {
		this->info = info;
	}

	inline const Smart::String &getInfo() const
	{ return this->info; }

	inline void clear()
	{
		this->info = "Sem Erro";
	}
};

class CMissaoDados : public ExternalData {
public:
	Smart::String terreno, mapaFilename, terrainPath, terrainTexturePath, terrainDetailTexturePath;
	real terrainFriction, marPosition;
	Vector terrainPosition, terrainScale;
protected:
	
	//Smart::Ptr<CTerrain> terreno;
	const CEquipe *EncontrarEquipe(const Smart::String &equipedesejada) const;
	const CEquipe *AdicionarEquipe(const Smart::String &equipedesejada);
	const CBaseTipoDados *EncontrarBaseTipo(const Smart::String &tbasedesejada) const;
	const CBaseDados *EncontrarBase(const Smart::String &basedesejada) const;
	const Ambiente *ambdados;

#ifdef UNICODE
	typedef std::wostringstream missionstream;
#else
	typedef std::ostringstream missionstream;
#endif

	bool CarregarTerreno(const INIParser::INIFile &file);
	bool CarregarBases(const INIParser::INIFile &file);
	bool CarregarObjetivos(const INIParser::INIFile &file);
	bool CarregarGrupoDeObjetos(const INIParser::INIFile &file);

	public:

	static ErrorInfo lastError;

	Smart::Array<CObjetivo> pObjetivos;
	Smart::Array<GrupoObjetoMissaoDados> pGObjetos;
	Smart::LinkedList<const CEquipe*> pEquipesParticipantes;
	Smart::LinkedList<CBaseTipoDados> pBasesTipos;
	Smart::LinkedList<CBaseDados> pBases;

	CMissaoDados(const Smart::String &szNome):terreno("") {
		this->setNome(szNome);
	}

	inline const Ambiente *getAmbiente(void) const
	{ return this->ambdados; }

	bool Unload();

	~CMissaoDados();

	Vector p1pos;
	Vector hotspot;
	real p1life;

	const CObjetivo *operator[](const size_t i) const
	{
		return &pObjetivos[i];
	}

	CObjetivo *operator()(const size_t i)
	{
		return &pObjetivos[i];
	}

	static CMissaoDados *Carregar(const Smart::String &filename);

	friend class MissionDataManager;
};

class CGrupoObjetoMissao {
	size_t nActive;
public:
	CGrupoObjetoMissao():nActive(0)
	{ }

	inline void Reset()
	{ this->nActive = 0; }

	inline void operator++()
	{ this->nActive++; }

	inline void operator--()
	{ this->nActive--; }

	inline size_t GetActive() const
	{ return this->nActive; }
};

class CMissao {
	const CMissaoDados *dados;
	Smart::Array<CGrupoObjetoMissao> cgom;
	Smart::Array<CBase> bases;
public:
	CMissao():dados(NULL)
	{ }

	size_t GetNObjetivos() const
	{
		if(dados)
			return this->dados->pObjetivos.GetCapacity();
		return 0;
	}

	size_t getNBases(const CEquipe *) const;

	inline const CMissaoDados *GetDados() const
	{ return this->dados; }
	
	inline const Smart::Array<CBase> &GetBases() const
	{ return this->bases; }

	void Reset();

	void Start(const CMissaoDados *cd);

	void Update(const real dt);

	void Render();

	CGrupoObjetoMissao &GetGOM(const size_t i)
	{
		return this->cgom[i];
	}

	CGrupoObjetoMissao &GetGOM(const CTarefaDados *ct)
	{
		return this->cgom[ct->GetObjGroup()->index];
	}

	friend class MissionDataManager;
};

#endif