#pragma once

#include "PointEntity.h"

class VelocityEntity : public PointEntity
{
	Vector acceleration, velocity;
	Vector vAccelerationDamping, vVelocityDamping;
public:
	VelocityEntity(void);
	~VelocityEntity(void);

	const Vector *GetVelocity() const;
	const Vector *GetAcceleration() const;
	
	void setAccelerationDamping(const Vector *);
	void setAccelerationDamping(const Vector &);
	void setAccelerationDamping(const real x, const real y, const real z);

	void setVelocityDamping(const Vector *);
	void setVelocityDamping(const Vector &);
	void setVelocityDamping(const real x, const real y, const real z);

	void setVelocity(const Vector *);
	void setVelocity(const Vector &);
	void setVelocity(const real x, const real y, const real z);

	void setAcceleration(const Vector *);
	void setAcceleration(const Vector &);
	void setAcceleration(const real x, const real y, const real z);

	void Translate(const Vector *);
	void Translate(const Vector &);
	void Translate(const real x, const real y, const real z);

	void SetupPhysics();
	void UpdatePhysics();
};
