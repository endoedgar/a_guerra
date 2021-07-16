#ifndef _BANDEIRA_H
#define _BANDEIRA_H

#include "D3DObject.h"

#pragma once

typedef struct tagBVERTEX
{
	Vector coord, normal;
	float tu, tv;
} BVERTEX;

const DWORD BVERTEX_FVF = (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1);

class BandeiraMesh : public CResource, public VideoResource
{
	class Tri {
	public:
		unsigned short a, b, c;

		Tri():a(0), b(0), c(0)
		{ }
	};

	D3DMATERIAL9 Bmat;

	LPDIRECT3DVERTEXBUFFER9 m_pVB;
	LPDIRECT3DINDEXBUFFER9  m_pIB;
	unsigned int nIndices, width, height, numVertsX, numVertsY;
	real currenttime, tmul;
	Vector influences;
	bool IsOK;
	Smart::Array<BVERTEX> m_pVertices;
	Smart::Array<Smart::Array<Tri>> m_pTri;

public:
	BandeiraMesh():m_pVB(NULL), m_pIB(NULL), IsOK(false), nIndices(0), currenttime(0), tmul(1), influences(1, 1, 1), CResource(CLASSE_VIDEO)
	{
		Bmat.Diffuse.r = 1.0f;
		Bmat.Diffuse.g = 1.0f;
		Bmat.Diffuse.b = 1.0f;
		Bmat.Diffuse.a = 1.0f;

		Bmat.Ambient.r = 1.0f;
		Bmat.Ambient.g = 1.0f;
		Bmat.Ambient.b = 1.0f;
		Bmat.Ambient.a = 1.0f;

		Bmat.Specular.r = 0.0f;
		Bmat.Specular.g = 0.0f;
		Bmat.Specular.b = 0.0f;
		Bmat.Specular.a = 0.0f;
		Bmat.Power = 0.0f;

		Bmat.Emissive.r = 0.2f;
		Bmat.Emissive.g = 0.2f;
		Bmat.Emissive.b = 0.2f;
		Bmat.Emissive.a = 0.2f;
	}

	inline void SetXFactor(const real x)
	{ this->influences.x = x; }

	inline void SetYFactor(const real y)
	{ this->influences.y = y; }

	inline void SetZFactor(const real z)
	{ this->influences.z = z; }

	inline void SetTFactor(const real t)
	{ this->tmul = t; }

	inline void SetAllFactors(const real x, const real y, const real z, const real t)
	{
		SetXFactor(x);
		SetYFactor(y);
		SetZFactor(z);
		SetTFactor(t);
	}

	HRESULT OnLostDevice()
	{
		SafeRelease(this->m_pVB);
		SafeRelease(this->m_pIB);
		return S_OK;
	}

	HRESULT OnResetDevice()
	{
		HRESULT hr;
		this->OnLostDevice();
		hr = d3dDevice->CreateVertexBuffer(this->m_pVertices.GetMemSize(), D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC | D3DUSAGE_SOFTWAREPROCESSING, BVERTEX_FVF, D3DPOOL_DEFAULT, &this->m_pVB, NULL);
		if(SUCCEEDED(hr)) {
			hr = d3dDevice->CreateIndexBuffer((this->nIndices)*sizeof(short), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &this->m_pIB, NULL);
			if(SUCCEEDED(hr)) {
				unsigned short *pIndice = NULL;
				size_t count=0;
				hr = this->m_pIB->Lock(0, 0, reinterpret_cast<VOID**>(&pIndice), 0);
				if(SUCCEEDED(hr)) {
					for(register size_t x = 0; x < this->m_pTri.GetCapacity(); ++x) {
						for(register size_t y = 0; y < this->m_pTri[y].GetCapacity(); ++y) {
							pIndice[count++] = this->m_pTri[x][y].a;
							pIndice[count++] = this->m_pTri[x][y].b;
							pIndice[count++] = this->m_pTri[x][y].c;
						}
					}
					this->m_pIB->Unlock();
				}
			}
		}
		return hr;
	}

	HRESULT Cleanup()
	{
		this->m_pVertices.DestroyData();
		this->OnLostDevice();
		this->IsOK = false;
		this->currenttime = 0;
		this->nIndices = this->width = this->height = this->numVertsX = this->numVertsY = 0;
		return S_OK;
	}

	HRESULT Setup(const unsigned int width, const unsigned int height)
	{
		HRESULT hr = E_FAIL;
		this->Cleanup();

		const real norm = r(1.0/max(width, height));
		const real stepX=r(1.0/(width*(height*norm)));
		const real stepY=r(1.0/(height*(width*norm)));

		if(this->m_pVertices.SetSize((width+1)*(height+1))) {
			this->width = width;
			this->height = height;
			this->numVertsX=this->width+1;
			this->numVertsY=this->height+1;

			this->m_pTri.SetSize(width*2);

			for(register size_t a = 0; a < m_pTri.GetCapacity(); ++a) {
				this->m_pTri[a].SetSize(height);
			}

			Vector pos(0, 0, 0);

			size_t count=0;

			for (register size_t y=0;y<numVertsY;y++)
			{
				BVERTEX *myVertex;
				pos.x=0;
				for (register size_t x=0;x<numVertsX;x++)
				{
					myVertex = &this->m_pVertices[count];
					myVertex->coord=pos;
					myVertex->tu = r(x)/(numVertsX-1);
					myVertex->tv = r(y)/(numVertsY-1);

					pos.x-=stepX;
					count++;
				}

				pos.y-=stepY;
			}

			int vIndex=0;

			for(register size_t y = 0; y < height; ++y) {
				for(register size_t x = 0; x < width*2; x+=2) {
					this->m_pTri[x][y].a = static_cast<unsigned short>(vIndex);
					this->m_pTri[x][y].b = static_cast<unsigned short>(vIndex+numVertsX+1);
					this->m_pTri[x][y].c = static_cast<unsigned short>(vIndex+numVertsX);
					// .
					// |\
					// .-.

					this->m_pTri[x+1][y].a = static_cast<unsigned short>(vIndex);
					this->m_pTri[x+1][y].b = static_cast<unsigned short>(vIndex+1);
					this->m_pTri[x+1][y].c = static_cast<unsigned short>(vIndex+numVertsX+1);

					// .-.
					//  \.

					vIndex++;
				}
				vIndex++;
			}

			this->nIndices = (6*width*height);

			hr = this->OnResetDevice();
			if(SUCCEEDED(hr)) {
				mywrapper->AddResource(this);
				this->IsOK = true;
			} else
				this->Cleanup();
		} else
			this->Cleanup();
		return hr;
	}

	HRESULT Update(const real dtime)
	{
		this->currenttime += dtime;
		return S_OK;
	}

	HRESULT CalcNormals()
	{
		for(register size_t a = 0; a < this->m_pVertices.GetCapacity(); ++a) {
			Vector &n = this->m_pVertices[a].normal;
			n.clear();

			for(register size_t x = 0; x < this->m_pTri.GetCapacity(); ++x) {
				for(register size_t y = 0; y < this->m_pTri[x].GetCapacity(); ++y) {
					Tri *tri = &this->m_pTri[x][y];
					if(tri->a == a || tri->b == a || tri->c == a) {
						Vector normal;
						normal = (this->m_pVertices[tri->c].coord-this->m_pVertices[tri->a].coord).cross(this->m_pVertices[tri->b].coord-this->m_pVertices[tri->a].coord);
						n += normal;
					}			
				}
			}

			n.normalize();
		}
		return S_OK;
	}

	HRESULT Render()
	{
		HRESULT hr = S_FALSE;
		BVERTEX *myVertex = NULL;
		const real angle = currenttime * tmul;
		if(this->IsOK) {
			myVertex = &this->m_pVertices;
			for(register unsigned int a = 0; a < this->m_pVertices.GetCapacity(); a++) {
				myVertex->coord.z = sin(myVertex->coord.x*influences.x+angle);
				myVertex->coord.z += sin(myVertex->coord.y*influences.y+angle);
				myVertex->coord.z *= myVertex->coord.x * influences.z;
				++myVertex;
			}
			hr = this->m_pVB->Lock(0, 0, reinterpret_cast<VOID**>(&myVertex), D3DLOCK_DISCARD);
			if(SUCCEEDED(hr)) {
				memcpy(myVertex, &this->m_pVertices, this->m_pVertices.GetMemSize()); 
				this->m_pVB->Unlock();
				d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
				d3dDevice->SetFVF(BVERTEX_FVF);
				d3dDevice->SetMaterial(&this->Bmat);
				d3dDevice->SetIndices(this->m_pIB);
				d3dDevice->SetStreamSource(0, this->m_pVB, 0, sizeof(BVERTEX));
				d3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, this->m_pVertices.GetCapacity(), 0, this->nIndices/3); 
				d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
			}
		}
		return hr;
	}

	~BandeiraMesh()
	{
		if(this->m_pVB)
			this->m_pVB->Release();
		if(this->m_pIB)
			this->m_pIB->Release();
	}
};

extern BandeiraMesh *bmesh;

#endif