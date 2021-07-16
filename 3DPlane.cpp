#include "stdafx.h"
#include "GameGlobals.h"
#include "GenericFunctions.h"
#include "3DPlane.h"
#include "CMesh.h"
#include "D3DObject.h"

HRESULT CPlane::OnLostDevice()
{
	SafeRelease(this->vb_RM);
	return S_OK;
}

HRESULT CPlane::OnResetDevice()
{
	SPLANEVERTEX *pVertex;
	HRESULT hr;

	hr = d3dDevice->CreateVertexBuffer(4*sizeof(SPLANEVERTEX), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, PLANEFVF, D3DPOOL_DEFAULT, &this->vb_RM, NULL);
	if(SUCCEEDED(hr)) {
		HRESULT hr = this->vb_RM->Lock(0,4*sizeof(SPLANEVERTEX), reinterpret_cast<void**>(&pVertex), D3DLOCK_DISCARD);
		if(SUCCEEDED(hr)) {
			D3DCOLOR cor;
			cor = static_cast<D3DCOLOR>((255 & 0xFF) << 24);
 			unsigned char r = static_cast<unsigned char>((255) & 0xFF);
			unsigned char g = static_cast<unsigned char>((255) & 0xFF);
			unsigned char b = static_cast<unsigned char>((255) & 0xFF);
			r *= mywrapper->light->light->Diffuse.r;
			g *= mywrapper->light->light->Diffuse.g;
			b *= mywrapper->light->light->Diffuse.b;
			cor |= r << 16;
			cor |= g << 8;
			cor |= b;

			pVertex[0].color = pVertex[1].color = pVertex[2].color = pVertex[3].color = cor;

			pVertex[1].pos.x = -0.5f*100000;
			pVertex[1].pos.y = 0;
			pVertex[1].pos.z = -0.5f*100000;

			pVertex[0].pos.x = 0.5f*100000;
			pVertex[0].pos.y = 0;
			pVertex[0].pos.z = -0.5f*100000;

			pVertex[3].pos.x = -0.5f*100000;
			pVertex[3].pos.y = 0;
			pVertex[3].pos.z = 0.5f*100000;

			pVertex[2].pos.x = 0.5f*100000;
			pVertex[2].pos.y = 0;
			pVertex[2].pos.z = 0.5f*100000;
			this->vb_RM->Unlock();
		}
		else
			RuntimeError("Falha ao trancar buffer de vértices para o plano %p!\nCódigo do Erro: %d", this, HRESULT_CODE(hr));
	}
	else
		RuntimeError("Falha ao criar buffer de vértices para o plano %p!\nCódigo do Erro: %d", this, HRESULT_CODE(hr));

	return hr;
}

CPlane::CPlane():vb_RM(NULL), CResource(CLASSE_VIDEO)
{
	this->Position(0, 0, 0);
	this->OnResetDevice();
	mywrapper->AddResource(this);
	this->Scale(1, 1, 1);
}

CPlane::~CPlane()
{
	SafeRelease(this->vb_RM);
}

HRESULT CPlane::Render(const CCamera *cam)
{
	SPLANEVERTEX* pVertexData; //pointer to vertex
	HRESULT hr;
	ASSERT(cam)
	D3DXMATRIX mScale, mRotation;
	//this->Position(cam->GetPositionX(), 0, cam->GetPositionZ());

	hr = this->vb_RM->Lock(0,4*sizeof(SPLANEVERTEX), reinterpret_cast<void**>(&pVertexData), D3DLOCK_DISCARD);
	if(SUCCEEDED(hr)) {
		const real dx = cam->GetPositionX()/this->GetScale()->x;
		const real dy = cam->GetPositionZ()/this->GetScale()->z;

		pVertexData[0].tex.x = 0-dx;
		pVertexData[0].tex.y = this->GetPositionZ()+0+dy;
		pVertexData[0].tex *= 30000;

		pVertexData[1].tex.x = 1-dx;
		pVertexData[1].tex.y = this->GetPositionZ()+0+dy;
		pVertexData[1].tex *= 30000;

		pVertexData[2].tex.x = 0-dx;
		pVertexData[2].tex.y = this->GetPositionZ()+1+dy;
		pVertexData[2].tex *= 30000;

		pVertexData[3].tex.x = 1-dx;
		pVertexData[3].tex.y = this->GetPositionZ()+1+dy;
		pVertexData[3].tex *= 30000;

		this->vb_RM->Unlock();
	}

	D3DXMatrixTranslation( &mWorld, cam->GetPositionX(), this->GetPositionY(), cam->GetPositionZ() );
    D3DXMatrixScaling( &mScale, this->scale.x, this->scale.y, this->scale.z );
	this->currentState->quat.rotationMatrix(&mRotation);
	//D3DXMatrixRotationQuaternion(&this->mRotation, &this->qQuat);

	D3DXMatrixMultiply( &mRotation, &mRotation, &mScale);
	D3DXMatrixMultiply( &mWorld, &mRotation, &mWorld );

	//hr = d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	if(SUCCEEDED(hr)) {
		
		hr = d3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
		hr = d3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
		if(SUCCEEDED(hr)) {
			hr = d3dDevice->SetTransform(D3DTS_WORLD, &this->mWorld); 
			if(SUCCEEDED(hr)) {
				hr = d3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE );
				if(SUCCEEDED(hr)) {
					d3dDevice->SetFVF(PLANEFVF);
					d3dDevice->SetStreamSource(0,this->vb_RM,0,sizeof(SPLANEVERTEX));
					d3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
					d3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE );
					d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
				}
			}
		}
	}
	return hr;
}