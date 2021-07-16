#include "stdafx.h"
#include "EstadoFisico.h"

State::State(void)
{
}

State::~State(void)
{
}

void State::recalculate() {
	velocity = momentum * inverseMass;
	angularVelocity = angularMomentum * inverseInertiaTensor;
	orientation.normalize();
	spin = real(0.5) * CQuaternion(angularVelocity.x, angularVelocity.y, angularVelocity.z, 0) * orientation;
	D3DXMATRIX translation, rotation;
	D3DXMatrixTranslation(&translation, this->position.x, this->position.y, this->position.z);
	bodyToWorld = translation * (*orientation.rotationMatrix(&rotation));
	D3DXMatrixInverse(&worldToBody, NULL, &bodyToWorld);
}