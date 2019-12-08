#pragma once
#include "../../Common/Vector3.h"
#include "../../Common/Matrix3.h"

using namespace NCL::Maths;

namespace NCL {
	class CollisionVolume;
	
	namespace CSC8503 {
		class Transform;

		class PhysicsObject	{
		public:
			PhysicsObject(Transform* parentTransform, const CollisionVolume* parentVolume);
			~PhysicsObject();

			Vector3 GetTorque() const {
				return torque;
			}

			Vector3 GetForce() const {
				return force;
			}

			float GetInverseMass() const {
				return inverseMass;
			}
			void SetInverseMass(float invMass) {
				inverseMass = invMass;
			}

			void ApplyAngularImpulse(const Vector3& force);
			void ApplyLinearImpulse(const Vector3& force);
			
			void AddForce(const Vector3& force);

			void AddForceAtPosition(const Vector3& force, const Vector3& position);
			void AddForceAroundPosition(const Vector3& force, const Vector3& position);

			void AddTorque(const Vector3& torque);


			void ClearForces();
			Vector3 GetLinearVelocity() const {
				return linearVelocity;
			}
			void SetLinearVelocity(const Vector3& v) {
				linearVelocity = v;
			}

			Vector3 GetAngularVelocity() const {
				return angularVelocity;
			}
			void SetAngularVelocity(const Vector3& v) {
				angularVelocity = v;
			}

			void InitCubeInertia();
			void InitSphereInertia(bool isHollow);
			void UpdateInertiaTensor();

			Matrix3 GetInertiaTensor() const {
				return inverseInertiaTensor;
			}

			float GetElasticity() const { return elasticity; }
			void SetElasticity(float el) { elasticity=el; }

			float GetStiffness() const { return stiffness; }
			void SetStiffness(float stiff) { stiffness = stiff; }
			bool GetHandleLikeImpulse() const { return handleLikeImpulse; }
			void SetHandleLikeImpulse(bool handling) { handleLikeImpulse = handling; }
			bool GetHandleLikeSpring() const { return handleLikeSpring; }
			void SetHandleLikeSpring(bool handling) { handleLikeSpring = handling; }

		protected:
			const CollisionVolume* volume;
			Transform* transform;

			//linear stuff
			float inverseMass;
			Vector3 linearVelocity;
			Vector3 force;			

			//angular stuff
			Vector3 angularVelocity;
			Vector3 torque;
			Vector3 inverseInertia;
			Matrix3 inverseInertiaTensor;

			float elasticity;
			float friction;
			float stiffness;

			bool handleLikeImpulse;
			bool handleLikeSpring;
		};
	}
}

