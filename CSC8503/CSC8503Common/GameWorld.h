#pragma once
#include <vector>
#include "Ray.h"
#include "CollisionDetection.h"
#include "QuadTree.h"
namespace NCL {
		class Camera;
		using Maths::Ray;
	namespace CSC8503 {
		class GameObject;
		class Constraint;

		typedef std::function<void(GameObject*)> GameObjectFunc;
		typedef std::vector<GameObject*>::const_iterator GameObjectIterator;

		class GameWorld	{
		public:
			GameWorld();
			~GameWorld();

			void Clear();
			void ClearAndErase();

			void AddGameObject(GameObject* o);
			void RemoveGameObject(GameObject* o);

			void AddConstraint(Constraint* c);
			void RemoveConstraint(Constraint* c);

			Camera* GetMainCamera() const {
				return mainCamera;
			}

			void ShuffleConstraints(bool state) {
				shuffleConstraints = state;
			}

			void ShuffleObjects(bool state) {
				shuffleObjects = state;
			}

			bool Raycast(Ray& r, RayCollision& closestCollision, bool closestObject = false) const;

			virtual void UpdateWorld(float dt);

			void OperateOnContents(GameObjectFunc f);

			void GetObjectIterators(
				GameObjectIterator& first,
				GameObjectIterator& last) const;

			void GetConstraintIterators(
				std::vector<Constraint*>::const_iterator& first,
				std::vector<Constraint*>::const_iterator& last) const;

			GameObject* GetPlayer() {
				GameObjectIterator itr;
				for (itr = gameObjects.begin(); itr < gameObjects.end(); ++itr)
				{
					if ((*itr)->GetLayer() == LayerType::PLAYER) {
						return (*itr);
					}
				}
				return nullptr;
			}
			GameObject* GetNest() {
				GameObjectIterator itr;
				for (itr = gameObjects.begin(); itr < gameObjects.end(); ++itr)
				{
					if ((*itr)->GetLayer() == LayerType::NEST) {
						return (*itr);
					}
				}
				return nullptr;
			}
			int GetScore() { return score; }
			void SetScore(int scr) { score = scr; }
			int collectedObjects;
			std::vector<GameObject*> carryingObjects;
			void SetApplesToBeSpawned(int number){ applesToBeSpawned = number;}
			int GetApplesToBeSpawned() { return applesToBeSpawned; }

			void SetBonusItemsToBeSpawned(int number) { bonusItemsToBeSpawned = number; }
			int GetBonusItemsToBeSpawned() { return bonusItemsToBeSpawned; }


			void SetEnableAppleThrower(bool status) { enableAppleThrower = status; }
			bool GetEnableAppleThrower() { return enableAppleThrower; }

		protected:
			void UpdateTransforms();
			void UpdateQuadTree();

			std::vector<GameObject*> gameObjects;
			std::vector<Constraint*> constraints;

			QuadTree<GameObject*>* quadTree;
			int applesToBeSpawned = 0;
			int bonusItemsToBeSpawned = 0;
			bool enableAppleThrower = false;

			Camera* mainCamera;
			int score;
			bool shuffleConstraints;
			bool shuffleObjects;
		};
	}
}

