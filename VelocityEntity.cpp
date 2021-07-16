#include "stdafx.h"
#include "VelocityEntity.h"

VelocityEntity::VelocityEntity(void):PointEntity()
{ }

VelocityEntity::~VelocityEntity(void)
{ }

const Vector *VelocityEntity::GetVelocity() const
{ return &this->velocity; }

const Vector *VelocityEntity::GetAcceleration() const
{ return &this->acceleration; }

void VelocityEntity::setVelocity(const Vector *velocity)
{ this->velocity = *velocity; }

void VelocityEntity::setVelocity(const Vector &velocity)
{ this->velocity = velocity; }

void VelocityEntity::setVelocity(const real x, const real y, const real z)
{
	this->velocity.x = x;
	this->velocity.x = y;
	this->velocity.x = z;
}

void VelocityEntity::setAcceleration(const Vector *acceleration)
{ this->acceleration = *acceleration; }

void VelocityEntity::setAcceleration(const Vector &acceleration)
{ this->acceleration = acceleration; }

void VelocityEntity::setAcceleration(const real x, const real y, const real z)
{
	this->acceleration.x = x;
	this->acceleration.x = y;
	this->acceleration.x = z;
}

void VelocityEntity::Translate(const Vector *amount)
{ this->acceleration += *amount; }

void VelocityEntity::Translate(const Vector &amount)
{ this->acceleration += amount; }

void VelocityEntity::Translate(const real x, const real y, const real z)
{
	this->acceleration.x += x;
	this->acceleration.y += y;
	this->acceleration.z += z;
}

void VelocityEntity::UpdatePhysics()
{
	this->velocity += this->acceleration;
	this->Position(this->GetPositionX()+this->velocity.x, this->GetPositionY()+this->velocity.y, this->GetPositionZ()+this->velocity.z);

	this->velocity *= this->vVelocityDamping;
	this->acceleration *= this->vAccelerationDamping;
}

void VelocityEntity::setAccelerationDamping(const Vector *accelerationdamping)
{ this->vAccelerationDamping = *accelerationdamping; }

void VelocityEntity::setAccelerationDamping(const Vector &accelerationdamping)
{ this->vAccelerationDamping = accelerationdamping; }

void VelocityEntity::setAccelerationDamping(const real x, const real y, const real z)
{
	this->vAccelerationDamping.x = x;
	this->vAccelerationDamping.y = y;
	this->vAccelerationDamping.z = z;
}

void VelocityEntity::setVelocityDamping(const Vector *velocitydamping)
{ this->vVelocityDamping = *velocitydamping; }

void VelocityEntity::setVelocityDamping(const Vector &velocitydamping)
{ this->vVelocityDamping = velocitydamping; }

void VelocityEntity::setVelocityDamping(const real x, const real y, const real z)
{
	this->vVelocityDamping.x = x;
	this->vVelocityDamping.y = y;
	this->vVelocityDamping.z = z;
}