#ifndef _3DCAMERA_H
#define _3DCAMERA_H

#pragma once

#include "3DEntity.h"
#include "Plane.h"

enum FRUSTUM_SIDE {
	FRUSTUM_LEFT,
	FRUSTUM_RIGHT,
	FRUSTUM_TOP,
	FRUSTUM_BOTTOM,
	FRUSTUM_NEAR,
	FRUSTUM_FAR,
	FRUSTUM_MAX_SIDES
};

class CCamera : public Entity 
{ 
private:
	D3DXMATRIX	matOrientation, matView, matProj, matTmp;
	D3DVIEWPORT9 viewport;
	Smart::Array<Plane> m_pViewFrustum;
	real shake, shakevel, y, height;

	void BuildViewFrustum();
public:
	CCamera(const bool enable_viewfrustum = true);
	void UpdateShake();
	void Project();
	void Shake(real shake)
	{ this->shake = shake; }

	Vector *ProjectVector(Vector *pOut, const Vector *pV) const;
	bool PointVisible(const Vector *v) const;

	inline void AffineTransformation()
	{
		this->matOrientation = this->mWorld;
		//D3DXMATRIX mTranslate, mRotate;
		//D3DXMatrixTranslation(&mTranslate, this->GetPositionX(), this->GetPositionY(), this->GetPositionZ());
		//this->matOrientation = this->matTmp * *this->GetQuaternion()->RotationMatrix(&mRotate) * mTranslate;
	}

	inline bool PointVisible(const real x, const real y, const real z) const
	{
		Vector tmp;
		tmp.x = x;
		tmp.y = y;
		tmp.z = z;

		return this->PointVisible(&tmp);
	}
	bool SphereVisible(const Vector *v, const real radius) const;
	inline bool SphereVisible(const real x, const real y, const real z, const real radius) const
	{
		Vector v(x, y, z);
		return SphereVisible(&v, radius);
	}
	bool EntityVisible(const Entity *e) const;

	~CCamera();

	inline D3DXMATRIX *GetMatrixOrientation()
	{ return &this->matOrientation; }

	inline const D3DXMATRIX *GetMatrixView() const
	{ return &this->matView; }

	inline D3DXMATRIX *GetMatrixProj() 
	{ return &this->matProj; }

	inline void setViewport(D3DVIEWPORT9 &viewport)
	{
		this->viewport = viewport;
		this->y = static_cast<real>(viewport.Y);
		this->height = static_cast<real>(viewport.Height);
	}

	inline D3DVIEWPORT9 *getViewport()
	{ return &this->viewport; }
};

#endif