#ifndef _CTERRAIN_H
#define _CTERRAIN_H

#include "3DEntity.h"
#include "diamondAlgorithm.h"
#include "3DTexture.h"
#include "Segment.h"

class CCamera;

class CTerrain : public Entity, public VideoResource
{
	CTexture *m_pTextura, *m_pTexturaDetalhe;
	class Tri {
		Vector a, b, c;
		Vector normal;
	public:
		Tri(const Vector &a, const Vector &b, const Vector &c):a(a), b(b), c(c)
		{ }

		Tri():a(0, 0, 0), b(0, 0, 0), c(0, 0, 0)
		{ }

		void SetNormal(const Vector &norm)
		{ this->normal = norm; }

		const Vector &GetNormal(void) const
		{ return this->normal; }

		Vector &getA()
		{ return this->a; }

		Vector &getB()
		{ return this->b; }

		Vector &getC()
		{ return this->c; }
	};

	inline Vector *VertexSet(Vector *vec, const int fCx, const int fCy) const
	{
		ASSERT(vec)
		vec->x = (fCx/static_cast<real>(this->width))*this->GetScale()->x;
		vec->y = this->tdata[fCx][fCy]*this->GetScale()->y;
		vec->z = (fCy/static_cast<real>(this->width))*this->GetScale()->z;

		return vec;
	}

public:
	Smart::Array<Smart::Array<real>> tdata;
	Smart::Array<Tri> triNormals;
	unsigned int width, height;
	unsigned int lasttri;
	real oneoverwidth, oneoverheight;
	real friction;

	class CDiamondLOD *lod;

	CTerrain();
	inline CTexture *getTexture() {
		return this->m_pTextura;
	}
	HRESULT LoadTerrain(const Smart::String &filename);
	void Render(const CCamera *);
	void setTexture(CTexture *tex)
	{
		if(this->m_pTextura)
			this->m_pTextura->Release();
		this->m_pTextura = tex;
		if(this->m_pTextura)
			this->m_pTextura->AddRef();
	}
	void setDetailTexture(CTexture *tex)
	{ 
		if(this->m_pTexturaDetalhe)
			this->m_pTexturaDetalhe->Release();
		this->m_pTexturaDetalhe = tex;
		if(this->m_pTexturaDetalhe)
			this->m_pTexturaDetalhe->AddRef();
	}

	void getCellNum(const Vector *pos, int &x, int &y) const;
	real GetPointInformation(const Vector *pos, real &h, Vector &triNormal) const;
	real GetTerrainHeight(const Vector *);
	real GetTerrainHeight2(const Vector *);
	Vector *GetTerrainNormal(Vector *vOut, const Vector *pos);
	bool segmentIntersect(real &frac, Vector &pos, Vector &normal, const Segment &seg) const;

	inline HRESULT OnLostDevice()
	{ return this->lod->OnLostDevice(); }
	inline HRESULT OnResetDevice()
	{ return this->lod->OnResetDevice(); }

	inline real GetTerrainHeight(const real x, const real y, const real z)
	{
		Vector tmp;
		tmp.x = x;
		tmp.y = y;
		tmp.z = z;
		return this->GetTerrainHeight(&tmp);
	}

	void CalcNormals();

	real GetWidth()
	{ return this->width * this->GetScale()->x; }

	real GetDepth()
	{ return this->height * this->GetScale()->z; }

	//Vector *GetTriNormal(Vector *triNormal, const unsigned int Tri) const;
	~CTerrain();
};

#include "diamondAlgorithm.h"

#endif