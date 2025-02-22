#include "PhysicsObject.h"
#include "PhysicsSystem.h"
#include "../CSC8503Common/Transform.h"
using namespace NCL;
using namespace CSC8503;

PhysicsObject::PhysicsObject(Transform* parentTransform, const CollisionVolume* parentVolume)	{
	transform	= parentTransform;
	volume		= parentVolume;

	inverseMass = 1.0f;
	elasticity	= 0.8f;
	friction	= 0.8f;
}

PhysicsObject::~PhysicsObject()	{

}

void PhysicsObject::AddForce(const Vector3& addedForce) {
	force += addedForce;
}

void PhysicsObject::AddForceAtPosition(const Vector3& addedForce, const Vector3& position) { // calculate position relative to object's center of mass
	Vector3 localPos = position - transform->GetWorldPosition();

	force  += addedForce;
	torque += Vector3::Cross(localPos, addedForce);  // use cross product to determine the axis around which force will cause object to spin
}

void PhysicsObject::AddForceAroundPosition(const Vector3& addedForce, const Vector3& position) {
	force += addedForce;
	torque += Vector3::Cross(position, addedForce);
}

void PhysicsObject::AddTorque(const Vector3& addedTorque) {
	torque += addedTorque;
}

void PhysicsObject::ClearForces() {
	force = Vector3();
	torque = Vector3();
}

void PhysicsObject::ApplyAngularImpulse(const Vector3& force) {	//scales its input by the appropriate inverse mass representation, and adds it to the appropriate velocity vector.

	angularVelocity += inverseInertiaTensor * force;
}

void PhysicsObject::ApplyLinearImpulse(const Vector3& force) {	//scales its input by the appropriate inverse mass representation, and adds it to the appropriate velocity vector.

	linearVelocity += force * inverseMass;
}

void PhysicsObject::InitCubeInertia() {
	Vector3 dimensions	= transform->GetLocalScale();

	Vector3 fullWidth = dimensions * 2;

	Vector3 dimsSqr = fullWidth * fullWidth;

	inverseInertia.x = (12.0f * inverseMass) / (dimsSqr.y + dimsSqr.z);
	inverseInertia.y = (12.0f * inverseMass) / (dimsSqr.x + dimsSqr.z);
	inverseInertia.z = (12.0f * inverseMass) / (dimsSqr.x + dimsSqr.y);
}

void PhysicsObject::InitSphereInertia(bool isHollow) {
	float radius = transform->GetLocalScale().GetMaxElement();
	float i = 0;
	
	if (isHollow)
	{
		i = 1.5f * inverseMass / (radius * radius);
	}
	else
	{
		i = 2.5f * inverseMass / (radius * radius);
	}

	inverseInertia	= Vector3(i, i, i);
}

void PhysicsObject::UpdateInertiaTensor() {
	Quaternion q = transform->GetWorldOrientation();
	
	Matrix3 invOrientation	= Matrix3(q.Conjugate());
	Matrix3 orientation		= Matrix3(q);

	inverseInertiaTensor = orientation * Matrix3::Scale(inverseInertia) *invOrientation;
}