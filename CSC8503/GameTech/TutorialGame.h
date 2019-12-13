#pragma once
#include "GameTechRenderer.h"
#include "../CSC8503Common/PhysicsSystem.h"
#include "../CSC8503Common/GameWorld.h"
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLTexture.h"
#include "../../Common/TextureLoader.h"

#include "../CSC8503Common/PositionConstraint.h"
#include "../CSC8503Common/NavigationGrid.h"
#include "../CSC8503Common/StateMachine.h"
#include "../CSC8503Common/State.h"
#include "../CSC8503Common/StateTransition.h"
#include "../CSC8503Common/AIAgent.h"

namespace NCL {	
	namespace CSC8503 {

		class TutorialGame		{
		public:
			TutorialGame();
			~TutorialGame();

			virtual void UpdateGame(float dt);
			void PutItemsInNest(int& score);

		protected:
			void InitialiseAssets();

			void InitCamera();
			void UpdateKeys();

			void InitWorld();

			void AddGroundAndLake();

			void CreateLimitsForAI();

			void AddBonusItems();
			void AddWalls();

			/*
			These are some of the world/object creation functions I created when testing the functionality
			in the module. Feel free to mess around with them to see different objects being created in different
			test scenarios (constraints, collision types, and so on). 
			*/
			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius, const Vector3& positionTranslation);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims, const Vector3& positionTranslation);
			void BridgeConstraintTest();
			void SimpleGJKTest();

			bool SelectObject();
			void MoveSelectedObject();
			void DebugObjectMovement();
			void LockedObjectMovement();
			void LockedCameraMovement();
			
			GameObject* AddFloorToWorld(
				const Vector3& position, 
				Vector3 floorSize = Vector3(60, 2, 60), 
				OGLTexture* texture = nullptr, 
				LayerType layer = LayerType::FLOOR, 
				string name= "floor",
				bool resolveAsSprings = false
			);
			//GameObject* AddPoolToWorld(const Vector3& position);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float elasticity, float inverseMass = 10.0f, LayerType layer = LayerType::SPHERE, string name= "Sphere");
			GameObject* AddCubeToWorld(
				const Vector3& position, 
				Vector3 dimensions, 
				float elasticity, 
				float inverseMass = 10.0f, 
				Vector4 colour = Vector4(1,1,1,1), 
				bool isAppleThrower=false);
			//IT'S HAPPENING
			GameObject* AddGooseToWorld(const Vector3& position);
			AIAgent* AddParkKeeperToWorld(const Vector3& position);
			GameObject* AddCharacterToWorld(const Vector3& position);
			GameObject* AddAppleToWorld(const Vector3& position);

			void AddBasket();
			void AddFences();

			GameTechRenderer*	renderer;
			PhysicsSystem*		physics;
			GameWorld*			world;

			bool useGravity;
			bool inSelectionMode;

			float		forceMagnitude;

			GameObject* selectionObject = nullptr;
			GameObject* seenObject = nullptr;

			OGLMesh*	cubeMesh	= nullptr;
			OGLMesh*	sphereMesh	= nullptr;
			OGLTexture* basicTex	= nullptr; 
			OGLTexture* wallTex		= nullptr;
			OGLTexture* waterTex	= nullptr;
			OGLShader*	basicShader = nullptr;

			//Coursework Meshes
			OGLMesh*	gooseMesh	= nullptr;
			OGLMesh*	keeperMesh	= nullptr;
			OGLMesh*	appleMesh	= nullptr;
			OGLMesh*	charA		= nullptr;
			OGLMesh*	charB		= nullptr;

			//Coursework Additional functionality	
			GameObject* lockedObject	= nullptr;
			Vector3 lockedOffset		= Vector3(0, 14, 25);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}
			void SpawnApples(int amount, float dt);
			void ReSpawnItems(int amount, bool isApple);
			int itemsSpawned;
			int repetitions;
			Vector3 gooseInitPos;
			Vector3 agentInitPos;
			Vector3 appleThrowerPos;
			AIAgent* parkKeeper;
			GameObject* player;
		};
	}
}

