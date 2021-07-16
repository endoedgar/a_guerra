#include "stdafx.h"
#include "globalheader.h"
#include "GameGlobals.h"
#include "LineRender.h"
#include "D3DObject.h"

LineRender::LineRender(void):CResource(CLASSE_VIDEO), m_pVB(NULL), VBLength(0)
{
	mywrapper->AddResource(this);
}

void LineRender::DrawLine(const Vector &start, const Vector &end, const D3DCOLOR cor)
{
	Line l;
	l.color = cor;
	l.end = end;
	l.start = start;
	this->lines.push(l);
}

HRESULT LineRender::Render()
{
	HRESULT hr = S_OK;
	const unsigned int nLines = this->lines.size();
	if(nLines) {
		unsigned int currentVD = 0;
		const unsigned int vertices = nLines*2;
		LVERTEX* pVertexData;
		LVERTEX *vd;

		if(this->VBLength < vertices) {
			this->VBLength = vertices;
			hr = d3dDevice->CreateVertexBuffer(this->VBLength*sizeof(LVERTEX), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, LVERTEXFVF, D3DPOOL_DEFAULT, &this->m_pVB, NULL);
			if(FAILED(hr)) {
				this->VBLength = 0;
				return hr;
			}
		}

		hr = this->m_pVB->Lock(0,nLines*sizeof(LVERTEX), reinterpret_cast<void**>(&pVertexData), D3DLOCK_DISCARD);
		
		if(SUCCEEDED(hr)) {
			vd = pVertexData;
			while(!this->lines.empty()) {
				Line &l = this->lines.top();

				vd->color = l.color;
				vd->coord = Vector(0, 0, 0);
				++vd;

				vd->color = l.color;
				vd->coord = Vector(0, 0, 100);
				++vd;

				this->lines.pop();
				currentVD+=2;
			}
			this->m_pVB->Unlock();
			DWORD lighting;

			d3dDevice->GetRenderState(D3DRS_LIGHTING, &lighting);

			hr = d3dDevice->SetTransform(D3DTS_WORLD, &mIdentity);
			if(SUCCEEDED(hr)) {
				hr = d3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
				if(SUCCEEDED(hr)) {
					hr = d3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
					if(SUCCEEDED(hr)) {
						hr = d3dDevice->SetStreamSource(0, this->m_pVB, 0, sizeof(LVERTEX));
						if(SUCCEEDED(hr)) {
							hr = d3dDevice->DrawPrimitive(D3DPT_LINELIST, 0, nLines);
							if(SUCCEEDED(hr)) {
								d3dDevice->SetRenderState(D3DRS_LIGHTING, lighting);
								d3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
							}
						}
					}
				}
			}
		}
	}
	return hr;
}

HRESULT LineRender::OnResetDevice()
{ return S_OK; }

HRESULT LineRender::OnLostDevice()
{
	SafeRelease(this->m_pVB);
	this->VBLength = 0;
	return S_OK;
}

LineRender::~LineRender(void)
{
	SafeRelease(m_pVB);
}
