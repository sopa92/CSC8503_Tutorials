#pragma once
#include "Transform.h"
#include "CollisionVolume.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "NetworkObject.h"

#include <vector>

using std::vector;

namespace NCL {
	enum class LayerType {
		PLAYER, ENEMY, OBJECT, CAMERA, FLOOR, CUBE, SPHERE, WATER, NEST, LIMIT
	};
	namespace CSC8503 {
		class NetworkObject;
		class GameWorld;

		class GameObject	{
		public:
			GameObject(string name = "");
			~GameObject();

			void SetBoundingVolume(CollisionVolume* vol) {
				boundingVolume = vol;
			}

			const CollisionVolume* GetBoundingVolume() const {
				return boundingVolume;
			}

			bool IsActive() const {
				return isActive;
			}
			void SetAsActive(bool act) {
				isActive = act;
			}

			const Transform& GetConstTransform() const {
				return transform;
			}

			Transform& GetTransform() {
				return transform;
			}

			RenderObject* GetRenderObject() const {
				return renderObject;
			}

			PhysicsObject* GetPhysicsObject() const {
				return physicsObject;
			}

			NetworkObject* GetNetworkObject() const {
				return networkObject;
			}

			void SetRenderObject(RenderObject* newObject) {
				renderObject = newObject;
			}

			void SetPhysicsObject(PhysicsObject* newObject) {
				physicsObject = newObject;
			}

			const string& GetName() const {
				return name;
			}

			virtual void OnCollisionBegin(GameObject* otherObject) {
				if (this->GetLayer() == LayerType::PLAYER && otherObject->GetLayer() == LayerType::NEST) {
					playerIsInNest = true;
				}
			}

			virtual void OnCollisionEnd(GameObject* otherObject) {
				if (this->GetLayer()== LayerType::PLAYER && otherObject->GetLayer() == LayerType::NEST) {
					playerIsInNest = false;
				}
			}

			bool GetBroadphaseAABB(Vector3&outsize) const;

			void UpdateBroadphaseAABB();

			void DrawDebug(const Vector4& color);

			void DrawDebugVolume();


			LayerType GetLayer() { return layer; }
			void SetLayer(LayerType type) { layer = type; }

			void CollectObject(GameObject* collectable);

			bool IsCollected() { return collected; }
			void SetAsCollected(bool collect) {
				collected = collect;
				this->SetAsActive(false);
			}
			int GetCollectedItems() { return collectedObjects; }

			vector<GameObject*> GetCarryingItems() { return carryingObjects; }
			void ClearCarryingItems() { carryingObjects.clear(); }


			vector<Vector3> GetRespawningPositions() { return respawningPositions; }

			void SetApplesToBeSpawned(int number) { applesToBeSpawned = number; }
			int GetApplesToBeSpawned() { return applesToBeSpawned; }

			void SetBonusItemsToBeSpawned(int number) { bonusItemsToBeSpawned = number; }
			int GetBonusItemsToBeSpawned() { return bonusItemsToBeSpawned; }


			bool IsInNest() { return playerIsInNest; }

			void DropCarryingItems();

			void SetWorld(GameWorld* gameWorld) { world = gameWorld; }

		protected:
			Transform			transform;

			CollisionVolume*	boundingVolume;
			PhysicsObject*		physicsObject;
			RenderObject*		renderObject;
			NetworkObject*		networkObject;
			GameWorld* world;
			bool	isActive;
			string	name;

			LayerType	layer;
			bool collected;
			bool playerIsInNest;
			int collectedObjects;
			vector<GameObject*> carryingObjects;
			vector<Vector3> respawningPositions;

			Vector3 broadphaseAABB;
			int applesToBeSpawned;
			int bonusItemsToBeSpawned;
		};
	}
}

