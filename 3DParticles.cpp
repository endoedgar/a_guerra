#include "stdafx.h"
#include "GameGlobals.h"
#include "3DCamera.h"
#include "3DParticles.h"
#include "3DFunctions.h"
#include "loader.h"
#include "GenericFunctions.h"
#include "globalheader.h"
#include "objeto.h"
#include "D3DObject.h"
#include "GameHUD.h"

Emissor::Emissor()
{
	static unsigned int ix = 0;
	this->id = ix;
	++ix;
	ZeroMemory(this, sizeof(Emissor));
}

int Emissor::Update(ParticleSystem *ps)
{
	if(this->binder)
		this->pos = *this->binder->GetPosition();
	if(!(GameTicks%this->intervalo))
	{
		Vector destpos, vel;
		vel.x = this->su->minv.x+randomf(this->su->maxv.x-this->su->minv.x);
		vel.y = this->su->minv.y+randomf(this->su->maxv.y-this->su->minv.y);
		vel.z = this->su->minv.z+randomf(this->su->maxv.z-this->su->minv.z);

		destpos.x = this->pos.x+randomf(this->su->max.x-this->su->min.x);
		destpos.y = this->pos.y+randomf(this->su->max.y-this->su->min.y);
		destpos.z = this->pos.z+randomf(this->su->max.z-this->su->min.z);
		if(this->binder)
			destpos += *this->binder->GetPosition();

		ps->CreateParticle(&this->pos, &vel, 1, this->su->alpha_vel, this->su->alpha_acel, this->su->minstartscale+randomf(this->su->maxstartscale-this->su->minstartscale), this->su->minscalevel+randomf(this->su->maxscalevel-this->su->minscalevel), this->su); 
	}
	if(this->lifetime < GameTicks)
		return 1;
	return 0;
}

Emissor::~Emissor()
{
}

Spark::Spark()
{
}

int UpdateSprite(Spark *sp)
{
	//unsigned char alpha = static_cast<unsigned char>(sp->alpha*255);
	if(sp->getDrawn()) {
		sp->alpha = 0;
		return 1;
	}
	

	//sp->pos += sp->vel;
	return 0;
	//this->pos.z += 0.2;
}

int UpdateSpark(Spark *sp)
{
	unsigned char alpha = static_cast<unsigned char>(sp->alpha*255);
	if(sp->cv != 0)
		sp->scale += sp->cv;
	sp->color = (alpha << 24);
	
	int coralterada = 0;
	if(sp->cor_vel > 0) {
		real r = static_cast<real>((sp->su->cor >> 16) & 0xFF);
		real g = static_cast<real>((sp->su->cor >> 8) & 0xFF);
		real b = static_cast<real>((sp->su->cor) & 0xFF);
		r *= sp->cor_vel;
		g *= sp->cor_vel;
		b *= sp->cor_vel;
		coralterada |= static_cast<int>(r) << 16;
		coralterada |= static_cast<int>(g) << 8;
		coralterada |= static_cast<int>(b);
	}

	if(sp->su->blendflags == 3)
		sp->color |= coralterada;
	else {
		real r = static_cast<real>((coralterada >> 16) & 0xFF);
		real g = static_cast<real>((coralterada >> 8) & 0xFF);
		real b = static_cast<real>((coralterada) & 0xFF);
		r *= mywrapper->light->light->Diffuse.r;
		g *= mywrapper->light->light->Diffuse.g;
		b *= mywrapper->light->light->Diffuse.b;
		sp->color |= static_cast<int>(r) << 16;
		sp->color |= static_cast<int>(g) << 8;
		sp->color |= static_cast<int>(b);
	}
	sp->cor_vel += sp->cor_acel;

	if(sp->alpha > 0) {
		sp->alpha -= sp->alpha_vel;
		sp->alpha_vel += sp->alpha_acel;
	} else
		return 1;
	sp->pos += sp->vel;
	return 0;
	//this->pos.z += 0.2;
}

Spark::~Spark()
{
}

ParticleSystem::ParticleSystem():VBLength(0), IBLength(0), dsparks(0), m_pVB(NULL), m_pIB(NULL), pTicks(0), CResource(CLASSE_VIDEO)
{
	TRACE("Criando Sistema de Partículas %p...\n", this);

	this->m_pEmitters = new CObjectManager<Emissor>();
	this->m_pEmitters->Precache(gc->GetPrecacheEmitters());
	this->m_pParticles = new CObjectManager<Spark>();
	this->m_pParticles->Precache(gc->GetPrecacheParticles());
	this->m_pPackList = new CObjectManager<SparkPack>;
	this->m_pPackList->Precache(gc->GetPrecacheParticlesPack());
	this->szin = gc->GetPrecacheParticles();
	this->szi = new SparkZInfo[szin];

	mywrapper->AddResource(this);
}

ParticleSystem::~ParticleSystem()
{
	TRACE("Liberando sistema de partículas %p...\n", this);

	SafeDelete(this->m_pParticles);
	SafeDelete(this->m_pEmitters);

	SafeRelease(this->m_pVB);
	SafeDelete(this->m_pPackList);
	SafeDeleteArray(this->szi);

	TRACE("Sistema de partículas %p liberado!\n", this);
}

HRESULT ParticleSystem::CriarEmissor(const Vector *pos, const SparkUnit *su, const Entity *binder)
{
	Emissor *pEmissor;
	/*if( this->m_pEmitters->GEFreeList->GetFirst() )
	{
		pEmissor = static_cast<Emissor*>(this->EFreeList->Pop());
		// If so, hand over the first free one to be reused.
		pEmissor = this->EFreeList;
		// Then make the next free particle in the list next to go!
		this->EFreeList = pEmissor->m_pNext;
		if(this->EFreeList)
			this->EFreeList->m_pPrev = NULL;
	}
	else
	{
		// There are no free particles to recycle...
		// We'll have to create a new one from scratch!
		//if( m_dwActiveCount < m_dwMaxParticles )
		//{
		if( NULL == ( pEmissor = new Emissor ) )
			return E_OUTOFMEMORY;
		//}
	}*/
	pEmissor = this->m_pEmitters->New();
	
	pEmissor->pos = (*pos);
	pEmissor->su = su;
	pEmissor->intervalo = max(1, su->emissor->intervalo);
	pEmissor->lifetime = GameTicks+su->emissor->lifetime;
	pEmissor->binder = binder;

	//this->EActiveList->Insert(pEmissor);
	/*pEmissor->SetPrevious(NULL);
	//pEmissor->m_pPrev = NULL;
	if(this->EActiveList->GetFirst())
		this->EActiveList->GetFirst()->SetPrevious(pEmissor);
	pEmissor->m_pNext = this->EActiveList; // Make it the new head...
	this->EActiveList = pEmissor;*/
	return S_OK;
}

Spark *ParticleSystem::CreateParticle(const Vector *pos, const Vector *vel, const real alpha, const real alpha_vel, const real alpha_acel, const real scale, const real cv, const SparkUnit *su, int (*US)(Spark *), const Entity *binder)
{
	Spark *pSpark;
	/*if( this->m_pFreeList->GetFirst() )
	{
		pSpark = static_cast<Spark*>(this->m_pFreeList->Pop());
		// If so, hand over the first free one to be reused.
		pSpark = this->m_pFreeList;
		// Then make the next free particle in the list next to go!
		this->m_pFreeList = pSpark->m_pNext;
		if(this->m_pFreeList)
			this->m_pFreeList->m_pPrev = NULL;
	}
	else
	{
		// There are no free particles to recycle...
		// We'll have to create a new one from scratch!
		//if( m_dwActiveCount < m_dwMaxParticles )
		//{
		if( NULL == ( pSpark = new Spark ) )
			return E_OUTOFMEMORY;
		//}
	}*/
	
	pSpark = this->m_pParticles->New();

	pSpark->pos = *pos;
	/*pSpark->pos.x *= -1;
	pSpark->pos.z *= -1;*/
	pSpark->vel = *vel;
	pSpark->alpha = alpha;
	pSpark->scale = scale;
	pSpark->cv = cv;
	pSpark->alpha_vel = alpha_vel;
	pSpark->alpha_acel = alpha_acel;
	pSpark->su = su;
	pSpark->cor_vel = su->cor_vel;
	pSpark->cor_acel = su->cor_acel;
	pSpark->Update = US;
	pSpark->setDrawn(false);

	//this->m_pActiveList->Insert(pSpark);
	/*pSpark->m_pPrev = NULL;
	if(this->m_pActiveList)
		this->m_pActiveList->m_pPrev = pSpark;
	pSpark->m_pNext = this->m_pActiveList; // Make it the new head...
	this->m_pActiveList = pSpark;*/
	return pSpark;
}

HRESULT ParticleSystem::Update()
{
	Emissor *pEmissor;
	PROFILE_START;

	pEmissor = this->m_pEmitters->GetFirstActive();

	while(pEmissor)
	{
		if(pEmissor->Update(this))
			this->m_pEmitters->Delete(&pEmissor);
		else
			pEmissor = this->m_pEmitters->GetNext(pEmissor);
	}

	Spark *pSpark = this->m_pParticles->GetFirstActive();
	while(pSpark)
	{
		if(pSpark->Update(pSpark)) {
			this->m_pParticles->Delete(&pSpark);
		} else
			pSpark = static_cast<Spark*>(pSpark->GetNext());	
	}

	pTicks++;
	PROFILE_END;
	return S_OK;
}

int cmp(const void *c1, const void *c2)
{
	const SparkZInfo *szi1 = static_cast<const SparkZInfo*>(c1);
	const SparkZInfo *szi2 = static_cast<const SparkZInfo*>(c2);
	
	return static_cast<int>(szi1->z-szi2->z);
}

HRESULT ParticleSystem::OnLostDevice()
{
	SafeRelease(this->m_pIB);
	SafeRelease(this->m_pVB);

	return S_OK;
}

HRESULT ParticleSystem::OnResetDevice()
{
	HRESULT hr;

	if(this->VBLength) {
		hr = d3dDevice->CreateVertexBuffer(this->VBLength*sizeof(SVERTEX), D3DUSAGE_DONOTCLIP | D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, PVERTEXFVF, D3DPOOL_DEFAULT, &this->m_pVB, NULL);
		if(SUCCEEDED( hr ))
		{
			hr = d3dDevice->CreateIndexBuffer(sizeof(short)*IBLength, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_pIB, NULL);
			if(FAILED( hr ))
				RuntimeError("Falha ao tentar criar buffer de índices para o sistema de partículas %p\nCódigo do erro: %d", this, HRESULT_CODE(hr));
		}
		else
			RuntimeError("Falha ao tentar criar buffer de vértices para o sistema de partículas %p\nCódigo do erro: %d", this, HRESULT_CODE(hr));
	} else
		hr = S_OK;

	return hr;
}

/*const D3DXVECTOR2 g_sQuad[4] =
{
	D3DXVECTOR2( -0.5, -0.5 ),
	D3DXVECTOR2( 0.5, -0.5 ),
	D3DXVECTOR2( 0.5, 0.5 ),
	D3DXVECTOR2( -0.5, 0.5 )
};*/

/*struct VECTOR2D
{
	real x, y;

	VECTOR2D()
	{ }

	VECTOR2D(const real x, const real y):x(x), y(y)
	{ }
};*/

const VECTOR2D g_sQuad2[4] =
{
	VECTOR2D( 0, 0 ),
	VECTOR2D( 1, 0 ),
	VECTOR2D( 0, 1 ),
	VECTOR2D( 1, 1 )
};


const VECTOR2D g_sQuad[4] =
{
	VECTOR2D( -1, -1 ),
	VECTOR2D( 1, -1 ),
	VECTOR2D( 1, 1 ),
	VECTOR2D( -1, 1 )
};

void QuickDepthSort( SparkZInfo* indices, const int lo, const int hi )
{
    //  lo is the lower index, hi is the upper index
    //  of the region of array a that is to be sorted
    int i = lo, j = hi;
    SparkZInfo index;
    float x = indices[( lo + hi ) / 2].z;

    //  partition
    do
    {
        while( indices[i].z < x ) i++;
        while( indices[j].z > x ) j--;
        if( i <= j )
        {
            index = indices[i]; indices[i] = indices[j]; indices[j] = index;
            i++; j--;
        }
    } while( i <= j );

    //  recursion
    if( lo < j ) QuickDepthSort( indices, lo, j );
    if( i < hi ) QuickDepthSort( indices, i, hi );
}

HRESULT ParticleSystem::Render(const CCamera *cam)
{
	SVERTEX* pVertexData; //pointer to vertex
	unsigned short* pIndexData; //pointer to index
	Spark *pSpark;
	HRESULT hr;
	SparkPack *sp = NULL;
	SparkZInfo *pSzi;
	int blendflags = 0;
	SVERTEX *vd;
	unsigned short u = 0;

	if(this->m_pParticles->GetNumActive())
	{
		const Vector vRight( -cam->GetMatrixView()->_11, -cam->GetMatrixView()->_21, -cam->GetMatrixView()->_31 );
		const Vector vUp( cam->GetMatrixView()->_12, cam->GetMatrixView()->_22, cam->GetMatrixView()->_32 );
		const Vector vFoward( cam->GetMatrixView()->_13, cam->GetMatrixView()->_23, cam->GetMatrixView()->_33 );

		if(szin < this->m_pParticles->GetNumActive())
		{
			if(szi)
			{
				TRACE("ParticleSystem::Render: Buffer Z não suficiente!\n");
				delete [] szi;
				szi = NULL;
			}
			TRACE("ParticleSystem::Render: Aumentando de %d para %d particulas Z\n", szin, this->m_pParticles->GetNumActive());
			szin = this->m_pParticles->GetNumActive();
			szi = new SparkZInfo[szin];
		}
		const size_t sizeneeded = this->m_pParticles->GetNumActive()*4;
		if(this->m_pVB)
		{
			if(this->VBLength < sizeneeded) {
				TRACE("ParticleSystem::Render: Buffer de vertices não suficiente!\n");
				m_pVB->Release();
				m_pVB = NULL;
			}
		}
		if(!this->m_pVB) {
			TRACE("ParticleSystem::Render: Aumentando de %d para %d vértices\n", this->VBLength, sizeneeded);
			this->VBLength = sizeneeded;
			hr = d3dDevice->CreateVertexBuffer(this->VBLength*sizeof(SVERTEX), D3DUSAGE_DONOTCLIP | D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, PVERTEXFVF, D3DPOOL_DEFAULT, &this->m_pVB, NULL);
			if(FAILED(hr)) {
				TRACE("ParticleSystem::Render: Falha! (Código: %d)\n", HRESULT_CODE(hr));
				return hr;
			}
		}

		pSpark = this->m_pParticles->GetFirstActive();

		this->dsparks = 0;

		hr = this->m_pVB->Lock(0,this->m_pParticles->GetNumActive()*sizeof(SVERTEX), reinterpret_cast<void**>(&pVertexData), D3DLOCK_DISCARD);
		if(FAILED(hr)) {
			return hr;
		}

		PROFILE_START;
		const float fSinTheta = sinf( D3DXToRadian(pTicks) );
		const float fCosTheta = cosf( D3DXToRadian(pTicks) );
		while(pSpark)
		{
			if(u + 4 <= 0xFFFF)
			if(cam->SphereVisible(&pSpark->pos, pSpark->scale)) //&& DistanceF(cam->GetPosition(), &pSpark->pos) <= 2500000)
			{	
				pSpark->setDrawn(true);
				VECTOR2D New[4];
				for( register size_t v = 0; v < 4; ++v )
				{
					New[v].x = fCosTheta * g_sQuad[v].x - fSinTheta * g_sQuad[v].y;
					New[v].y = fSinTheta * g_sQuad[v].x + fCosTheta * g_sQuad[v].y;

					New[v].x *= pSpark->scale;
					New[v].y *= pSpark->scale;
				}

				Vector vDelta = cam->GetPosition();
				vDelta -= pSpark->pos;
				szi[dsparks].spark = pSpark;
				szi[dsparks].z = vDelta.lengthsq();//D3DXVec3LengthSq(&vDelta);

				pSpark->setVB(u);

				/*cxx=camx.x*pSpark->scale;
				cxy=camx.y*pSpark->scale;
				cxz=camx.z*pSpark->scale;
				cyx=camy.x*pSpark->scale;
				cyy=camy.y*pSpark->scale;
				cyz=camy.z*pSpark->scale;*/

				vd = &pVertexData[u];

				vd->coord = pSpark->pos;
				vd->coord += (vRight * New[0].x + vUp * New[0].y)*cam->GetScale()->y;
				vd->color = pSpark->color; //color
				vd->tu = pSpark->su->startuv.x;
				vd->tv = pSpark->su->startuv.y;
				++vd;

				vd->coord = pSpark->pos;
				vd->coord += (vRight * New[1].x + vUp * New[1].y)*cam->GetScale()->y;
				vd->color = pSpark->color; //color
				vd->tu = pSpark->su->enduv.x;
				vd->tv = pSpark->su->startuv.y;
				++vd;

				vd->coord = pSpark->pos;
				vd->coord += (vRight * New[2].x + vUp * New[2].y)*cam->GetScale()->y;
				vd->color = pSpark->color; //color
				vd->tu = pSpark->su->enduv.x;
				vd->tv = pSpark->su->enduv.y;
				++vd;

				vd->coord = pSpark->pos;
				vd->coord +=(vRight * New[3].x + vUp * New[3].y)*cam->GetScale()->y;
				vd->color = pSpark->color; //color
				vd->tu = pSpark->su->startuv.x;
				vd->tv = pSpark->su->enduv.y;
				++vd;

				u+=4;

				++this->dsparks;
			}

			pSpark = this->m_pParticles->GetNext(pSpark);
		}
		PROFILE_END;

		this->m_pVB->Unlock();

		const size_t IBsizeneeded = this->m_pParticles->GetNumActive()*6;
		if(this->m_pIB)
		{
			if(this->IBLength < IBsizeneeded)
			{
				TRACE("ParticleSystem::Render: Buffer de indices não suficiente!\n");
				m_pIB->Release();
				m_pIB = NULL;
			}
		
		}
		if(!this->m_pIB)
		{
			TRACE("ParticleSystem::Render: Aumentando de %d para %d entradas\n", this->IBLength, IBsizeneeded);
			this->IBLength = IBsizeneeded;
			hr = d3dDevice->CreateIndexBuffer(sizeof(unsigned short)*IBLength, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_pIB, NULL);
			if(FAILED(hr)) {
				TRACE("ParticleSystem::Render: Falha! (Código: %d)\n", HRESULT_CODE(hr));
				return hr;
			}
		}

		pSpark = this->m_pParticles->GetFirstActive();

		QuickDepthSort(szi, 0, this->dsparks-1);
		//qsort(szi, this->dsparks, sizeof(SparkZInfo), cmp);

		pSzi = szi;

		// TODO: Multithreading
		hr = m_pIB->Lock(0,this->dsparks*sizeof(unsigned short), reinterpret_cast<LPVOID*>(&pIndexData), D3DLOCK_DISCARD);
		if(FAILED(hr)) {
			return hr;
		}

		for(register size_t i = 0; i < this->dsparks; ++i)
		{
			pSpark = pSzi->spark;
			
			const unsigned short v1 = pSpark->getVB();

			pSpark->setIB(i*6);

			*pIndexData = v1;
			*++pIndexData = v1+1;
			*++pIndexData = v1+3;
			*++pIndexData = v1+3;
			*++pIndexData = v1+1;
			*++pIndexData = v1+2;
			++pIndexData;
			
			if(sp)
			{
				if((sp->su->textura != pSpark->su->textura || sp->su->blendflags != pSpark->su->blendflags))
				{
					sp = this->m_pPackList->New();

					sp->su = pSpark->su;
					sp->ib_start = pSpark->getIB();
					sp->scount = 0;
					sp->minVertex = v1;
					sp->maxVertex = v1+3;
				}
				if(v1 < sp->minVertex)
					sp->minVertex = v1;
				if(v1+3 > sp->maxVertex)
					sp->maxVertex = v1+3;
			}
			else
			{
				sp = this->m_pPackList->New();

				sp->su = pSpark->su;
				sp->ib_start = pSpark->getIB();
				sp->scount = 0;
				sp->minVertex = v1;
				sp->maxVertex = v1+3;
			}
			++sp->scount;
			++pSzi;
		}

		m_pIB->Unlock();

		d3dDevice->SetTransform(D3DTS_WORLD, &mIdentity); 
		d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE );
		d3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE );
		d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		d3dDevice->SetRenderState(D3DRS_CLIPPING, FALSE);
		d3dDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
		d3dDevice->SetFVF(PVERTEXFVF);
		d3dDevice->SetStreamSource(0,m_pVB,0,sizeof(SVERTEX));
		d3dDevice->SetIndices(m_pIB);

		d3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );

		dbgcounter = sizeof(SVERTEX)*VBLength;
		sp = this->m_pPackList->GetFirstActive();
		LPDIRECT3DTEXTURE9 tex = NULL;

		while(sp)
		{
			if(tex != sp->su->textura->Get()) {
				tex = sp->su->textura->Get();
				d3dDevice->SetTexture(0, tex);
			}

			if(sp->su->blendflags == 1 && blendflags != 1)
			{
				d3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
				blendflags = 1;
			}
			else if(sp->su->blendflags == 3 && blendflags != 3)
			{
				d3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
				blendflags = 3;
			}

			d3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
											0,
											sp->minVertex,
											(sp->maxVertex-sp->minVertex)+1, //TODO: 4*this->dsparks locka o vertex buffer inteiro, otimizar
											sp->ib_start,
											2*sp->scount);//this->IBLength);
			this->m_pPackList->Delete(&sp);
		}

		d3dDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
		d3dDevice->SetRenderState(D3DRS_CLIPPING, TRUE);
		d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE );
		d3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE );
		d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE );
	}
	return S_OK;
}

HRESULT ParticleSystem::CriarExplod(const Vector *pos, const DadosDoSpark *sd, CObjeto **shooter, Entity *binder)
{
	for(register unsigned int i = 0; i < sd->ninteracoes; ++i)
	{
		SparkUnit *su = sd->sparks[i];
		if(su->tipo == 0)
		{
			Vector dpos, dvel;
			dpos.x = su->min.x+randomf(su->max.x-su->min.x);
			dpos.y = su->min.y+randomf(su->max.y-su->min.y);
			dpos.z = su->min.z+randomf(su->max.z-su->min.z);

			dvel.x = su->minv.x+randomf(su->maxv.x-su->minv.x);
			dvel.y = su->minv.y+randomf(su->maxv.y-su->minv.y);
			dvel.z = su->minv.z+randomf(su->maxv.z-su->minv.z);

			dpos += *pos;
			this->CreateParticle(&dpos, &dvel, 1, su->alpha_vel, su->alpha_acel, su->minstartscale+randomf(su->maxstartscale-su->minstartscale), su->minscalevel+randomf(su->maxscalevel-su->minscalevel), su);
		}
		else
		{
			this->CriarEmissor(pos, su, binder);
		}
	}

	if(sd->raio && sd->dano) {
		CObjeto *obj = g_Unidades->GetFirstActive();

		while(obj) {
			if(obj->getDistanceSq(pos) <= sd->raio) {
				if(!equal(sd->kiloForca,0)) {
					real dist = obj->getDistanceSq(pos);
					real sraio = obj->dados->getRadiusSq()+sd->raio;
					if(dist <= sraio) {
						Vector normal = (*pos-*obj->GetPosition());
						Vector tmp; // Velocidade balanceada
						normal.normalize();
						normal = -normal;
				
						const Vector p = (normal*obj->dados->getRadius())+(*obj->GetPosition(true));
						const real penetrationDepth = sqrtf(sraio-dist);
						Vector force(0,0,0);
						const real c = 0.16, k = 0.03;
						const Vector pontoRelativo = p-(*obj->GetPosition(true));

						Vector penaltyForce = normal * (penetrationDepth * k);
						force += penaltyForce;

						force *= sd->kiloForca;

						obj->Impulse(&pontoRelativo, &force);
					}
				}
				obj->setDamage(sd->dano, shooter);
				/*if(obj->life <= 0) {
					if(shooter)
						if(*shooter)
							(*shooter)->ReacaoAoMatarInimigo();
				}*/
			}
			obj = g_Unidades->GetNext(obj);
		}
	}
	return S_OK;
}