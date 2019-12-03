#pragma once
#include "CollisionVolume.h"

namespace NCL {
	class CapsuleVolume : CollisionVolume{
	public:
		CapsuleVolume(float bodyHeight, float halfSphereRadius = 1.0f) {
			type = VolumeType::Capsule;
			height = bodyHeight;
			radius = halfSphereRadius;
		}
		~CapsuleVolume() {}

		float GetRadius() const {
			return radius;
		}
	protected:
		float height;
		float radius;
	};
}

