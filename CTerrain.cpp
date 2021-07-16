#include "stdafx.h"
#include "GameGlobals.h"
#include "Jogador.h"
#include "3DCamera.h"
#include "CTerrain.h"
#include "diamondAlgorithm.h"
#include "globalheader.h"
#include "Segment.h"

CTerrain::CTerrain():width(0), height(0), lasttri(0), lod(NULL), m_pTextura(NULL), m_pTexturaDetalhe(NULL)
{
}

bool CTerrain::segmentIntersect(real &frac, Vector &pos, Vector &normal, const Segment &seg) const {
	if(seg.mDelta.z > -SCALAR_TINY)
		return false;

	Vector normalStart;
	real heightStart;
	GetPointInformation(&seg.mOrigin, heightStart, normalStart);
	normalStart.normalize();

	if(heightStart < 0)
		return false;

	Vector normalEnd;
	real heightEnd;
	Vector end = seg.getEnd();
	GetPointInformation(&end, heightEnd, normalEnd);
	normalEnd.normalize();

	if(heightEnd > 0)
		return false;

	real depthEnd = -heightEnd;

	real weightStart = 1 / (SCALAR_TINY + heightStart);
	real weightEnd = 1 / (SCALAR_TINY + depthEnd);

	normal = (normalStart * weightStart + normalEnd * weightEnd) / (weightStart+weightEnd);

	frac = heightStart / (heightStart + depthEnd + SCALAR_TINY);

	pos = seg.getPoint(frac);

	return false;
}

void CTerrain::CalcNormals()
{
	register size_t count = 0;
	const real oneoverwidth = 1.0f/this->width;
	const real oneoverheight = 1.0f/this->height;
	const real oneovery = 1.0f/255;
	PROFILE_START;
	for(register size_t y = 0; y < this->height; y++) {
		for(register size_t x = 0; x < this->width; x+=2)
		{
			Vector v0, v1, v2;
			Tri &first = triNormals[x+(y*this->width)];
			Tri &second = triNormals[(x+1)+(y*this->width)];

			v0.x = x*oneoverwidth;
			v0.y = tdata[x][y]*oneovery;
			v0.z = y*oneoverheight;

			v1.x = (x+1)*oneoverwidth;
			v1.y = tdata[x+1][y+1]*oneovery;
			v1.z = (y+1)*oneoverheight;

			v2.x = x*oneoverwidth;
			v2.y = tdata[x][y+1]*oneovery;
			v2.z = (y+1)*oneoverheight;

			first = Tri(v0, v1, v2);
			first.SetNormal((v1-v0).cross(v2-v0));
			
			v0.x = x*oneoverwidth;
			v0.y = tdata[x][y]*oneovery;
			v0.z = y*oneoverheight;

			v1.x = (x+1)*oneoverwidth;
			v1.y = tdata[x+1][y]*oneovery;
			v1.z = y*oneoverheight;

			v2.x = (x+1)*oneoverwidth;
			v2.y = tdata[x+1][y+1]*oneovery;
			v2.z = (y+1)*oneoverheight;
			
			second = Tri(v0, v1, v2);
			second.SetNormal((v1-v0).cross(v2-v0));

			//triNormals[x+(y*this->width)].normal = ;
			//D3DXVec3Cross(&this->triNormals[x+(y*this->width)], &(v1-v0), &(v2-v0)); 
			count++;
		}
	}
	PROFILE_END;
}

HRESULT CTerrain::LoadTerrain(const Smart::String &filename)
{
	LPDIRECT3DTEXTURE9 texture;
	D3DSURFACE_DESC surfaceDesc;
	D3DLOCKED_RECT locked;
	HRESULT hr;
	BYTE *bytePointer;
	terrain = this;

	TRACE("Carregando terreno %s...\n", filename.c_str());

	hr=D3DXCreateTextureFromFileEx(d3dDevice, filename.c_str(), 0, 0, 1 ,0 ,D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, D3DX_FILTER_NONE ,D3DX_FILTER_NONE ,0 ,NULL ,NULL ,&texture );
	if(FAILED(hr)) {
		TRACE("\tNão foi possível carregar o terreno! (Código do erro: %d)\n", HRESULT_CODE(hr));
		return hr;
	}
	else
		TRACE("\tSucesso ao carregar a textura do terreno!\n");

	hr=texture->GetLevelDesc(0,&surfaceDesc);
	if(FAILED(hr)) {
		TRACE("\tNão foi possível obter as informações da textura! (Código do erro: %d)\n", HRESULT_CODE(hr));
		texture->Release();
		return hr;
	}
	else
		TRACE("\tSucesso ao obter as informações da textura!\n");

	hr = texture->LockRect(0,&locked,NULL,0);
	if(FAILED(hr)) {
		TRACE("\tNão foi possível trancar a textura! (Código do erro: %d)\n", HRESULT_CODE(hr));
		texture->Release();
		return hr;
	}
	else
		TRACE("\tSucesso ao trancar a textura!\n");

	bytePointer=static_cast<BYTE*>(locked.pBits);

	this->width = surfaceDesc.Width;
	this->height = surfaceDesc.Height;

	this->oneoverwidth = r(1.0)/this->width;
	this->oneoverheight = r(1.0)/this->height;

	const size_t tamanhox = (this->width+1);
	//this->tdata = new unsigned char*[tamanhox];
	this->tdata.SetSize(tamanhox);
	ZeroMemory(&this->tdata, (this->width+1)*sizeof(unsigned char));
	for(register unsigned i = 0; i < this->width; ++i)
	{
		this->tdata[i].SetSize(this->height+1);
		ZeroMemory(&this->tdata[i], (this->height+1)*sizeof(unsigned char));
	}

	for (register unsigned int y=0;y<this->height;++y)
	{
	    for (register unsigned int x=0;x<this->width;++x)
	    {
	       unsigned int index=(x*4+(y*(locked.Pitch)));

		   //this->tdata[x][y] = 0.5;
		   this->tdata[x][y] = r(bytePointer[index+2]/255.0);
	   }
	}

	this->triNormals.SetSize((this->width+1)*(this->height+1));

	this->CalcNormals();

	lod = new CDiamondLOD(this);
	//this->lod->scale.x = this->GetScale()->x;
	this->lod->InitScapeDiamond();

	TRACE("\tTerreno carregado com sucesso!\n");
	return S_OK;
}

CTerrain::~CTerrain()
{
	SafeDelete(this->lod);
	SafeRelease(this->m_pTextura);
	SafeRelease(this->m_pTexturaDetalhe);
	TRACE("Terreno %p removido!\n", this);
}

/*Vector *CTerrain::GetTriNormal(Vector *triNormal, const unsigned int Tri) const
{
	*triNormal = this->triNormals[Tri];
	return triNormal;
}*/

void CTerrain::getCellNum(const Vector *pos, int &x, int &y) const {
	x = static_cast<int>(((pos->x - this->GetPositionX()) / this->GetScale()->x)*this->width)-1;
	y = static_cast<int>(((pos->z - this->GetPositionZ()) / this->GetScale()->z)*this->width)-1;
}

real CTerrain::GetPointInformation(const Vector *pos, real &h, Vector &triNormal) const {
	static Vector v0, v1, v2;
	int CellX, CellY;

	getCellNum(pos, CellX, CellY);

	const real dx = pos->x-((CellX+1)/(this->width/this->GetScale()->x));
	const real dz = pos->z-((CellY+1)/(this->width/this->GetScale()->z));

	const Vector position = *pos-(*this->GetPosition());

	if(CellX > static_cast<int>(this->width-2))
		CellX = static_cast<int>(this->width-2);
	else if(CellX < 1)
		CellX = 1;

	if(CellY > static_cast<int>(this->height-2))
		CellY = static_cast<int>(this->height-2);
	else if(CellY < 1)
		CellY = 1;

	if(dx > dz)
	{
		VertexSet(&v0, CellX, CellY);
		VertexSet(&v1, CellX+1, CellY);
		VertexSet(&v2, CellX+1, CellY+1);
	}
	else
	{
		VertexSet(&v0, CellX, CellY);
		VertexSet(&v1, CellX+1, CellY+1);
		VertexSet(&v2, CellX, CellY+1);
	}
	triNormal = ((v1-v0).cross(v2-v0));

	const real height = this->GetPositionY()+(v0.y+((triNormal.x*dx+triNormal.z*dz)/-triNormal.y));

	triNormal.normalize();

	Vector ponto;
	ponto.x = position.x;
	ponto.y = height;
	ponto.z = position.z;

	Plane plane(ponto, triNormal);
	h = plane.distance(&position);

	return height;
}

real CTerrain::GetTerrainHeight2(const Vector *pos) {
	static Vector triNormal;
	static Vector v0, v1, v2;

	int CellX = static_cast<int>(((pos->x - this->GetPositionX()) / this->GetScale()->x)*this->width)-1;
	int CellY = static_cast<int>(((pos->z - this->GetPositionZ()) / this->GetScale()->z)*this->width)-1;
	const real dx = pos->x-((CellX+1)/(this->width/this->GetScale()->x));
	const real dz = pos->z-((CellY+1)/(this->width/this->GetScale()->z));

	const Vector position = *pos-(*this->GetPosition());

	if(CellX > static_cast<int>(this->width-2))
		CellX = static_cast<int>(this->width-2);
	else if(CellX < 1)
		CellX = 1;

	if(CellY > static_cast<int>(this->height-2))
		CellY = static_cast<int>(this->height-2);
	else if(CellY < 1)
		CellY = 1;

	if(dx > dz)
	{
		VertexSet(&v0, CellX, CellY);
		VertexSet(&v1, CellX+1, CellY);
		VertexSet(&v2, CellX+1, CellY+1);

		this->lasttri = 0;
	}
	else
	{
		VertexSet(&v0, CellX, CellY);
		VertexSet(&v1, CellX+1, CellY+1);
		VertexSet(&v2, CellX, CellY+1);

		this->lasttri = 1;
	}
	triNormal = ((v1-v0).cross(v2-v0));

	const real height = this->GetPositionY()+(v0.y+((triNormal.x*dx+triNormal.z*dz)/-triNormal.y));

	triNormal.normalize();

	Vector ponto;
	ponto.x = v0.x;
	ponto.y = height;
	ponto.z = v0.z;

	Plane plane(ponto, triNormal);
	return plane.distance(&position);
}

real CTerrain::GetTerrainHeight(const Vector *pos)
{
	PROFILE_START;
	static Vector triNormal;
	static Vector v0, v1, v2;

	int CellX = static_cast<int>(((pos->x - this->GetPositionX()) / this->GetScale()->x)*this->width)-1;
	int CellY = static_cast<int>(((pos->z - this->GetPositionZ()) / this->GetScale()->z)*this->width)-1;
	const real dx = pos->x-((CellX+1)/(this->width/this->GetScale()->x));
	const real dz = pos->z-((CellY+1)/(this->width/this->GetScale()->z));

	if(CellX > static_cast<int>(this->width-2))
		CellX = static_cast<int>(this->width-2);
	else if(CellX < 1)
		CellX = 1;

	if(CellY > static_cast<int>(this->height-2))
		CellY = static_cast<int>(this->height-2);
	else if(CellY < 1)
		CellY = 1;

	if(dx > dz)
	{
		VertexSet(&v0, CellX, CellY);
		VertexSet(&v1, CellX+1, CellY);
		VertexSet(&v2, CellX+1, CellY+1);

		this->lasttri = 0;
	}
	else
	{
		VertexSet(&v0, CellX, CellY);
		VertexSet(&v1, CellX+1, CellY+1);
		VertexSet(&v2, CellX, CellY+1);

		this->lasttri = 1;
	}
	triNormal = ((v1-v0).cross(v2-v0));
	
	PROFILE_END;
	return this->GetPositionY()+(v0.y+((triNormal.x*dx+triNormal.z*dz)/-triNormal.y));
}

Vector *CTerrain::GetTerrainNormal(Vector *vOut, const Vector *pos)
{
	PROFILE_START;
	static Vector v0, v1, v2;

	int CellX = static_cast<int>(((pos->x - this->GetPositionX()) / this->GetScale()->x)*this->width)-1;
	int CellY = static_cast<int>(((pos->z - this->GetPositionZ()) / this->GetScale()->z)*this->width)-1;
	const real dx = pos->x-((CellX+1)/(this->width/this->GetScale()->x));
	const real dz = pos->z-((CellY+1)/(this->width/this->GetScale()->z));

	if(CellX > static_cast<int>(this->width-2))
		CellX = static_cast<int>(this->width-2);
	else if(CellX < 1)
		CellX = 1;

	if(CellY > static_cast<int>(this->height-2))
		CellY = static_cast<int>(this->height-2);
	else if(CellY < 1)
		CellY = 1;

	if(dx > dz)
	{
		VertexSet(&v0, CellX, CellY);
		VertexSet(&v1, CellX+1, CellY);
		VertexSet(&v2, CellX+1, CellY+1);

		this->lasttri = 0;
	}
	else
	{
		VertexSet(&v0, CellX, CellY);
		VertexSet(&v1, CellX+1, CellY+1);
		VertexSet(&v2, CellX, CellY+1);

		this->lasttri = 1;
	}
	PROFILE_END;
	return &(*vOut = ((v1-v0).cross(v2-v0)));
}

void CTerrain::Render(const CCamera *cam)
{
	PROFILE_START;
	d3dDevice->SetTexture(0, this->m_pTextura->Get());
	d3dDevice->SetTexture(1, this->m_pTexturaDetalhe->Get());

	d3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_MODULATE4X);

	d3dDevice->SetSamplerState( 1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	d3dDevice->SetSamplerState( 1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );

	this->UpdateMatrix();
	d3dDevice->SetTransform( D3DTS_WORLD, &this->mWorld );
	this->lod->SetCamera(cam);
	this->lod->DoDiamond();

	d3dDevice->SetSamplerState( 1, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	d3dDevice->SetSamplerState( 1, D3DSAMP_MAGFILTER, D3DTEXF_POINT);

	d3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE);

	d3dDevice->SetTexture(1, NULL);

	PROFILE_END;
} 