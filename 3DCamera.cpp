#include "stdafx.h"
#include "3DCamera.h"
#include "GameGlobals.h"
#include "defines.h"
#include "profile.h"
#include "globalheader.h"

void CCamera::BuildViewFrustum()
{
	D3DXMATRIX mViewProjection;
	Plane *actual;
	PROFILE_START;
	D3DXMatrixMultiply(&mViewProjection, &this->matView, &this->matProj);

	actual = &this->m_pViewFrustum[FRUSTUM_SIDE::FRUSTUM_LEFT];
	actual->n.x = mViewProjection._14 + mViewProjection._11;
	actual->n.y = mViewProjection._24 + mViewProjection._21;
	actual->n.z = mViewProjection._34 + mViewProjection._31;
	actual->d = mViewProjection._44 + mViewProjection._41;
	//actual->normalize();

	actual = &this->m_pViewFrustum[FRUSTUM_SIDE::FRUSTUM_RIGHT];
	actual->n.x = mViewProjection._14 - mViewProjection._11;
	actual->n.y = mViewProjection._24 - mViewProjection._21;
	actual->n.z = mViewProjection._34 - mViewProjection._31;
	actual->d = mViewProjection._44 - mViewProjection._41;
	//actual->normalize();

	actual = &this->m_pViewFrustum[FRUSTUM_SIDE::FRUSTUM_TOP];
	actual->n.x = mViewProjection._14 - mViewProjection._12;
	actual->n.y = mViewProjection._24 - mViewProjection._22;
	actual->n.z = mViewProjection._34 - mViewProjection._32;
	actual->d = mViewProjection._44 - mViewProjection._42;
	//actual->normalize();

	actual = &this->m_pViewFrustum[FRUSTUM_SIDE::FRUSTUM_BOTTOM];
	actual->n.x = mViewProjection._14 + mViewProjection._12;
	actual->n.y = mViewProjection._24 + mViewProjection._22;
	actual->n.z = mViewProjection._34 + mViewProjection._32;
	actual->d = mViewProjection._44 + mViewProjection._42;
	//actual->normalize();

	actual = &this->m_pViewFrustum[FRUSTUM_SIDE::FRUSTUM_NEAR];
	actual->n.x = mViewProjection._13;
	actual->n.y = mViewProjection._23;
	actual->n.z = mViewProjection._33;
	actual->d = mViewProjection._43;
	//actual->normalize();

	actual = &this->m_pViewFrustum[FRUSTUM_SIDE::FRUSTUM_FAR];
	actual->n.x = mViewProjection._14 - mViewProjection._13;
	actual->n.y = mViewProjection._24 - mViewProjection._23;
	actual->n.z = mViewProjection._34 - mViewProjection._33;
	actual->d = mViewProjection._44 - mViewProjection._43;
	//actual->normalize();

	PROFILE_END;
}

CCamera::CCamera(const bool enable_viewfrustum)
{
	TRACE("Camera %p criada!\n", this);
	if(enable_viewfrustum)
		this->m_pViewFrustum.SetSize(FRUSTUM_SIDE::FRUSTUM_MAX_SIDES);
	D3DXMatrixScaling(&this->matTmp, 1.25, 1.25, 1.25);
	this->SetupPhysics();
	this->shake = 0;
	this->shakevel = 1;
}

void CCamera::UpdateShake() {
	if(this->shake > 0) {
		this->shake -= this->shakevel;
		this->viewport.Y = static_cast<DWORD>(this->y + fabs(7.0*sin(shake)));
		this->viewport.Height = static_cast<DWORD>(this->height + fabs(7.0*sin(shake)));
	}
}

void CCamera::Project()
{
	const float aspect = static_cast<float>(this->viewport.Width) / this->viewport.Height;
	// Build quaternion and transform matrix
	/*D3DXQUATERNION qR;
	qR.x = static_cast<FLOAT>(this->GetQuaternion()->x);
	qR.y = static_cast<FLOAT>(this->GetQuaternion()->y);
	qR.z = static_cast<FLOAT>(this->GetQuaternion()->z);
	qR.w = static_cast<FLOAT>(this->GetQuaternion()->w);
	D3DXVECTOR3 pos;
	pos.x = static_cast<float>(this->GetPositionX());
	pos.y = static_cast<float>(this->GetPositionY());
	pos.z = static_cast<float>(this->GetPositionZ());
// 	pos += pos;
    //D3DXQuaternionRotationYawPitchRoll( &qR, camera.yaw, camera.pitch, camera.roll );
	D3DXMatrixAffineTransformation( &matOrientation, 1.25, NULL, &qR, &pos );.*/
	this->AffineTransformation();


    D3DXMatrixInverse( &matView, NULL, &matOrientation );
    d3dDevice->SetTransform( D3DTS_VIEW, &matView );

    // Projection matrix
    
	const float fogEnd = 10000.0f;
	const float fogStart = 0;

	d3dDevice->SetRenderState(D3DRS_FOGENABLE, TRUE);
	d3dDevice->SetRenderState(D3DRS_FOGCOLOR, D3DCOLOR_XRGB(255,255,255));

	d3dDevice->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_LINEAR);
	d3dDevice->SetRenderState(D3DRS_FOGSTART, *(DWORD *)(&fogStart));
	d3dDevice->SetRenderState(D3DRS_FOGEND, *(DWORD *)(&fogEnd));

	D3DXMatrixPerspectiveFovLH( &matProj, (D3DX_PI/4)/this->scale.y, aspect, 1.0f, fogEnd*1.1f );
    d3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

	if(this->m_pViewFrustum.GetCapacity())
	{
		this->BuildViewFrustum();
	}
}

/*#define REPEAT_CHECK_P(n, v) \
	if(D3DXPlaneDotCoord(&this->m_pViewFrustum[n], v) < 0) {\
		PROFILE_END; \
		return false; \
	}*/

#define REPEAT_CHECK_P(n, v) \
	if(this->m_pViewFrustum[n].distance(v) < 0) {\
		PROFILE_END; \
		return false; \
	}

/*#define REPEAT_CHECK_S(n, v, r) \
	if(D3DXPlaneDotCoord(&this->m_pViewFrustum[n], v) + r < 0) { \
		PROFILE_END; \
		return false; \
	}*/

#define REPEAT_CHECK_S(n, v, r) \
	if(this->m_pViewFrustum[n].distance(v) + r < 0) { \
		PROFILE_END; \
		return false; \
	}

	/*plane.b = this->m_pViewFrustum[n].n.y; \
	plane.c = this->m_pViewFrustum[n].n.z; \
	plane.d = this->m_pViewFrustum[n].d; \
	vec.x = v->x; \
	vec.y = v->y; \
	vec.z = v->z; \
	aaa = D3DXPlaneDotCoord(&plane, &vec); \*/

bool CCamera::PointVisible(const Vector *v) const
{
	PROFILE_START;
	REPEAT_CHECK_P(0, v);
	REPEAT_CHECK_P(1, v);
	REPEAT_CHECK_P(2, v);
	REPEAT_CHECK_P(3, v);
	REPEAT_CHECK_P(4, v);
	REPEAT_CHECK_P(5, v);
	PROFILE_END;
	return true;
}

bool CCamera::SphereVisible(const Vector *v, const real radius) const
{
	if(this->m_pViewFrustum[0].distance(v) + radius < 0)
		return false;
	if(this->m_pViewFrustum[1].distance(v) + radius < 0)
		return false;
	if(this->m_pViewFrustum[2].distance(v) + radius < 0)
		return false;
	if(this->m_pViewFrustum[3].distance(v) + radius < 0)
		return false;
	if(this->m_pViewFrustum[4].distance(v) + radius < 0)
		return false;
	if(this->m_pViewFrustum[5].distance(v) + radius < 0)
		return false;
	return true;
}

bool CCamera::EntityVisible(const Entity *e) const
{
	return this->PointVisible(e->GetPosition());
}

Vector *CCamera::ProjectVector(Vector *pOut, const Vector *pV) const
{
	//Vector tmp;
	D3DXVec3Project(reinterpret_cast<D3DXVECTOR3*>(pOut), reinterpret_cast<const D3DXVECTOR3*>(pV), &this->viewport, &this->matProj, &this->matView, &mIdentity);
	//tmp = pV->Transformed(&this->matProj);
	//tmp = pV->Transformed(&this->matView);

	return pOut;
}

CCamera::~CCamera()
{
	TRACE("Camera %p removida!\n", this);
}
