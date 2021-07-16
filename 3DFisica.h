#ifndef _3DFISICA_H
#define _3DFISICA_H

#pragma once

#include "3DMath.h"
#include "Vector.h"
#include "Quaternion.h"

using namespace Matematica;

struct PhysicState {
	// Primary
	Vector position;
	Vector momentum;

	CQuaternion orientation;
	Vector angularMomentum;

	// Secondary
	Vector velocity;

	CQuaternion spin;
	Vector angularVelocity;

	// Constant
	float mass;
	float inverseMass;

	float inertia;
	float inverseInertia;

	void recalculate() {
		this->velocity = this->momentum * this->inverseMass;

		this->angularVelocity = this->angularMomentum * this->inverseInertia;

		this->orientation.normalize();

		this->spin = 0.5 * CQuaternion(this->angularVelocity.x, this->angularVelocity.y, this->angularVelocity.z, 0) * this->orientation;
	}
};

struct PhysicDerivative {
	Vector velocity;
	Vector force;
	
	CQuaternion spin;
	Vector torque;
};

#include "GameGlobals.h"
class RigidBody {
public:
	PhysicState previous, current;

	RigidBody() {
		current.mass = 1;
		current.inverseMass = 1;
		current.inertia = 1;
		current.inverseInertia = 1;
		current.position = Vector(15, 25, 15);
		current.orientation.identity();
		current.spin.identity();
		current.angularMomentum = current.momentum = Vector(0, 0, 0);

		current.recalculate();
	}

	void UpdatePhysics(float time, float dt) {
		previous = current;
		integrate(current, time, dt);
	}

	static void forces(const PhysicState &state, real t, Vector &force, Vector &torque)
	{
		force = -5 * state.position;

		torque = force.cross(Vector(-1, 0, 0));

		torque -= 0.2f * state.angularVelocity;
	}

	PhysicDerivative evaluate(const PhysicState &state, real t)
	{
		PhysicDerivative output;
		output.velocity = state.velocity;
		output.spin = state.spin;
		forces(state, t, output.force, output.torque);
		return output;
	}

	PhysicDerivative evaluate(const PhysicState &initial, real t, real dt, const PhysicDerivative &d)
	{
		PhysicState state;

		state.position += d.velocity*dt;
		state.momentum += d.force*dt;
		state.orientation += d.spin*dt;
		state.angularMomentum += d.torque*dt;

		state.mass = 1;
		state.inverseMass = 1;
		state.inertia = 1;
		state.inverseInertia = 1;
		state.spin.identity();

		state.recalculate();

		PhysicDerivative output;

		output.velocity = state.velocity;
		output.spin = state.spin;
		forces(state, t+dt, output.force, output.torque);

		return output;
	}

	void integrate(PhysicState &state, real t, real dt) {
		PhysicDerivative a = evaluate(state, t);
		PhysicDerivative b = evaluate(state, t+dt*r(0.5), dt*r(0.5), a);
		PhysicDerivative c = evaluate(state, t+dt*r(0.5), dt*r(0.5), b);
		PhysicDerivative d = evaluate(state, t+dt, dt, c);

		state.position += r(1.0)/r(6.0) * dt * (a.velocity + 2 * (b.velocity + c.velocity) + d.velocity);
		state.momentum += r(1.0)/r(6.0) * dt * (a.force + 2 * (b.force + c.force) + d.force);
		state.orientation += r(1.0)/r(6.0) * dt * (a.spin + 2 * (b.spin + c.spin) + d.spin);
		state.angularMomentum += r(1.0)/r(6.0) * dt * (a.torque + 2 * (b.torque + c.torque) + d.torque);

		state.recalculate();
	}

	Vector torque(const PhysicState &state, real t) {
		return Vector(1, 0, 0) - state.angularVelocity * real(0.1);
	}
};

#endif