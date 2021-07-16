#ifndef _GAMEHUD_H
#define _GAMEHUD_H

#pragma once

#include "3DTexture.h"
#include "InlineList.h"
#include "GameGlobals.h"
#include "D3DObject.h"

using Matematica::real;
using Matematica::r;

class MATRIX2D {
public:
	real elements[2][2];

	MATRIX2D()
	{ }

	MATRIX2D(const real radians)
	{
		real const CosAngle = cos(radians);
		real const SinAngle = sin(radians);

		this->elements[0][0] = CosAngle;
		this->elements[0][1] = -SinAngle;
		this->elements[1][0] = SinAngle;
		this->elements[1][1] = CosAngle;
	}
};

struct VECTOR2D
{
	real x, y;

	VECTOR2D()
	{ }

	VECTOR2D(const VECTOR2D *vec)
	{
		this->x = vec->x;
		this->y = vec->y;
	}

	VECTOR2D(const VECTOR2D &vec)
	{
		this->x = vec.x;
		this->y = vec.y;
	}
	
	VECTOR2D(const real x, const real y):x(x), y(y)
	{ }

	inline VECTOR2D &operator+=(const VECTOR2D &b)
	{
		this->x += b.x;
		this->y += b.y;
		return *this;
	}

	inline VECTOR2D &operator*=(const MATRIX2D &m)
	{
		real tx = this->x, ty = this->y;
		this->x = m.elements[0][0] * tx + m.elements[0][1] * ty;
		this->y = m.elements[1][0] * tx + m.elements[1][1] * ty;

		return *this;
	}

	inline VECTOR2D operator*(const MATRIX2D &m) const
	{
		VECTOR2D vTmp(*this);

		return vTmp *= m;
	}
};

extern const VECTOR2D g_sQuad[4];
extern const VECTOR2D g_sQuad2[4];

struct TScreenVertex
{
	VECTOR2D coord;
	float z;
	float rhw;
	D3DCOLOR color;
	float u, v;
};
#define SCREEN_SPACE_FVF (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)

class GameHUD : public CResource, public VideoResource {
	struct Quad {
		VECTOR2D coord, hotspot;
		VECTOR2D scale;
		real u, v, angle;
		D3DCOLOR color;
		const CTexture *tex;
		unsigned int flags;
	};

	LPDIRECT3DVERTEXBUFFER9 m_pVB;
	LPDIRECT3DINDEXBUFFER9  m_pIB;
	size_t VBLength, VBUsed, IBLength, IBUsed;
	TScreenVertex *tsv;
	CSimpleListAP<Quad> quadlist;
	unsigned int drawmode, DesiredLength;

	void CreateQuad(const real x, const real y, const real w, const real h, const D3DCOLOR color, const CTexture *tex = NULL, const real u = 1, const real v = 1, const real angle = 0, const VECTOR2D hotspot = VECTOR2D(0, 0))
	{
		Quad *myquad = quadlist.New()->GetPtr();
		
		if((drawmode & F_CENTER) == F_CENTER) {
			myquad->hotspot.x = -(w*r(0.5));
			myquad->hotspot.y = -(h*r(0.5));
			myquad->hotspot += hotspot;
		} else {
			myquad->hotspot = hotspot;
		}
		myquad->coord.x = x;
		myquad->coord.y = y;
		myquad->scale.x = w;
		myquad->scale.y = h;
		myquad->u = u;
		myquad->v = v;
		myquad->color = color;
		myquad->flags = this->drawmode;
		myquad->angle = angle;
		if(tex) {
			myquad->tex = tex;
			myquad->flags |= F_TEXTURIZED;
		} else {
			myquad->flags |= F_FILLQUAD;
		}
	}

public:

	enum MODES {
		F_FILLQUAD = 0x00,
		F_TEXTURIZED = 0x01,
		F_ALPHABLEND = 0x04,
		F_ALPHABLENDTRANS = 0x04,
		F_ALPHABLENDADD = 0x06,
		F_CENTER = 0x08,
		F_INVU = 0x10,
		F_INVV = 0x20
	};

	GameHUD():m_pVB(NULL), VBLength(0) ,m_pIB(NULL), IBLength(0), VBUsed(0), drawmode(0), CResource(CLASSE_VIDEO)
	{
	}

	~GameHUD()
	{
		this->Cleanup();
	}

	HRESULT Cleanup()
	{
		SafeRelease(this->m_pIB);
		SafeRelease(this->m_pVB);
		this->VBLength = this->IBLength = this->IBUsed = this->VBUsed = 0;
		return S_OK;
	}

	HRESULT OnLostDevice()
	{ return this->Cleanup(); }

	HRESULT OnResetDevice()
	{
		HRESULT hr;
		
		hr = d3dDevice->CreateVertexBuffer(this->DesiredLength*sizeof(TScreenVertex), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, SCREEN_SPACE_FVF, D3DPOOL_DEFAULT, &m_pVB, NULL);
		if(SUCCEEDED(hr)) {
			VBLength = this->DesiredLength;
			hr = d3dDevice->CreateIndexBuffer(4*sizeof(short), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &this->m_pIB, NULL);
			if(SUCCEEDED(hr)) {
				IBLength = 4;
				short *pShort = NULL;
				hr = this->m_pIB->Lock(0, 0, reinterpret_cast<VOID**>(&pShort), 0);
				if(SUCCEEDED(hr)) {
					*pShort = 0;
					*++pShort = 1;
					*++pShort = 2;
					*++pShort = 3;
					this->m_pIB->Unlock();
				} else
					Cleanup();
			}
			else
				Cleanup();
		}
		return hr;
	}

	HRESULT Setup(const UINT Length)
	{
		HRESULT hr;
		this->DesiredLength = Length;
		hr = this->OnLostDevice();
		if(SUCCEEDED(hr)) {
			hr = this->OnResetDevice();
			if(SUCCEEDED(hr))
				mywrapper->AddResource(this);
		}
		return hr;
	}

	void SetDrawMode(const unsigned int mode)
	{ this->drawmode = mode; }

	void RenderQuad(const real x, const real y, const real w, const real h, const D3DCOLOR color)
	{
		this->CreateQuad(x, y, w, h, color);
		this->VBUsed += 4;
	}

	void RenderQuad(const real x, const real y, const real w, const real h, const CTexture *tex, const D3DCOLOR color = 0xFFFFFFFF)
	{
		if(tex) {
			this->CreateQuad(x, y, w, h, color, tex);
			this->VBUsed += 4;
		}
	}

	void RenderQuad(const real x, const real y, const real size, const CTexture *tex, const real angle, const real hotx, const real hoty,const D3DCOLOR color = 0xFFFFFFFF)
	{
		if(tex) {
			this->CreateQuad(x, y, tex->GetWidth()*size, tex->GetHeight()*size, color, tex, 1, 1, angle, VECTOR2D(hotx, hoty));
			this->VBUsed += 4;
		}
	}

	void RenderQuad(const real x, const real y, const real size, const CTexture *tex, const D3DCOLOR color = 0xFFFFFFFF)
	{
		if(tex) {
			this->CreateQuad(x, y, tex->GetWidth()*size, tex->GetHeight()*size, color, tex);
			this->VBUsed += 4;
		}
	}

	void RenderQuad(const real x, const real y, const CTexture *tex, const D3DCOLOR color = 0xFFFFFFFF)
	{
		if(tex) {
			this->CreateQuad(x, y, static_cast<const real>(tex->GetWidth()), static_cast<const real>(tex->GetHeight()), color, tex);
			this->VBUsed += 4;
		}
	}

	void RenderQuad(const real x, const real y, const CTexture *tex, const real u, const real v, const D3DCOLOR color = 0xFFFFFFFF)
	{
		if(tex) {
			this->CreateQuad(x, y, tex->GetWidth()*u, tex->GetHeight()*v, color, tex, u, v);
			this->VBUsed += 4;
		}
	}

	inline void WriteVertexRotated(Quad *q, TScreenVertex *tsv)
	{
		ASSERT(q && tsv)
		//VECTOR2D New[4];
		MATRIX2D matrix(q->angle);

		/*for(register unsigned int a = 0; a < 4; ++a) {
			VECTOR2D vTmp = g_sQuad2[a];
			//if((q->flags & F_CENTER) == F_CENTER) {
			//	vTmp.x -= r(0.5);
			//	vTmp.y -= r(0.5);
			//}
			New[a].x = fCosTheta * vTmp.x - fSinTheta * vTmp.y;
			New[a].y = fSinTheta * vTmp.x + fCosTheta * vTmp.y;
		}*/

		tsv->coord.x = 0;
		tsv->coord.y = 0;
		tsv->coord += q->hotspot;
		tsv->z = 0;

		tsv->coord *= matrix;

		tsv->coord += q->coord;
		tsv->color = q->color;
		tsv->rhw = 1;

		if(q->flags & F_TEXTURIZED) {
			tsv->u = (q->flags & F_INVU) ? q->u : 0;
			tsv->v = (q->flags & F_INVV) ? q->v : 0;
		}

		++tsv;

		tsv->coord.x = q->scale.x;
		tsv->coord.y = 0;
		tsv->coord += q->hotspot;
		tsv->z = 0;

		tsv->coord *= matrix;

		tsv->coord += q->coord;
		tsv->color = q->color;
		tsv->rhw = 1;

		if(q->flags & F_TEXTURIZED) {
			tsv->u = (q->flags & F_INVU) ? 0 : q->u;
			tsv->v = (q->flags & F_INVV) ? q->v : 0;
		}

		++tsv;

		tsv->coord.x = 0;
		tsv->coord.y = q->scale.y;
		tsv->coord += q->hotspot;
		tsv->z = 0;

		tsv->coord *= matrix;

		tsv->coord += q->coord;
		tsv->color = q->color;
		tsv->rhw = 1;

		if(q->flags & F_TEXTURIZED) {
			tsv->u = (q->flags & F_INVU) ? q->u : 0;
			tsv->v = (q->flags & F_INVV) ? 0 : q->v;
		}

		++tsv;

		tsv->coord.x = q->scale.x;
		tsv->coord.y = q->scale.y;
		tsv->coord += q->hotspot;
		tsv->z = 0;

		tsv->coord *= matrix;

		tsv->coord += q->coord;
		tsv->color = q->color;
		tsv->rhw = 1;

		if(q->flags & F_TEXTURIZED) {
			tsv->u = (q->flags & F_INVU) ? 0 : q->u;
			tsv->v = (q->flags & F_INVV) ? 0 : q->v;
		}

		++tsv;
	}

	inline void WriteVertex(Quad *q, TScreenVertex *tsv)
	{
		ASSERT(q && tsv)

		tsv->coord = q->coord;
		tsv->coord += q->hotspot;
		tsv->z = 0;
		tsv->color = q->color;
		tsv->rhw = 1;

		if(q->flags & F_TEXTURIZED) {
			tsv->u = tsv->v = 0;
		}

		++tsv;

		tsv->coord = q->coord;
		tsv->coord += q->hotspot;
		tsv->coord.x += q->scale.x;
		tsv->z = 0;
		tsv->color = q->color;
		tsv->rhw = 1;

		if(q->flags & F_TEXTURIZED) {
			tsv->u = q->u;
			tsv->v = 0;
		}

		++tsv;

		tsv->coord = q->coord;
		tsv->coord += q->hotspot;
		tsv->coord.y += q->scale.y;
		tsv->z = 0;
		tsv->color = q->color;
		tsv->rhw = 1;

		if(q->flags & F_TEXTURIZED) {
			tsv->u = 0;
			tsv->v = q->v;
		}

		++tsv;

		tsv->coord = q->coord;
		tsv->coord += q->hotspot;
		tsv->coord += q->scale;
		tsv->z = 0;
		tsv->color = q->color;
		tsv->rhw = 1;

		if(q->flags & F_TEXTURIZED) {
			tsv->u = q->u;
			tsv->v = q->v;
		}

		++tsv;
	}

	HRESULT Render()
	{
		HRESULT hr;
		TScreenVertex *tsv;
		
		if(this->VBUsed > this->VBLength) {
			if(this->m_pVB) {
				this->m_pVB->Release(); 
				this->m_pVB = NULL;
				this->VBLength = 0;
			}
			hr = d3dDevice->CreateVertexBuffer(this->VBUsed*sizeof(TScreenVertex), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, SCREEN_SPACE_FVF, D3DPOOL_DEFAULT, &m_pVB, NULL);
			if(FAILED(hr))
				return hr;
		}

		hr = this->m_pVB->Lock(0, this->VBUsed*sizeof(TScreenVertex), reinterpret_cast<LPVOID*>(&tsv), D3DLOCK_DISCARD);
		if(SUCCEEDED(hr)) {

			CSimpleNode<Quad> *q = quadlist.GetFirstActive();
			while(q) {
				if(equal(q->GetPtr()->angle, 0))
					WriteVertex(q->GetPtr(), tsv);
				else
					WriteVertexRotated(q->GetPtr(), tsv);
				tsv += 4;

				q = q->GetNext();
			}
		
			this->m_pVB->Unlock();

			d3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
			d3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

			d3dDevice->SetFVF(SCREEN_SPACE_FVF);
			d3dDevice->SetStreamSource(0,this->m_pVB,0,sizeof(TScreenVertex));
			d3dDevice->SetIndices(this->m_pIB);
			q = quadlist.GetFirstActive();
			d3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );

			register unsigned int a = 0;
			while(q) {
				Quad *quad = q->GetPtr();
				if((quad->flags & F_TEXTURIZED) == F_TEXTURIZED) {
					d3dDevice->SetTexture(0, quad->tex->Get());
				} else {
					d3dDevice->SetTexture(0, NULL);
				}
				if(quad->flags & F_ALPHABLEND) {
					d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
					if((quad->flags & F_ALPHABLENDADD) == F_ALPHABLENDADD)
						d3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
					else
						d3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );		
				} else
					d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
				d3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, a*4, 0, 4, 0, 2);
				quadlist.Delete(&q);
				++a;
			}
			d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
			d3dDevice->SetRenderState(D3DRS_ZENABLE, zbuffer );
			d3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE );

			this->VBUsed = 0;
		}
		//d3dDevice->SetTextureStageState(0,D3DTSS_COLORARG1, D3DTA_TEXTURE );
		//d3dDevice->SetTextureStageState(0,D3DTSS_COLORARG2, D3DTA_DIFFUSE );
		return hr;
	}
};

#endif