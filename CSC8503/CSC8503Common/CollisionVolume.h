#pragma once
#include "../../Common/Vector4.h"
namespace NCL {
	enum class VolumeType {
		AABB	= 1,
		OBB		= 2,
		Sphere	= 4, 
		Mesh	= 8,
		Compound= 16,
		Capsule = 32,
		Invalid = 256
	};

	class CollisionVolume
	{
	public:
		CollisionVolume() {
			type = VolumeType::Invalid;
		}
		~CollisionVolume() {}
		VolumeType type;
		virtual void DrawDebug(const NCL::Maths::Vector3& position, const NCL::Maths::Vector4& colour = NCL::Maths::Vector4(0, 1, 0, 1)) {}
	};
}