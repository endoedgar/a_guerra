#include "stdafx.h"
#include "GameGlobals.h"
#include "3DFunctions.h"
#include "3DEntity.h"
#include "globalheader.h"

#define sgn(x) ((x) > 0 ? 1 : -1)

/*D3DXVECTOR3 i(const D3DXQUATERNION *q)
{
	ASSERT(q)
	float xz=q->x*q->z,wy=q->w*q->y;
	float xy=q->x*q->y,wz=q->w*q->z;
	float yy=q->y*q->y,zz=q->z*q->z;
	return D3DXVECTOR3( 1-2*(yy+zz),2*(xy-wz),2*(xz+wy) );
}

D3DXVECTOR3 j(const D3DXQUATERNION *q)
{
	ASSERT(q)
	float yz=q->y*q->z,wx=q->w*q->x;
	float xy=q->x*q->y,wz=q->w*q->z;
	float xx=q->x*q->x,zz=q->z*q->z;
	return D3DXVECTOR3( 2*(xy+wz),1-2*(xx+zz),2*(yz-wx) );
}

D3DXVECTOR3 k(const D3DXQUATERNION *q)
{
	ASSERT(q)
	float xz=q->x*q->z,wy=q->w*q->y;
	float yz=q->y*q->z,wx=q->w*q->x;
	float xx=q->x*q->x,yy=q->y*q->y;
	return D3DXVECTOR3( 2*(xz-wy),2*(yz+wx),1-2*(xx+yy) );
}

D3DXVECTOR3 *QuaternionToEuler(D3DXVECTOR3 *vOut, const D3DXQUATERNION *q)
{
	const float Epsilon = 0.0009765625f;
	const float Threshold = 0.5f - Epsilon;

	const float XY = q->x * q->y;
	const float ZW = q->z * q->w;

	const float TEST = XY + ZW;
	
	if(TEST < -Threshold || TEST > Threshold)
	{
		int sign = sgn(TEST);
		vOut->y = sign * 2 * atan2(q->x, q->w);
		vOut->x = sign * (3.1415f*3.1415f);
		vOut->z = 0;
	}
	else
	{
		const float XX = q->x * q->x;
		const float XZ = q->x * q->z;
		const float XW = q->x * q->w;

		const float YY = q->y * q->y;
		const float YW = q->y * q->w;
		const float YZ = q->y * q->z;

		const float ZZ = q->z * q->z;

		vOut->y = atan2(2 * YW - 2 * XZ, 1 - 2 * YY - 2 * ZZ);
		vOut->x = atan2(2 * XW - 2 * YZ, 1 - 2 * XX - 2 * ZZ);
		vOut->z = asin(2 * TEST);
	} 

	vOut->y = -vOut->y;
	return vOut;
}*/

/*float VectorPitch(const Vector *vec)
{
	return -atan2f( vec->y,sqrtf( vec->x*vec->x+vec->z*vec->z ) );
	D3DXVECTOR3 temp;
	temp = *vec;
	D3DXVec3Normalize(&temp, &temp);
	return -asin(temp.y);
}

float VectorYaw(const Vector *vec)
{
	return -atan2f( vec->x,vec->z );
	/*D3DXVECTOR3 temp;
	temp = *vec;
	D3DXVec3Normalize(&temp, &temp);

	return -atan2(temp.x, temp.z);
	//return asin(temp.x);
}*/

/*D3DXVECTOR3 *PointVector(D3DXVECTOR3 *pOut, const D3DXVECTOR3 *pointer, const D3DXVECTOR3 *target)
{
	D3DXVECTOR3 temp;
	temp = *target-*pointer;
	D3DXVec3Normalize(&temp, &temp);
	pOut->x = asin(temp.y);
	pOut->y = atan2(-temp.x, -temp.z);
	pOut->x = pOut->x + 3.1415f;
	pOut->y = pOut->y;

	return pOut;
}*/

//TRECHO FEITO COM SUCESSO COM ROTACOES
/*D3DXVECTOR3 *TFormVector(D3DXVECTOR3 *pOut, const D3DXVECTOR3 *coord, const Entity *source, const Entity *dest)
{
	D3DXMATRIX mWorld, mRotation, mRotX, mRotY, mRotZ, mScale;

	D3DXMatrixScaling( &mScale, source->scale.x, source->scale.y, source->scale.z );
	D3DXMatrixTranslation( &mWorld, source->pos.x, source->pos.y, source->pos.z );
	D3DXMatrixRotationYawPitchRoll(&mRotation, D3DXToRadian(-source->rotvel.y), D3DXToRadian(source->rotvel.x), D3DXToRadian(source->rotvel.z));
	D3DXMatrixMultiply( &mRotation, &mRotation, &mScale );
	D3DXMatrixMultiply( &mWorld, &mRotation, &mWorld );

	//mWorld = mRotation;
	D3DXVec3TransformCoord( pOut, coord, &mWorld );
	D3DXMatrixIdentity(&mWorld);
	D3DXMatrixIdentity(&mRotX);
	D3DXMatrixIdentity(&mRotY);
	D3DXMatrixIdentity(&mRotZ);
	D3DXMatrixIdentity(&mScale);
	D3DXMatrixIdentity(&mRotation);

	const float result_x = -dest->pos.x;
	const float result_y = -dest->pos.y;
	const float result_z = -dest->pos.z;

	const float pitch = -D3DXToRadian(dest->rotvel.x);
	const float yaw = D3DXToRadian(dest->rotvel.y);
	const float roll = -D3DXToRadian(dest->rotvel.z);

	D3DXMatrixScaling( &mScale, dest->scale.x, dest->scale.y, dest->scale.z );
	D3DXMatrixTranslation( &mWorld, result_x, result_y, result_z );
	D3DXMatrixRotationX(&mRotX, pitch);
	D3DXMatrixRotationY(&mRotY, yaw);
	D3DXMatrixRotationZ(&mRotZ, roll);
	mRotation = mRotY*mRotX*mRotZ;
	//D3DXMatrixRotationYawPitchRoll(&mRotation, yaw, pitch, roll);
	D3DXMatrixMultiply( &mRotation, &mRotation, &mScale );
	D3DXMatrixMultiply( &mWorld, &mRotation, &mWorld );

	D3DXVec3TransformCoord( pOut, pOut, &mWorld );

	return pOut;
}*/

// Funciona Translação e Rotação (falta scaling)
/* D3DXVECTOR3 *TFormVector(D3DXVECTOR3 *pOut, const D3DXVECTOR3 *coord, const Entity *source, const Entity *dest)
{
	D3DXMATRIX mWorld, mRotation, mRotX, mRotY, mRotZ, mScale;

	D3DXMatrixScaling( &mScale, 1/source->scale.x, 1/source->scale.y, 1/source->scale.z );
	D3DXMatrixTranslation( &mWorld, 0, 0, 0);//source->pos.x, source->pos.y, source->pos.z );
	D3DXMatrixRotationYawPitchRoll(&mRotation, D3DXToRadian(-source->rotvel.y), D3DXToRadian(source->rotvel.x), D3DXToRadian(source->rotvel.z));
	D3DXMatrixMultiply( &mRotation, &mRotation, &mScale );
	D3DXMatrixMultiply( &mWorld, &mRotation, &mWorld );

	//mWorld = mRotation;
	D3DXVec3TransformCoord( pOut, coord, &mWorld );
	D3DXMatrixIdentity(&mWorld);
	D3DXMatrixIdentity(&mRotX);
	D3DXMatrixIdentity(&mRotY);
	D3DXMatrixIdentity(&mRotZ);
	D3DXMatrixIdentity(&mScale);
	D3DXMatrixIdentity(&mRotation);

	const float result_x = -dest->pos.x*(1/dest->scale.x);
	const float result_y = -dest->pos.y*(1/dest->scale.y);
	const float result_z = -dest->pos.z*(1/dest->scale.z);

	const float pitch = -D3DXToRadian(dest->rotvel.x);
	const float yaw = D3DXToRadian(dest->rotvel.y);
	const float roll = -D3DXToRadian(dest->rotvel.z);

	//D3DXMatrixScaling( &mScale, 1/dest->scale.x, 1/dest->scale.y, 1/dest->scale.z );
	D3DXMatrixTranslation( &mWorld, 0, 0, 0);//result_x, result_y, result_z );
	D3DXMatrixRotationX(&mRotX, pitch);
	D3DXMatrixRotationY(&mRotY, yaw);
	D3DXMatrixRotationZ(&mRotZ, roll);
	mRotation = mRotY*mRotX*mRotZ;
	//D3DXMatrixRotationYawPitchRoll(&mRotation, yaw, pitch, roll);
	//D3DXMatrixMultiply( &mRotation, &mRotation, &mScale );
	D3DXMatrixMultiply( &mWorld, &mWorld, &mRotation );

	D3DXVec3TransformCoord( pOut, pOut, &mWorld );
	return pOut;
} */

/* D3DXVECTOR3 *TFormVector(D3DXVECTOR3 *pOut, const D3DXVECTOR3 *coord, Entity *Source, Entity *Dest)
{

	Entity *source = Source;
	Entity *dest = Dest;
	if(!source) source = &wld;
	if(!dest) dest = &wld;
	D3DXMATRIX mWorld, mRotation, mRotX, mRotY, mRotZ, mScale;

	D3DXMatrixScaling( &mScale, source->scale.x, source->scale.y, source->scale.z );
	D3DXMatrixTranslation( &mWorld, 0, 0, 0);//source->pos.x, source->pos.y, source->pos.z );
	D3DXMatrixRotationYawPitchRoll(&mRotation, -source->yaw, source->pitch, source->roll);
	D3DXMatrixMultiply( &mScale, &mScale, &mRotation );
	D3DXMatrixMultiply( &mWorld, &mScale, &mWorld );

	//mWorld = mRotation;
	D3DXVec3TransformCoord( pOut, coord, &mWorld );
	D3DXMatrixIdentity(&mWorld);
	D3DXMatrixIdentity(&mRotX);
	D3DXMatrixIdentity(&mRotY);
	D3DXMatrixIdentity(&mRotZ);
	D3DXMatrixIdentity(&mScale);
	D3DXMatrixIdentity(&mRotation);

	const float result_x = -dest->pos.x*(1/dest->scale.x);
	const float result_y = -dest->pos.y*(1/dest->scale.y);
	const float result_z = -dest->pos.z*(1/dest->scale.z);

	const float pitch = -dest->pitch;
	const float yaw = -dest->yaw;
	const float roll = -dest->roll;

	D3DXMatrixScaling( &mScale, dest->scale.x, dest->scale.y, dest->scale.z );
	D3DXMatrixTranslation( &mWorld, 0, 0, 0);//result_x, result_y, result_z );
	D3DXMatrixRotationX(&mRotX, pitch);
	D3DXMatrixRotationY(&mRotY, yaw);
	D3DXMatrixRotationZ(&mRotZ, roll);
	mRotation = mRotY*mRotX*mRotZ;
	//D3DXMatrixRotationYawPitchRoll(&mRotation, yaw, pitch, roll);
	D3DXMatrixMultiply( &mScale, &mScale, &mRotation );
	D3DXMatrixMultiply( &mWorld, &mWorld, &mScale );

	D3DXVec3TransformCoord( pOut, pOut, &mWorld );
	return pOut;
} */

Vector *TFormVector(Vector *pOut, const Vector *coord, Entity *Source, Entity *Dest)
{

	Entity *source = Source;
	Entity *dest = Dest;
	PROFILE_START;

	if(!source) source = g_World;
	if(!dest) dest = g_World;
	static D3DXMATRIX mWorld;

	source->UpdateMatrix();
	dest->UpdateMatrix();

	*pOut = coord->Transformed(&source->mWorld);
	D3DXMatrixInverse(&mWorld, NULL, &dest->mWorld);

	*pOut = pOut->Transformed(&mWorld);

	PROFILE_END;
	return pOut;
}

Vector *TFormPoint(Vector *pOut, const Vector *coord, Entity *Source, Entity *Dest)
{

	Entity *source = Source;
	Entity *dest = Dest;
	PROFILE_START;

	if(!source) source = g_World;
	if(!dest) dest = g_World;
	static D3DXMATRIX mWorld;

	source->UpdateMatrix();
	dest->UpdateMatrix();

	*pOut = coord->Transformed(&source->mWorld);
	D3DXMatrixInverse(&mWorld, NULL, &dest->mWorld);

	*pOut = pOut->Transformed(&mWorld);
	pOut->x += source->mWorld._41;
	pOut->y += source->mWorld._42;
	pOut->z += source->mWorld._43;
	pOut->x += mWorld._41;
	pOut->y += mWorld._42;
	pOut->z += mWorld._43;

	PROFILE_END;
	return pOut;
}

float DeltaPitch(Entity *source, const Vector *dest)
{
	PROFILE_START;
	Vector coord, result;

	source->UpdateMatrix();
	TFormPoint(&result, dest, NULL, source);

	PROFILE_END;
	return result.pitch();
}

float DeltaYaw(Entity *source, const Vector *dest)
{
	Vector coord, result;
	PROFILE_START;

	source->UpdateMatrix();
	TFormPoint(&result, dest, NULL, source);

	PROFILE_END;
	return result.yaw();
}




/*D3DXVECTOR3 *PointVector(D3DXVECTOR3 *pOut, const D3DXVECTOR3 *pointer, const D3DXVECTOR3 *dir, const D3DXVECTOR3 *target, const float factor)
{
	D3DXVECTOR3 temp;
	D3DXQUATERNION quat, quat2;
	PointVector(&temp, pointer, target);

	D3DXQuaternionRotationYawPitchRoll(&quat, pointer->y, pointer->x, pointer->z);
	D3DXQuaternionRotationYawPitchRoll(&quat2, temp.x, temp.y, temp.z);
	D3DXQuaternionSlerp(&quat, &quat, &quat2, factor);

	QuaternionToEuler(pOut, &quat);
	return pOut;
}*/