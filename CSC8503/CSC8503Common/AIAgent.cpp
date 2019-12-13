#include "AIAgent.h"
namespace NCL
{
	namespace CSC8503
	{
		AIAgent::AIAgent(const Vector3& position, const std::string name) : GameObject(name)	{
			initialPos = position;
			world = nullptr;
			InitAIStateMachine();
		}

		AIAgent::~AIAgent()
		{
			delete stateMachine;
		}
		void AIAgent::InitAIStateMachine() {

			stateMachine = new StateMachine();
			void* data = this;

			StateFunc idleState = [](void* data)
			{
				AIAgent* obj = (AIAgent*)(data);
				obj->GetRenderObject()->SetColour(Vector4(0.1f, 0.7f, 0.7f, 1));
			};
			StateFunc walkState = [](void* data)
			{
				AIAgent* obj = (AIAgent*)(data);
				obj->GetRenderObject()->SetColour(Vector4(0.1f, 0.1f, 0.1f, 1));
				obj->Walk();
			};
			
			GenericState* stateIdle = new GenericState(idleState, (void*)this);
			GenericState* stateWalk = new GenericState(walkState, (void*)this);
			stateMachine->AddState(stateIdle);
			stateMachine->AddState(stateWalk);
			
			GenericTransition<bool&, bool>* transitionA = new GenericTransition<bool&, bool>(GenericTransition<bool&, bool>::EqualsTransition, walkTowardsGoose, true, stateIdle, stateWalk);
			GenericTransition<bool&, bool>* transitionB = new GenericTransition<bool&, bool>(GenericTransition<bool&, bool>::NotEqualsTransition, walkTowardsGoose, true, stateWalk, stateIdle);
			
			stateMachine->AddTransition(transitionA);
			stateMachine->AddTransition(transitionB);
		}
		void AIAgent::Update(float dt)
		{
			AgentPathfinding(this->GetTransform().GetWorldPosition(), world->GetWorldItemOfType(LayerType::PLAYER)->GetTransform().GetWorldPosition());
			
			stateMachine->Update();
		}
		void AIAgent::Walk() {
			Vector3 dir1 = (targetPoint - this->GetTransform().GetWorldPosition());
			Vector3 dir = (targetPoint - this->GetTransform().GetWorldPosition()).Normalised();
			dir.y = 0;
			this->GetPhysicsObject()->AddForce(dir * 400.5f);
		}

		void AIAgent::OnCollisionBegin(GameObject* other)
		{
			if (other->GetLayer() == LayerType::PLAYER) {
				other->DropCarryingItems();
				walkTowardsGoose = false;
				stateMachine->Update();
			}
		}

		void AIAgent::GoToInitialPosition() {

			NavigationGrid grid("GridMap.txt");
			NavigationPath outPath;

			Vector3 startPos = this->GetTransform().GetWorldPosition();
			bool found = grid.FindPath(startPos, initialPos, outPath);

			if (!found) {
				walkTowardsGoose = false;
			}
			else {
				Vector3 pos;
				Vector3 temp;
				reachableNodes.clear();

				outPath.PopWaypoint(pos);
				reachableNodes.push_back(pos);

				//if ((abs(pos.x - startPos.x) < 2 && abs(pos.z - startPos.z) <2)) {
				outPath.PopWaypoint(pos);
				reachableNodes.push_back(pos);
				//}
				targetPoint = pos;
				while (outPath.PopWaypoint(temp)) {
					reachableNodes.push_back(temp);
				}

				//DisplayPathfinding();
			}
		}

		//---------start of PATHFINDING------------
		void AIAgent::AgentPathfinding(Vector3 startPos, Vector3 endPos) {


			if (world->GetWorldItemOfType(LayerType::PLAYER)->GetCarryingItems().size() == 0) {	//if goose does not carry an object, ignore it
				return;
			}

			NavigationGrid grid("GridMap.txt");
			NavigationPath outPath;

			if (abs(endPos.z - startPos.z) > 80 || abs(endPos.x - startPos.x) > 80) { //if goose is not in a radius of 60 close to the agent, ignore it
				return;
			}

			bool found = grid.FindPath(startPos, endPos, outPath);

			if (!found) {
				GoToInitialPosition();
			}
			else {
				if (endPos != initialPos) {
					walkTowardsGoose = true;
				}
				Vector3 pos;
				Vector3 temp;
				reachableNodes.clear();

				outPath.PopWaypoint(pos);
				reachableNodes.push_back(pos);

				//if ((abs(pos.x - startPos.x) < 2 && abs(pos.z - startPos.z) <2)) {
				outPath.PopWaypoint(pos);
				reachableNodes.push_back(pos);
				//}
				targetPoint = pos;
				walkTowardsGoose = true;
				while (outPath.PopWaypoint(temp)) {
					reachableNodes.push_back(temp);
				}

				//DisplayPathfinding();
			}
		}

		void AIAgent::DisplayPathfinding() {
			for (int i = 1; i < reachableNodes.size(); ++i) {
				Vector3 a = reachableNodes[i - 1];
				Vector3 b = reachableNodes[i];

				Debug::DrawLine(a + Vector3(0, 8, 0), b + Vector3(0, 8, 0), Vector4(1, 0, 0, 1));
			}
		}
		//----------end of PATHFINDING-------------
	}
}
