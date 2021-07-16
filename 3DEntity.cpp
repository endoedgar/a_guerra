#include "stdafx.h"
#include "3DFunctions.h"
#include "3DEntity.h"
#include "defines.h"
#include "GenericFunctions.h"
#include "globalheader.h"

Entity::Entity():pParent(NULL), scale(1, 1, 1), dirtymatrix(TRANSLATE_FLAG | ROTATE_FLAG | SCALE_FLAG)
{
	this->mWorld = this->mRotation = this->mScale = this->mTranslate = mIdentity;
	this->childs = new CSimpleListAP<Entity*>();
	this->setMassa(1);
	this->setInertiaTensor(1);
	this->vFront.clear();
	this->SetupPhysics();
	this->recalculate();
	this->UpdateMatrix();
	this->setAngularDamping(1);
	this->setTorqueDamping(1);
	TRACE("Entidade %p criada!\n", this);
}



Entity::~Entity()
{
	this->Parent(NULL);
	if(this->childs) {
		CSimpleNode<Entity*> *p = this->childs->GetFirstActive();
		while(p) {
			(*p->GetPtr())->Parent(NULL);
			p = p->GetNext();
		}
	}
	SafeDelete(this->childs);
	TRACE("Entidade %p removida!\n", this);
}


void Entity::Position(const Vector *p)
{
	this->currentState->position = *p;
	this->dirtymatrix |= TRANSLATE_FLAG;
}

void Entity::SetupPhysics()
{
	this->currentStateNum = 0;
	this->currentState = &this->states[this->currentStateNum];

	this->currentState->quat.identity();
	this->copyStateToOldState();
	this->torque.clear();
	this->angularVelocity.clear();
	this->angularMomentum.clear();
	this->force.clear();
	this->velocity.clear();
	this->momentum.clear();
	this->vAccelerationDamping.clear();
	this->vVelocityDamping.clear();
}

Vector *Entity::velocityAtPoint(Vector *out, const Vector &point) const
{
	*out = *this->GetVelocity();
	*out += (this->getAngularVelocity().cross(point-this->com));
	return out;
}

Vector Entity::velocityAtPoint(const Vector &point) const
{
	Vector tmp;
	return *velocityAtPoint(&tmp, &point);
}

void Entity::Impulse(const Vector *src, const Vector *force)
{
	Vector forcerot;//, tmp;
	Vector comt;
	TFormVector(&comt, &com, this, NULL);
	this->Translate(force);
	forcerot = (*src-comt).cross(force);
	this->Spin(&forcerot);
}

void Entity::ImpulseRelative(const Vector *src, const Vector *force)
{
	Vector forceT, point;
	TFormVector(&point, src, this, NULL);
	TFormVector(&forceT, force, this, NULL);
	this->Impulse(&point, &forceT);
}


/*void Entity::Translate(const Vector *p)
{
	this->acel += *p;
}

void Entity::Translate(const real x, const real y, const real z)
{
	this->acel.x += x;
	this->acel.y += y;
	this->acel.z += z;
}*/

void Entity::Move(const Vector *v)
{
	this->currentState->quat.apply(&vFront, v);

	this->Translate(vFront);
	//this->acel += vFront;
}

void Entity::Spin(const Vector *t)
{
	this->torque += *t;
}

void Entity::Spin(const real pitch, const real yaw, const real roll)
{
	const Vector tmp(pitch, yaw, roll);
	this->Spin(&tmp);
}

void Entity::Turn(const Vector *t)
{
	Vector vTemp = *t;
	vTemp *= this->invTensor;

	CQuaternion qTemp(vTemp.x, vTemp.y, vTemp.z);

	//this->currentState->quat * (0.5 * CQuaternion(this->angularVelocity.x, this->angularVelocity.y, this->angularVelocity.z, 0));
	qTemp = this->currentState->quat * qTemp;

	qTemp.apply(&vTemp, t);

	//vTemp *= this->inertiaTensorr;

	this->Spin(&vTemp);
}

void Entity::Turn(const real pitch, const real yaw, const real roll)
{
	const Vector tmp(pitch, yaw, roll);
	this->Turn(&tmp);
}

bool Entity::UpdateMatrix(const D3DXMATRIX *mPre)
{
	Entity *parent = this->GetParent();
	bool updated = false;

	if(this->dirtymatrix) {
		if(this->dirtymatrix & TRANSLATE_FLAG)
			D3DXMatrixTranslation( &this->mTranslate, this->GetPositionX(), this->GetPositionY(), this->GetPositionZ() );
		D3DXMatrixMultiply(&this->mWorld, mPre, &this->mTranslate);

		if(this->dirtymatrix & SCALE_FLAG)
			D3DXMatrixScaling(&this->mScale, this->scale.x, this->scale.y, this->scale.z );

		if(this->dirtymatrix & ROTATE_FLAG)
			this->currentState->quat.rotationMatrix(&this->mRotation);

		D3DXMatrixMultiply( &this->mRotation, &this->mScale, &this->mRotation);
		D3DXMatrixMultiply( &this->mWorld, &this->mRotation, &this->mWorld );
		updated = true;
	}

	if(parent) {
		if(parent->UpdateMatrix() || updated)
			D3DXMatrixMultiply( &mWorld, &mWorld, &parent->mWorld );
	}
	this->dirtymatrix = 0;
	return updated;
}

void Entity::recalculate(void)
{
	this->velocity = this->momentum * invMassa;
	this->angularVelocity = this->angularMomentum * invTensor;

	this->currentState->quat.normalize();

	this->spin = this->currentState->quat * (static_cast<real>(0.5) * CQuaternion(this->angularVelocity.x, this->angularVelocity.y, this->angularVelocity.z, 0));
}

void Entity::UpdatePhysics()
{
	this->currentStateNum = (this->currentStateNum) ? 0 : 1;
	this->currentState = &this->states[this->currentStateNum];
	*this->currentState = this->getOldState();

	//p += v*dt + 0.5*a*dt*dt;
	// + 0.5 *this->force*this->invMassa;

	this->momentum += this->force;
	this->angularMomentum += this->torque;

	this->recalculate();

	this->currentState->position += this->velocity;
	this->currentState->quat += this->spin;
	this->currentState->quat.normalize();

	this->momentum *= this->vVelocityDamping;
	this->force *= this->vAccelerationDamping;

	this->angularMomentum *= this->fAngularDamping;
	this->torque *= this->fTorqueDamping;

	this->dirtymatrix |= TRANSLATE_FLAG | ROTATE_FLAG;
}

void Entity::Position(const real x, const real y, const real z)
{
	static Vector tmp;
	tmp.x = x;
	tmp.y = y;
	tmp.z = z;
	this->Position(&tmp);
}

void Entity::PositionY(const real y)
{
	this->currentState->position.y = y;
}

const Vector *Entity::GetVelocity() const
{ return &this->velocity; }

void Entity::setMomentum(const Vector *momentum)
{ this->momentum = *momentum; }

void Entity::setMomentum(const Vector &momentum)
{ this->momentum = momentum; }

void Entity::setMomentum(const real x, const real y, const real z)
{
	this->momentum.x = x;
	this->momentum.x = y;
	this->momentum.x = z;
}
void Entity::Translate(const Vector *amount)
{ this->force += *amount; }

void Entity::Translate(const Vector &amount)
{ this->force += amount; }

void Entity::Translate(const real x, const real y, const real z)
{
	this->force.x += x;
	this->force.y += y;
	this->force.z += z;
}

void Entity::setAccelerationDamping(const Vector *accelerationdamping)
{ this->vAccelerationDamping = *accelerationdamping; }

void Entity::setAccelerationDamping(const Vector &accelerationdamping)
{ this->vAccelerationDamping = accelerationdamping; }

void Entity::setAccelerationDamping(const real x, const real y, const real z)
{
	this->vAccelerationDamping.x = x;
	this->vAccelerationDamping.y = y;
	this->vAccelerationDamping.z = z;
}

void Entity::setVelocityDamping(const Vector *velocitydamping)
{ this->vVelocityDamping = *velocitydamping; }

void Entity::setVelocityDamping(const Vector &velocitydamping)
{ this->vVelocityDamping = velocitydamping; }

void Entity::setVelocityDamping(const real x, const real y, const real z)
{
	this->vVelocityDamping.x = x;
	this->vVelocityDamping.y = y;
	this->vVelocityDamping.z = z;
}

void Entity::AlignToVector(const Vector *vec, const unsigned char axis, const real rate)
{
	CQuaternion qTemp;
	
	switch(axis)
	{
		// TODO: Fazer os outros eixos
		case 3:
			qTemp.setEulerAngles(vec->pitch(), vec->yaw(), 0);
			//D3DXQuaternionRotationYawPitchRoll(&qTemp, -VectorYaw(vec), VectorPitch(vec), 0);
			break;
		default:
			RuntimeError("AlignToVector não contém esse eixo! (eixo = %d)", axis);
			return;
	}

	if(rate > 1)
		this->currentState->quat.slerp(&this->currentState->quat, &qTemp, 1);
	else if(rate < 0)
		this->currentState->quat.slerp(&this->currentState->quat, &qTemp, 0);
	else
		this->currentState->quat.slerp(&this->currentState->quat, &qTemp, rate);

	this->dirtymatrix = ROTATE_FLAG;
}