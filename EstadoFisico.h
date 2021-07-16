#pragma once

#include "Vector.h"
#include "Quaternion.h"

using Matematica::real;
using Matematica::Vector;
using Matematica::CQuaternion;

class State
{
public:
	Vector position;
	Vector momentum;
	CQuaternion orientation;
	Vector angularMomentum;

	Vector velocity;
	CQuaternion spin;
	Vector angularVelocity;
	D3DXMATRIX bodyToWorld;
	D3DXMATRIX worldToBody;

	real size;
	real mass;
	real inverseMass;
	real inertiaTensor;
	real inverseInertiaTensor;

	State(void);
	~State(void);

	void recalculate();

	bool operator==(const State &other) const
	{
		return position==other.position && orientation==other.orientation && 
		momentum==other.momentum && angularMomentum==other.angularMomentum;
	}
	
	/// inequality operator (primary quantities only)

	bool operator!=(const State &other) const
	{
		return !(*this==other);
	}
};

struct Derivative {
	Vector velocity;
	Vector force;
	CQuaternion spin;
	Vector torque;
};
