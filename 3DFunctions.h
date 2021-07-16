#ifndef _3DFUNCTIONS_H
#define _3DFUNCTIONS_H

#pragma once

#include "3DEntity.h"

/*D3DXVECTOR3 i(const D3DXQUATERNION *q);
D3DXVECTOR3 j(const D3DXQUATERNION *q);
D3DXVECTOR3 k(const D3DXQUATERNION *q);*/
D3DXVECTOR3 *QuaternionToEuler(D3DXVECTOR3 *vOut, const D3DXQUATERNION *q);

float DeltaPitch(Entity *source, const Vector *dest);
float DeltaYaw(Entity *source, const Vector *dest);

inline float DeltaPitch(Entity *source, Entity *dest)
{ return DeltaPitch(source, dest->GetPosition()); }

inline float DeltaYaw(Entity *source, Entity *dest)
{ return DeltaYaw(source, dest->GetPosition()); }

//float VectorPitch(const Vector *);
//float VectorYaw(const Vector *);


Vector *TFormVector(Vector *pOut, const Vector *coord, Entity *source, Entity *dest);
Vector *TFormPoint(Vector *pOut, const Vector *coord, Entity *source, Entity *dest);
D3DXVECTOR3 *PointVector(Vector *pOut, const Vector *eye, const Vector *target);

inline real randomf(const real value)
{
	real generated = static_cast<real>(rand())/RAND_MAX;
	return value * generated;
}

inline unsigned int random(const unsigned int value)
{
	if(value) {
		const unsigned int ntimes = (value/RAND_MAX)+2;
		unsigned int valor = 0;
		for(register unsigned int a = 0; a <= ntimes; a++) {
			valor += rand();
		}
		return valor % value;
	}
	return 0;
}

//D3DXVECTOR3 *PointVector(D3DXVECTOR3 *pOut, const D3DXVECTOR3 *pointer, const D3DXVECTOR3 *dir, const D3DXVECTOR3 *target, const float factor);

#endif