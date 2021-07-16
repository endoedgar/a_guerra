#ifndef _3DENTITY_H
#define _3DENTITY_H

#include "defines.h"
#include "InlineList.h"
#include "3DMath.h"
#include "Vector.h"
#include "Quaternion.h"
#include "GameGlobals.h"
#include "VelocityEntity.h"

class Entity
{
public:
	class State {
	public:
		Vector position;
		CQuaternion quat;

		inline const Vector &getPosition(void) const
		{ return this->position; }

		State interpolateTo(const State &end, const real a) const
		{
			State tmp;
			tmp.position = this->position + (end.position-this->position)*a;
			this->quat.slerp(&tmp.quat, &end.quat, a);
			return tmp;
		}
	};
protected:
	State states[2];
	State *currentState;
	int currentStateNum;

private:
	enum {
		TRANSLATE_FLAG = 0x001,
		ROTATE_FLAG = 0x002,
		SCALE_FLAG = 0x004,
		TOTAL_FLAG = TRANSLATE_FLAG | ROTATE_FLAG | SCALE_FLAG
	};
	unsigned int dirtymatrix;
	real massa, invMassa, inertiaTensor, invTensor;

	// primário
	Vector force, momentum;
	Vector vAccelerationDamping, vVelocityDamping;

	Vector com; // center of mass (centro da massa)
	Vector torque, angularMomentum;
	real fTorqueDamping, fAngularDamping;

	// secundário
	Vector velocity, angularVelocity;
	CQuaternion spin;

	void recalculate(void);
public:
	inline D3DXMATRIX *getInterpolatedMatrix(D3DXMATRIX *out, const real alpha) {
		D3DXMATRIX  mTmp2;
		Entity::State st;
		st = this->getOldState().interpolateTo(this->getCurrentState(), alpha);

		st.quat.rotationMatrix(out);
		D3DXMatrixScaling(&mTmp2, this->scale.x, this->scale.y, this->scale.z);
		D3DXMatrixMultiply(out, &mTmp2, out); 
		D3DXMatrixTranslation(&mTmp2, st.getPosition().x, st.getPosition().y, st.getPosition().z);
		D3DXMatrixMultiply(out, out, &mTmp2);

		return out;
	}

	inline void setCOM(const Vector &com)
	{ this->com = com;	}

	inline const State &getCurrentState(void) const
	{ return *this->currentState; }

	inline const State &getOldState(void) const
	{ return this->states[1-currentStateNum]; }

	inline State &getCurrentState(void)
	{ return *this->currentState; }

	inline State &getOldState(void)
	{ return this->states[1-currentStateNum]; }

	inline void copyStateToOldState(void)
	{ this->getOldState() = this->getCurrentState(); }

	const Vector *GetVelocity() const;
	const Vector *GetAcceleration() const;
	
	void setAccelerationDamping(const Vector *);
	void setAccelerationDamping(const Vector &);
	void setAccelerationDamping(const real x, const real y, const real z);

	void setVelocityDamping(const Vector *);
	void setVelocityDamping(const Vector &);
	void setVelocityDamping(const real x, const real y, const real z);

	void setMomentum(const Vector *);
	void setMomentum(const Vector &);
	void setMomentum(const real x, const real y, const real z);

	void Translate(const Vector *);
	void Translate(const Vector &);
	void Translate(const real x, const real y, const real z);

	void Position(const Vector *position);
	void Position(const real x, const real y, const real z);
	void PositionY(const real y);

	const Vector *GetPosition() const
	{ return &this->currentState->position; }

protected:
	Vector scale;

	Entity *pParent;
	CSimpleListAP<Entity*> *childs;
	CSimpleNode<Entity*> *pThisPtr;
public:
	Vector vFront;
	D3DXMATRIX mTranslate, mScale, mRotation, mWorld;
	unsigned char temp;

	inline void setMassa(const real massa) {
		this->massa = massa;
		this->invMassa = static_cast<real>(1.0)/massa;
	}

	inline void setInertiaTensor(const real inertiaTensor) {
		this->inertiaTensor = inertiaTensor;
		this->invTensor = static_cast<real>(1)/this->inertiaTensor;
	}

	inline real getDistanceSq(const Vector *p) const
	{ return DistanceSq(this->GetPosition(), p); }

	inline real getDistanceSq(const Vector &p) const
	{ return this->getDistanceSq(&p); }

	inline real getDistanceSq(const Entity &b) const
	{ return this->getDistanceSq(b.GetPosition()); }

	inline real getDistanceSq(const Entity *b) const
	{ return this->getDistanceSq(*b); }

	inline real getDistance(const Vector *p) const
	{ return sqrt(this->getDistanceSq(p)); }

	inline real getDistance(const Vector &p) const
	{ return this->getDistance(&p); }

	inline real getDistance(const Entity &b) const
	{ return this->getDistance(b.GetPosition()); }

	inline real getDistance(const Entity *b) const
	{ return this->getDistance(*b); }

	inline real getMassa(void)
	{ return this->massa; }

	inline real getInvMassa(void)
	{ return this->invMassa; }

	Entity();
	virtual ~Entity();

	void ImpulseRelative(const Vector *src, const Vector *force);
	void Impulse(const Vector *src, const Vector *force);

	Vector *velocityAtPoint(Vector *out, const Vector &point) const;
	Vector velocityAtPoint(const Vector &point) const;

	void SetupPhysics();

	inline Entity *GetParent() const
	{ return this->pParent; }

	inline void Parent(Entity *pParent)
	{
		if(this->pParent)
			this->pParent->childs->Delete(&this->pThisPtr);
		if(pParent) {
			this->pThisPtr = pParent->childs->New();
			this->pThisPtr->SetPtr(this);
		}
		this->pParent = pParent;
	}
	
	inline const Vector *GetPosition(const bool global) const
	{
		if(global) {
			static Vector vTemp;
			vTemp.x = this->mWorld._41;
			vTemp.y = this->mWorld._42;
			vTemp.z = this->mWorld._43;
			return &vTemp;	
		} else
			return this->GetPosition();
	}

	inline real getSpeed() const
	{
		return this->GetVelocity()->length();
	}

	inline real GetPositionX(const bool global = false) const
	{
		if(global)
			return this->mWorld._41;
		else
			return this->currentState->position.x;
	}

	inline real GetPositionY(const bool global = false) const
	{
		if(global)
			return this->mWorld._42;
		else
			return this->currentState->position.y;
	}

	inline real GetPositionZ(const bool global = false) const
	{
		if(global)
			return this->mWorld._43;
		else
			return this->currentState->position.z;
	}

	//inline const Vector *GetVelocity() const
	//{ return &this->vel; }
	inline const Vector *getForces() const
	{ return &this->force; }
	inline const Vector *GetScale() const
	{ return &this->scale; }
	inline const CQuaternion *GetQuaternion() const
	{ return &this->currentState->quat; }
	inline const Vector *GetFrontVector() const
	{ return &this->vFront; }

	inline void setAngularDamping(const real fAngularDamping)
	{ this->fAngularDamping = 1-fAngularDamping; }

	inline void setTorqueDamping(const real fTorqueDamping)
	{ this->fTorqueDamping = 1-fTorqueDamping; }

	inline const Vector &getAngularVelocity(void) const
	{ return this->angularVelocity; }

	inline Vector &getAngularVelocity(void)
	{ return this->angularVelocity; }

	//void Translate(const Vector *);
	//void Translate(const real x, const real y, const real z);
	void Move(const Vector *);
	inline void Move(const real x, const real y, const real z)
	{ Vector v;
	  v.x = x;
	  v.y = y;
	  v.z = z;
	  this->Move(&v); }

	inline void Scale(const Vector *s)
	{
		ASSERT(s)
		this->scale = *s;
		this->dirtymatrix |= SCALE_FLAG;
	}

	/*inline void SlerpTo(const CQuaternion *q, const real rate)
	{
		CQuaternion tmp;
		this->currentState->quat.slerp(&tmp, q, rate);
		this->Turn(tmp.Pitch(), -tmp.Yaw(), -tmp.Roll());
		//this->dirtymatrix |= ROTATE_FLAG;
		//D3DXQuaternionSlerp(&this->qQuat, q, &this->qQuat, rate);
	}*/


	inline void Scale(const real x, const real y, const real z)
	{
		this->scale.x = x;
		this->scale.y = y;
		this->scale.z = z;
		this->dirtymatrix |= SCALE_FLAG;
		//D3DXMatrixScaling(&this->mScale, this->scale.x, this->scale.y, this->scale.z );
	}

	inline void Rotate(const CQuaternion *q)
	{
		ASSERT(q)
		this->currentState->quat = *q;
		this->dirtymatrix |= ROTATE_FLAG;
	}

	inline void Rotate(const Vector *r)
	{
		ASSERT(r)
		this->currentState->quat.setEulerAngles(r->x, r->y, r->z);
		this->dirtymatrix |= ROTATE_FLAG;
		//D3DXQuaternionRotationYawPitchRoll(&this->qQuat, r->y, r->x, r->z);
	}

	inline void Rotate(const real pitch, const real yaw, const real roll)
	{
		this->currentState->quat.setEulerAngles(pitch, yaw, roll);
		this->dirtymatrix |= ROTATE_FLAG;
	}


	inline real Entity::pitch() const
	{
		return this->currentState->quat.pitch();
	}

	inline real Entity::yaw() const
	{
		return this->currentState->quat.yaw();
	}

	inline real Entity::roll() const
	{
		return this->currentState->quat.roll();
	}

	void Spin(const Vector *);
	void Spin(const real pitch, const real yaw, const real roll);
	void Turn(const Vector *);
	void Turn(const real pitch, const real yaw, const real roll);

	void AlignToVector(const Vector *, const unsigned char axis, const real rate = 1.0f);

	bool UpdateMatrix(const D3DXMATRIX *mPre = &mIdentity);

	void UpdatePhysics();
	friend class CJogador;
};

#endif