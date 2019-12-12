#pragma once
#include "GameObject.h"
#include "GameWorld.h"
#include "StateMachine.h"
#include "State.h"
#include "StateTransition.h"
#include "NavigationGrid.h"

namespace NCL
{
	namespace CSC8503
	{
		class AIAgent : public GameObject
		{
		public:
			AIAgent(const Vector3& position, std::string name = "Enemy");
			virtual ~AIAgent();

			void InitAIStateMachine();
			void Update(float dt);
			void Walk();
			void GoToInitialPosition();
			virtual void OnCollisionBegin(GameObject* other) override;

			void SetWorld(GameWorld* gameWorld) { world = gameWorld; }

			//void ClearTargetPoint() { targetPoint = initialPos; }
		protected:
			GameWorld* world;
			void AgentPathfinding(Vector3 startPos, Vector3 endPos);
			void DisplayPathfinding();
			Vector3 targetPoint;
			unsigned int objectID;
			Vector3 initialPos;
			StateMachine* stateMachine;
			bool walkTowardsGoose = false;
			vector<Vector3> reachableNodes;
		};
	}
}

