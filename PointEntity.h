#pragma once

#include "Vector.h"

using Matematica::Vector;
using Matematica::real;

class PointEntity
{
	Vector oldposition, position;
public:
	PointEntity(void);
	PointEntity(const real x, const real y, const real z);
	PointEntity(const Vector *position);
	PointEntity(const Vector &position);
	~PointEntity(void);

	void Position(const Vector *position);
	void Position(const real x, const real y, const real z);
	void PositionY(const real y);

	const Vector *GetPosition() const;
	const Vector *GetOldPosition() const;
	void copyPositionToOldPosition()
	{ this->oldposition = position; }

	real GetPositionX() const;
	real GetPositionY() const;
	real GetPositionZ() const;
};
