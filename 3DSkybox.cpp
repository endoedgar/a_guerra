#include "stdafx.h"
#include "3DSkybox.h"
#include "GameGlobals.h"
#include "globalheader.h"

CSkybox::CSkybox():skytex(NULL), VB(NULL), IB(NULL), CResource(CLASSE_VIDEO)
{
	TRACE("Skybox %p criado!\n", this);
}

void AddVertex(SBVERTEX *vd, float x, float y, float z, float u, float v)
{
	vd->coord.x = x;
	vd->coord.y = y;
	vd->coord.z = z;
	vd->tu = u;
	vd->tv = v;
}

HRESULT CSkybox::RotinaRepetitiva(const Smart::String &filename, const Smart::String &sufix, const Smart::String &end, CTexture **skytex)
{
	Smart::String helper;

	helper = "skybox\\";
	helper += filename;
	helper += sufix;
	helper += '.';
	helper += end;
	*skytex = (*skytex)->Load(helper);
	if(!(*skytex))
		return E_FAIL;

	return S_OK;
}

HRESULT CSkybox::Load(const Smart::String &filename, const Smart::String &end)
{
	TRACE("Carregando Skybox %s...\n", filename.c_str());

	this->UnloadTexture();
	if(!this->skytex)
		this->skytex = new CTexture*[5];

	this->OnResetDevice();

	if(FAILED(RotinaRepetitiva(filename, "_frente", end, this->skytex)))
		return E_FAIL;
	if(FAILED(RotinaRepetitiva(filename, "_direita", end, this->skytex+1))) 
		return E_FAIL;
	if(FAILED(RotinaRepetitiva(filename, "_tras", end, this->skytex+2))) 
		return E_FAIL;
	if(FAILED(RotinaRepetitiva(filename, "_esquerda", end, this->skytex+3)))
		return E_FAIL;
	if(FAILED(RotinaRepetitiva(filename, "_topo", end, this->skytex+4))) 
		return E_FAIL;

	TRACE("\tSkybox carregado com sucesso!\n");

	mywrapper->AddResource(this);

	return S_OK;
}

HRESULT CSkybox::OnLostDevice()
{
	SafeRelease(this->VB);
	SafeRelease(this->IB);
	return S_OK;
}

HRESULT CSkybox::OnResetDevice()
{
	HRESULT hr = S_OK;
	static const float skysize = 800;
	if(!this->IB)
	{
		short* pIndexData; //pointer to index
		hr = d3dDevice->CreateIndexBuffer((sizeof(short)*4), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &this->IB, NULL);
		if(SUCCEEDED(hr)) {
			hr = this->IB->Lock(0,0,reinterpret_cast<void**>(&pIndexData),0);
			if(SUCCEEDED(hr)) {
				*pIndexData = 2;
				*++pIndexData = 1;
				*++pIndexData = 3;
				*++pIndexData = 0;

				this->IB->Unlock();
			}
			else {
				TRACE("Falha ao trancar buffer de índices! (Código do Erro: %d)\n", HRESULT_CODE(hr));
			}
		}
		else {
			TRACE("\tFalha ao criar Buffer de Índices! (Código do Erro: %d)\n", HRESULT_CODE(hr));
			return hr;
		}
	}
	if(!this->VB)
	{
		SBVERTEX* pVertexData, *vd; //pointer to vertex
		hr = d3dDevice->CreateVertexBuffer((4*sizeof(SBVERTEX))*5, D3DUSAGE_WRITEONLY, SBVERTEXFVF, D3DPOOL_DEFAULT, &this->VB, NULL);
		if(SUCCEEDED(hr)) {
			this->VB->Lock(0,0,reinterpret_cast<void**>(&pVertexData),0);

			for(register unsigned int a = 0; a < 5; ++a) {
				vd = &pVertexData[a*4];
				switch(a)
				{
					case 0:
						AddVertex(vd, -skysize, skysize, -skysize, 0, 0);
						AddVertex(++vd, skysize, skysize, -skysize, 1, 0);
						AddVertex(++vd, skysize, -skysize, -skysize, 1, 1);
						AddVertex(++vd, -skysize, -skysize, -skysize, 0, 1);

						break;
					case 1:
						AddVertex(vd, skysize, skysize, -skysize, 0, 0);
						AddVertex(++vd, skysize, skysize, skysize, 1, 0);
						AddVertex(++vd, skysize, -skysize, skysize, 1, 1);
						AddVertex(++vd, skysize, -skysize, -skysize, 0, 1);

						break;
					case 2:
						AddVertex(vd, skysize, skysize, skysize, 0, 0);
						AddVertex(++vd, -skysize, skysize, skysize, 1, 0);
						AddVertex(++vd, -skysize, -skysize, skysize, 1, 1);
						AddVertex(++vd, skysize, -skysize, skysize, 0, 1);

						break;
					case 3:
						AddVertex(vd, -skysize, skysize, skysize, 0, 0);
						AddVertex(++vd, -skysize, skysize, -skysize, 1, 0);
						AddVertex(++vd, -skysize, -skysize, -skysize, 1, 1);
						AddVertex(++vd, -skysize, -skysize, skysize, 0, 1);

						break;
					case 4:
						AddVertex(vd, -skysize, skysize, skysize, 0, 1);
						AddVertex(++vd, skysize, skysize, skysize, 0, 0);
						AddVertex(++vd, skysize, skysize, -skysize, 1, 0);
						AddVertex(++vd, -skysize, skysize, -skysize, 1, 1);

						break;
				}
			}

			this->VB->Unlock();
		} else {
			TRACE("\tFalha ao criar Buffer de Vértices! (Código do Erro: %d)\n", HRESULT_CODE(hr));
			return hr;
		}
	}
	return hr;
}

HRESULT CSkybox::Render(const CCamera *camera)
{
	D3DXMATRIX matView;
	const D3DXMATRIX *matViewSave;
	HRESULT hr = S_OK;

	matViewSave = camera->GetMatrixView();
	matView = *matViewSave;
	matView._41 = 0.0f; matView._42 = 0.0f; matView._43 = 0.0f;

	d3dDevice->SetTransform( D3DTS_VIEW, &matView );
	d3dDevice->SetTransform( D3DTS_WORLD, &mIdentity );

	d3dDevice->SetRenderState( D3DRS_ZENABLE, FALSE );
	d3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
	d3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );

	d3dDevice->SetFVF(SBVERTEXFVF);
	d3dDevice->SetStreamSource(0,this->VB,0,sizeof(SBVERTEX));
	d3dDevice->SetIndices(this->IB);

	// TODO: Usar 1 textura
	for(register unsigned int a = 0; a < 5; ++a)
	{
		d3dDevice->SetTexture(0, this->skytex[a]->Get());
		hr = d3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP,
										a*4,
										0,
										4,
										0,
										2);
		if(FAILED(hr))
			return hr;
	}

	d3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
	d3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
	d3dDevice->SetRenderState( D3DRS_ZENABLE, zbuffer );

	return d3dDevice->SetTransform(D3DTS_VIEW, matViewSave);
}

void CSkybox::UnloadTexture()
{
	if(this->skytex)
	{
		TRACE("Liberando texturas do skybox %p...\n", this);
		for(register unsigned int a = 0; a < 5; ++a)
			SafeRelease(this->skytex[a]);
		delete [] this->skytex;
	}
	this->skytex = NULL;
}

CSkybox::~CSkybox()
{
	this->UnloadTexture();
	SafeRelease(this->IB);
	SafeRelease(this->VB);
	delete this->skytex;
	TRACE("Skybox %p liberado!\n", this);
}