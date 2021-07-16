#ifndef _3DPARTICLES_H
#define _3DPARTICLES_H

#include "D3DObject.h"
#include "3DEntity.h"
#include "InlineList.h"
#include "loader.h"

class CCamera;
class Spark;

int UpdateSpark(Spark *);
int UpdateSprite(Spark *);

typedef struct tagSparkZInfo
{
	Spark *spark;
	real z;
}	SparkZInfo;

typedef struct tagSVERTEX
{
	Vector coord;
	D3DCOLOR color;
	float tu, tv;
} SVERTEX;

const DWORD PVERTEXFVF = (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);

class Emissor : public CInlineNode
{
public:
	Vector pos;
	unsigned int lifetime, id, intervalo;
	const SparkUnit *su;
	const Entity *binder;

	Emissor();
	int Update(class ParticleSystem *ps);
	~Emissor();
};

class Spark : public CInlineNode
{
	unsigned int ib;
	unsigned short vb;
	bool drawn;
public:
	inline void setIB(const unsigned int ib)
	{ this->ib = ib; }
	inline void setVB(const unsigned short vb)
	{ this->vb = vb; }
	inline unsigned int getIB(void) const
	{ return this->ib; }
	inline unsigned short getVB(void) const
	{ return this->vb; }
	inline void setDrawn(const bool drawn)
	{ this->drawn = drawn; }
	inline bool getDrawn(void) const
	{ return this->drawn; }
	Vector pos, vel, endpos;
	real alpha, alpha_vel, alpha_acel;
	unsigned int color;
	real scale, cv, cor_vel, cor_acel;
	const SparkUnit *su;
	const Entity *binder;

	//0 - Spark
	//1 - Line
	int tipo;

	Spark();
	int (*Update)(Spark *);
	~Spark();
};

class SparkPack : public CInlineNode
{
public:
	unsigned int ib_start, scount;
	const SparkUnit *su;
	unsigned short minVertex, maxVertex;
};

extern const Vector kVento;

class ParticleSystem : public CResource, public VideoResource
{
public:
	CObjectManager<Spark> *m_pParticles;
	CObjectManager<Emissor> *m_pEmitters;

	CObjectManager<SparkPack> *m_pPackList;

	size_t VBLength, IBLength, dsparks;

	LPDIRECT3DVERTEXBUFFER9 m_pVB;
	LPDIRECT3DINDEXBUFFER9  m_pIB;

	unsigned int pTicks;
	SparkZInfo *szi;
	size_t szin;

	ParticleSystem();
	HRESULT Update();
	HRESULT OnLostDevice();
	HRESULT OnResetDevice();
	HRESULT Render(const CCamera *cam);
	HRESULT CriarEmissor(const Vector *pos, const SparkUnit *, const Entity *binder = NULL);
	Spark *CreateParticle(const Vector *pos, const Vector *vel, const real alpha, const real avel, const real aacel, const real scale, const real cv, const SparkUnit *su, int (*UpdateSpark)(Spark *) = UpdateSpark, const Entity *binder = NULL);
	HRESULT CriarExplod(const Vector *pos, const DadosDoSpark *sd, class CObjeto **shooter = NULL, Entity *binder = NULL);
	~ParticleSystem();

	friend unsigned int __stdcall UpdateSparkPack(void *ptr);
};

#endif