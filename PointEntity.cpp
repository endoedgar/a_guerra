#include "stdafx.h"
#include "PointEntity.h"

PointEntity::PointEntity(void)
{ }

PointEntity::PointEntity(const real x, const real y, const real z):position(x, y, z), oldposition(x, y, z)
{ }

PointEntity::PointEntity(const Vector *position):position(*position), oldposition(*position)
{ }

PointEntity::PointEntity(const Vector &position):position(position), oldposition(position)
{ }

PointEntity::~PointEntity(void)
{ }

void PointEntity::Position(const Vector *position)
{ 
	this->oldposition = this->position;
	this->position = *position;
}

void PointEntity::Position(const real x, const real y, const real z)
{
	static Vector tmp;
	tmp.x = x;
	tmp.y = y;
	tmp.z = z;
	this->Position(&tmp);
}

void PointEntity::PositionY(const real y)
{
	this->oldposition = this->position;
	this->position.y = y;
}

const Vector *PointEntity::GetPosition() const
{ return &this->position; }

const Vector *PointEntity::GetOldPosition() const
{ return &this->oldposition; }

real PointEntity::GetPositionX() const
{ return this->position.x; }

real PointEntity::GetPositionY() const
{ return this->position.y; }

real PointEntity::GetPositionZ() const
{ return this->position.z; }