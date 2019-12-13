/*

http://soundbible.com/1187-Goose.html
*/

#include "TutorialGame.h"
#include <sstream>

using namespace NCL;
using namespace CSC8503;

TutorialGame::TutorialGame()	{
	world		= new GameWorld();
	renderer	= new GameTechRenderer(*world);
	physics		= new PhysicsSystem(*world);
	forceMagnitude	= 10.0f;
	useGravity		= false;
	inSelectionMode = false;
	itemsSpawned = 0;
	repetitions = 1;
	Debug::SetRenderer(renderer);

	InitialiseAssets();
}

/*

Each of the little demo scenarios used in the game uses the same 2 meshes, 
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void TutorialGame::InitialiseAssets() {
	auto loadFunc = [](const string& name, OGLMesh** into) {
		*into = new OGLMesh(name);
		(*into)->SetPrimitiveType(GeometryPrimitive::Triangles);
		(*into)->UploadToGPU();
	};

	loadFunc("cube.msh"		 , &cubeMesh);
	loadFunc("sphere.msh"	 , &sphereMesh);
	loadFunc("RotatedGoose.msh"	 , &gooseMesh);
	loadFunc("RotatedCharacterA.msh", &keeperMesh);
	loadFunc("RotatedCharacterM.msh", &charA);
	loadFunc("RotatedCharacterF.msh", &charB);
	loadFunc("Apple.msh"	 , &appleMesh);

	basicTex	= (OGLTexture*)TextureLoader::LoadAPITexture("checkerboard.png");
	wallTex = (OGLTexture*)TextureLoader::LoadAPITexture("brick.png");
	waterTex = (OGLTexture*)TextureLoader::LoadAPITexture("water.tga");
	basicShader = new OGLShader("GameTechVert.glsl", "GameTechFrag.glsl");

	InitWorld();
	InitCamera();
}

TutorialGame::~TutorialGame()	{
	delete cubeMesh;
	delete sphereMesh;
	delete gooseMesh;
	delete basicTex;
	delete wallTex;
	delete waterTex;
	delete basicShader;

	delete physics;
	delete renderer;
	delete world;
}

void TutorialGame::UpdateGame(float dt) {

	parkKeeper->Update(dt);

	if (!inSelectionMode) {
		world->GetMainCamera()->UpdateCamera(dt);
	}
	if (lockedObject != nullptr) {
		LockedCameraMovement();
	}

	UpdateKeys();

	if (useGravity) {
		Debug::Print("(G)ravity on", Vector2(10, 40));
	}
	else {
		Debug::Print("(G)ravity off", Vector2(10, 40));
	}
	int applesCreated = 20;
	SpawnApples(applesCreated, dt);

	int bonusItemsToRespawn = player->GetBonusItemsToBeSpawned();
	if (bonusItemsToRespawn > 0) {
		ReSpawnItems(bonusItemsToRespawn, false);
	}
	int applesRespawned = player->GetApplesToBeSpawned();
	if (applesRespawned > 0) {
		ReSpawnItems(applesRespawned, true);

	}
	itemsSpawned -= applesRespawned + bonusItemsToRespawn;
	int score = world->GetScore();
	int collected = player->GetCollectedItems();
	if (collected > 0) {
		Debug::Print("Collected : " + std::to_string(collected) + "/" + std::to_string(itemsSpawned) + "...", Vector2(10, 100));
		if (collected == itemsSpawned)
			Debug::Print("You must return to nest quickly!", Vector2(10, 80));
	}

	if (player->IsInNest()) {
		PutItemsInNest(score);
	}

	if (score > 0) {
		if ((collected == applesCreated && score == applesCreated) || score == applesCreated + 20) {

			const Vector2 posText(200, 500);
			Debug::Print(" We have a WINNER!!! Scored: " + std::to_string(world->GetScore()), posText, Vector4(0, 1, 0, 1));
			Debug::Print(" Press ESC to leave or H to honk! ", Vector2(posText.x - 50, posText.y -70), Vector4(0, 1, 0, 1));			
		}
		else
			Debug::Print("Score : " + std::to_string(score), Vector2(10, 60), Vector4(0.9f, 0.9f, 0.9f, 1));
	}
	SelectObject();
	MoveSelectedObject();

	world->UpdateWorld(dt);
	renderer->Update(dt);
	physics->Update(dt);

	Debug::FlushRenderables();
	renderer->Render();
}

void TutorialGame::PutItemsInNest(int& score) {

	int itemsToPutInBasket = player->GetCarryingItems().size();
	if (itemsToPutInBasket > 0) {
		for (int i = 0; i < itemsToPutInBasket; ++i) {
			if (player->GetCarryingItems()[i]->GetName() == "apple") {
				++score;
				AddSphereToWorld(Vector3(-3.63f + i * 0.01f, 4 + i / 2.0f, -86.6f + i * 0.001f), 0.3f, 0, 1, LayerType::SPHERE, "apple");
			}
			else {
				score += 5;
				AddSphereToWorld(Vector3(-3.63f + i * 0.01f, 4 + i / 2.0f, -86.6f + i * 0.001f), 0.3f, 0, 1, LayerType::SPHERE, "bonusItem");
			}
		}
		world->SetScore(score);
		player->ClearCarryingItems();
	}

}

void TutorialGame::UpdateKeys() {

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1)) {
		InitWorld(); //We can reset the simulation at any time with F1
		selectionObject = nullptr;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F2)) {
		InitCamera(); //F2 will reset the camera to a specific default place
	}
	
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::G)) {
		useGravity = !useGravity; //Toggle gravity!
		physics->UseGravity(useGravity);
		world->SetEnableAppleThrower(true);
		
	}
	//Running certain physics updates in a consistent order might cause some
	//bias in the calculations - the same objects might keep 'winning' the constraint
	//allowing the other one to stretch too much etc. Shuffling the order so that it
	//is random every frame can help reduce such bias.
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F9)) {
		world->ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F10)) {
		world->ShuffleConstraints(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F7)) {
		world->ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F8)) {
		world->ShuffleObjects(false);
	}

	if (lockedObject) {
		LockedObjectMovement();
	}
	else {
		DebugObjectMovement();
	}
}

void TutorialGame::LockedObjectMovement() {
	Matrix4 view		= world->GetMainCamera()->BuildViewMatrix();
	Matrix4 camWorld	= view.Inverse();

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)) + Vector3(50, 0, 0); //view is inverse of model!

	//forward is more tricky -  camera forward is 'into' the screen...
	//so we can take a guess, and use the cross of straight up, and
	//the right axis, to hopefully get a vector that's good enough!

	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);
	Vector3 vertAxis = Vector3::Cross(fwdAxis, rightAxis);

	float rotationSpeed = 60.0f;
	Vector3 objectOrientation = lockedObject->GetConstTransform().GetLocalOrientation().ToEuler();

	objectOrientation.y = objectOrientation.y >= 0.0f 
		? (objectOrientation.y <= 360.0f 
			? objectOrientation.y 
			: objectOrientation.y - 360.0f) 
		: objectOrientation.y + 360.0f;
	
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT) || Window::GetKeyboard()->KeyDown(KeyboardKeys::A)) {
		objectOrientation.y += objectOrientation.y <= 270.0f
			? (objectOrientation.y > 90.0f
				? -rotationSpeed * 0.1f
				: rotationSpeed * 0.1f)
			: rotationSpeed * 0.1f;
		lockedObject->GetPhysicsObject()->AddForce(-rightAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT) || Window::GetKeyboard()->KeyDown(KeyboardKeys::D)) {
		objectOrientation.y += objectOrientation.y >= 90.0f
			? (objectOrientation.y < 270.0f
				? rotationSpeed * 0.1f
				: -rotationSpeed * 0.1f)
			: -rotationSpeed * 0.1f;
		lockedObject->GetPhysicsObject()->AddForce(rightAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP) || Window::GetKeyboard()->KeyDown(KeyboardKeys::W)) {
		objectOrientation.y += objectOrientation.y >= 0.0f
			? (objectOrientation.y < 180.0f
				? -rotationSpeed * 0.1f
				: rotationSpeed * 0.1f)
			: rotationSpeed * 0.1f;
		lockedObject->GetPhysicsObject()->AddForce(fwdAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN) || Window::GetKeyboard()->KeyDown(KeyboardKeys::S)) {
		objectOrientation.y += objectOrientation.y >= 0.0f
			? (objectOrientation.y < 180.0f
				? rotationSpeed * 0.1f
				: -rotationSpeed * 0.1f)
			: -rotationSpeed * 0.1f;
		lockedObject->GetPhysicsObject()->AddForce(-fwdAxis);
	}

	lockedObject->GetTransform().SetLocalOrientation(Quaternion::EulerAnglesToQuaternion(objectOrientation.x, objectOrientation.y, objectOrientation.z));
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::SPACE)) {
		lockedObject->GetPhysicsObject()->AddForce(Vector3(0,200,0));
	}
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::H)) {
		PlaySound("..\\..\\Assets\\GooseHonk.wav", NULL, SND_ASYNC);
	}
}

void  TutorialGame::LockedCameraMovement() {
	if (lockedObject != nullptr) {
		Vector3 objPos = lockedObject->GetTransform().GetWorldPosition();
		Vector3 camPos = objPos + lockedOffset;

		Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0, 1, 0));

		Matrix4 modelMat = temp.Inverse();

		Quaternion q(modelMat);
		Vector3 angles = q.ToEuler(); //nearly there now!

		world->GetMainCamera()->SetPosition(camPos);
		//world->GetMainCamera()->SetPitch(angles.x + 15);
		//world->GetMainCamera()->SetYaw(angles.y);
	}
}

void TutorialGame::DebugObjectMovement() {
//If we've selected an object, we can manipulate it with some key presses
	if (inSelectionMode && selectionObject) {
		//Twist the selected object!
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUMPAD6)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 0, -10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUMPAD4)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 0, 10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUMPAD8)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(-10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUMPAD2)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(50, 0, 0));
		}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(-50, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -50));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 50));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUMPAD5)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 50, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUMPAD0)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -50, 0));
		}
	}
}

/*

Every frame, this code will let you perform a raycast, to see if there's an object
underneath the cursor, and if so 'select it' into a pointer, so that it can be
manipulated later. Pressing Q will let you toggle between this behaviour and instead
letting you move the camera around.

*/
bool TutorialGame::SelectObject() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::Q)) {
		inSelectionMode = !inSelectionMode;
		if (inSelectionMode) {
			Window::GetWindow()->ShowOSPointer(true);
			Window::GetWindow()->LockMouseToWindow(false);
		}
		else {
			Window::GetWindow()->ShowOSPointer(false);
			Window::GetWindow()->LockMouseToWindow(true);
		}
	}

	if (inSelectionMode) {
		renderer->DrawString("Press Q to change to camera mode!", Vector2(10, 0));
		if (selectionObject)
		{
			Debug::Print("Selected object " + selectionObject->GetName() + " is located ", Vector2(0, 300), Vector4(0, 0, 1, 1));
			Debug::Print("at position: " + std::to_string(selectionObject->GetTransform().GetWorldPosition().x)
				+ ", " + std::to_string(selectionObject->GetTransform().GetWorldPosition().y)
				+ ", " + std::to_string(selectionObject->GetTransform().GetWorldPosition().z), Vector2(0, 270), Vector4(0, 0, 1, 1));

			if (seenObject)
			{
				seenObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				seenObject = nullptr;
			}
			// getting object IN-FRONT of selected object
			Ray objectForwardRay = Ray(selectionObject->GetConstTransform().GetWorldPosition(), selectionObject->GetConstTransform().GetWorldOrientation() * Vector3(0,0,-1));
			RayCollision closestObjectCollision;
			if (world->Raycast(objectForwardRay, closestObjectCollision, true))
			{
				seenObject = (GameObject*)closestObjectCollision.node;
				seenObject->GetRenderObject()->SetColour(Vector4(0, 0, 1, 1));  
				renderer->DrawLine(selectionObject->GetTransform().GetWorldPosition(), seenObject->GetTransform().GetWorldPosition(), seenObject->GetRenderObject()->GetColour());
			}

		}
		if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::LEFT)) {
			if (selectionObject) {	//set colour to deselected;
				selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				selectionObject = nullptr;
				if (seenObject)
				{
					seenObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
					seenObject = nullptr;
				}
			}
			
			Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

			RayCollision closestCollision;
			if (world->Raycast(ray, closestCollision, true)) {
				selectionObject = (GameObject*)closestCollision.node;
				selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
				return true;
			}
			else {
				return false;
			}
		}
		if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::L)) {
			if (selectionObject) {
				if (lockedObject == selectionObject) {
					lockedObject = nullptr;
				}
				else {
					lockedObject = selectionObject;
				}
			}
		}
	}
	else {
		renderer->DrawString("Press Q to change to select mode!", Vector2(10, 0));
	}
	return false;
}

/*
If an object has been clicked, it can be pushed with the right mouse button, by an amount
determined by the scroll wheel. In the first tutorial this won't do anything, as we haven't
added linear motion into our physics system. After the second tutorial, objects will move in a straight
line - after the third, they'll be able to twist under torque aswell.
*/

void TutorialGame::MoveSelectedObject() {
	//renderer->DrawString(" Click Force :" + std::to_string(forceMagnitude), Vector2(10, 20)); // Draw debug text at 10 ,20
	forceMagnitude += Window::GetMouse()->GetWheelMovement() * 100.0f;
	if (!selectionObject) {
		return;// we haven 't selected anything !
	}
	// Push the selected object !
	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::RIGHT)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());
		RayCollision closestCollision;
		if (world->Raycast(ray, closestCollision, true)) {
			if (closestCollision.node == selectionObject) {
				selectionObject->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * forceMagnitude, closestCollision.collidedAt);
			}
		}
	}
}

void TutorialGame::InitCamera() {
	world->GetMainCamera()->SetNearPlane(0.5f);
	world->GetMainCamera()->SetFarPlane(500.0f);
	world->GetMainCamera()->SetPitch(-15.0f);
	world->GetMainCamera()->SetYaw(0.0f);
	lockedObject = player;
}

void TutorialGame::InitWorld() {
	world->ClearAndErase();
	physics->Clear();

	appleThrowerPos = Vector3(-59, 2, -209);
	AddCubeToWorld(appleThrowerPos, Vector3(1, 2, 1), 0, 0, Vector4(0,1,0,1), true);		//applethrower
	
	agentInitPos = Vector3(40, 2, -160);
	AddParkKeeperToWorld(agentInitPos);
	//AddCharacterToWorld(Vector3(45, 2, -10));
	AddGroundAndLake();

	
	CreateLimitsForAI();	
	
	AddGooseToWorld(Vector3(0, 2, -90));

	AddFences();
	AddBonusItems();
	AddWalls();

}

void TutorialGame::AddGroundAndLake() {
	AddFloorToWorld(Vector3(0, -2, 0), Vector3(60, 2, 30));		// first piece of floor
	AddFloorToWorld(Vector3(-45, -2, -90), Vector3(15, 2, 60));	// left side piece of floor
	AddFloorToWorld(Vector3(45, -2, -90), Vector3(15, 2, 60));	// right side piece of floor
	AddFloorToWorld(Vector3(0, -2, -180), Vector3(60, 2, 30));	// second piece of floor

	AddFloorToWorld(Vector3(0, -1.9f, -90), Vector3(30, 2, 60), waterTex, LayerType::WATER, "lake", true); //lake
	AddFloorToWorld(Vector3(0, -1, -90), Vector3(30,0.5f,60));	// lake bottom

	OGLTexture* islangTex = (OGLTexture*)TextureLoader::LoadAPITexture("island.jpg");
	AddFloorToWorld(Vector3(0, -1, -90), Vector3(5, 1.5f, 5), islangTex, LayerType::NEST, "island");	// island nest
	AddBasket();
}

void TutorialGame::CreateLimitsForAI() {
	AddFloorToWorld(Vector3(0, 0, -29), Vector3(30, 0.5f, 1), nullptr, LayerType::LIMIT); //lake outbound
	AddFloorToWorld(Vector3(0, 0, -151), Vector3(30, 0.5f, 1), nullptr, LayerType::LIMIT); //lake outbound
	AddFloorToWorld(Vector3(-31, 0, -90), Vector3(1, 0.5f, 60), nullptr, LayerType::LIMIT); //lake outbound
	AddFloorToWorld(Vector3(31, 0, -90), Vector3(1, 0.5f, 60), nullptr, LayerType::LIMIT); //lake outbound

	AddFloorToWorld(Vector3(-45, 0, -94), Vector3(14.9f, 0.5f, 2), nullptr, LayerType::LIMIT);	// left obstacle
	AddFloorToWorld(Vector3(-45, 0, -86), Vector3(14.9f, 0.5f, 2), nullptr, LayerType::LIMIT);	// left obstacle


	AddFloorToWorld(Vector3(-44, 0, -197), Vector3(2, 0.2f, 14), nullptr, LayerType::LIMIT); //fence
	AddFloorToWorld(Vector3(-34, 0, -175), Vector3(10, 0.2f, 2), nullptr, LayerType::LIMIT); //fence
	AddFloorToWorld(Vector3(-18, 0, -175.2f), Vector3(6, 0.2f, 2), nullptr, LayerType::LIMIT); //door
	AddFloorToWorld(Vector3(-2, 0, -175), Vector3(10, 0.2f, 2), nullptr, LayerType::LIMIT); //fence
	AddFloorToWorld(Vector3(18, 0, -175), Vector3(10, 0.2f, 2), nullptr, LayerType::LIMIT); //fence
	AddFloorToWorld(Vector3(38, 0, -175), Vector3(10, 0.2f, 2), nullptr, LayerType::LIMIT); //fence
	AddFloorToWorld(Vector3(54, 0, -175), Vector3(6, 0.2f, 2), nullptr, LayerType::LIMIT); //fence
}

void TutorialGame::AddWalls() {
	AddFloorToWorld(Vector3(-45, 2, -90), Vector3(14.9f, 2, 3), wallTex);	// left obstacle
	AddFloorToWorld(Vector3(0, 25, -215), Vector3(60, 30, 5), wallTex);	// wall front
	AddFloorToWorld(Vector3(-61, 25, -90), Vector3(1, 30, 120), wallTex);	// wall left
	AddFloorToWorld(Vector3(61, 25, -90), Vector3(1, 30, 120), wallTex);	// wall right
	AddFloorToWorld(Vector3(0, 25, 35), Vector3(60, 30, 5), wallTex);	// wall front
}

void TutorialGame::AddBonusItems() {
	int xPos = 55;
	int yPosBall = 17;
	int yPosTramboline = 5;
	int zPos = -30;

	for (int i = 0; i < 4; ++i) {
		AddSphereToWorld(Vector3(-xPos, yPosBall + i * 3, zPos * i), 1.0f, 1, 0, LayerType::OBJECT, "bonusItem");
		AddCubeToWorld(Vector3(-xPos, yPosTramboline + i * 3, zPos * i), Vector3(5, 0.5f, 5), 20, 0, Vector4(0, 1, 1, 1));
		++itemsSpawned;
	}
}

GameObject* TutorialGame::AddFloorToWorld(const Vector3& position, Vector3 floorSize, OGLTexture* texture, NCL::LayerType layer, string name, bool handleAsSprings) {
	GameObject* floor = new GameObject(name);
	floor->SetLayer(layer);

	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform().SetWorldScale(floorSize);
	floor->GetTransform().SetWorldPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, texture, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->SetElasticity(0.8f);
	floor->GetPhysicsObject()->InitCubeInertia();
	if (texture == nullptr) {
		floor->GetRenderObject()->SetColour(Vector4(0.2f, 0.5f, 0.2f, 1));
		if(layer==LayerType::LIMIT)
			floor->GetRenderObject()->SetColour(Vector4(0.2f, 0.5f, 0.2f, 0));
	}
	if (handleAsSprings)
	{
		floor->GetPhysicsObject()->SetHandleLikeSpring(true);
		floor->GetPhysicsObject()->SetStiffness(200.f);
		floor->GetPhysicsObject()->SetHandleLikeImpulse(false);
	}
	world->AddGameObject(floor);

	return floor;
}

GameObject* TutorialGame::AddSphereToWorld(const Vector3& position, float radius, float elasticity, float inverseMass, LayerType layer, string name) {
	GameObject* sphere = new GameObject(name);
	sphere->SetLayer(layer);

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);
	sphere->GetTransform().SetWorldScale(sphereSize);
	sphere->GetTransform().SetWorldPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);

	if (name != "bonusItem") {
		sphere->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));
	}
	else {
		sphere->GetRenderObject()->SetColour(Vector4(1, 0, 1, 1));
	}
	sphere->GetPhysicsObject()->InitSphereInertia(true);
	
	sphere->GetPhysicsObject()->SetElasticity(elasticity);
	sphere->GetPhysicsObject()->SetStiffness(100.f);

	world->AddGameObject(sphere);

	return sphere;
}

void TutorialGame::AddBasket(){

	AddCubeToWorld(Vector3(-3.6f, 1, -86.5f), Vector3(1.1f, 0.05f, 1.1f), 0, 0, Vector4(0.05f, 0.05f, 0.05f, 1));	// island nest box
	AddCubeToWorld(Vector3(-3.6f, 1, -85.3f), Vector3(1.1f, 0.5f, 0.1f), 0, 0, Vector4(0.1f, 0.1f, 0.1f, 1));	// island nest box
	AddCubeToWorld(Vector3(-3.6f, 1, -87.7f), Vector3(1.1f, 0.5f, 0.1f), 0, 0, Vector4(0.1f, 0.1f, 0.1f, 1));	// island nest box
	AddCubeToWorld(Vector3(-4.6f, 1, -86.5f), Vector3(0.1f, 0.5f, 1.1f), 0, 0, Vector4(0.1f, 0.1f, 0.1f, 1));	// island nest box
	AddCubeToWorld(Vector3(-2.6f, 1, -86.5f), Vector3(0.1f, 0.5f, 1.1f), 0, 0, Vector4(0.1f, 0.1f, 0.1f, 1));	// island nest box
}

void TutorialGame::AddFences() {
	AddCubeToWorld(Vector3(-41, 3, -197), Vector3(3, 3, 14), 0, 0, Vector4(0.1f, 0.1f, 0.1f, 1)); //fence
	AddCubeToWorld(Vector3(-34, 3, -180), Vector3(10, 3, 3), 0, 0, Vector4(0.1f, 0.1f, 0.1f, 1)); //fence
	AddCubeToWorld(Vector3(-18, 3, -180.2f), Vector3(6, 3, 0.8f), 0, 0, Vector4(0.3f, 0.1f, 0.05f, 0.3f)); //door
	AddCubeToWorld(Vector3(-2, 3, -180), Vector3(10, 3, 3), 0, 0, Vector4(0.1f, 0.1f, 0.1f, 1)); //fence
	AddCubeToWorld(Vector3(18, 3, -180), Vector3(10, 3, 3), 0, 0, Vector4(0.1f, 0.1f, 0.1f, 1)); //fence
	AddCubeToWorld(Vector3(38, 3, -180), Vector3(10, 3, 3), 0, 0, Vector4(0.1f, 0.1f, 0.1f, 1)); //fence
	AddCubeToWorld(Vector3(54, 3, -180), Vector3(6, 3, 3), 0, 0, Vector4(0.1f, 0.1f, 0.1f, 1)); //fence
	AddCubeToWorld(Vector3(-22, 1, -175), Vector3(1.5f, 1.5f, 1.5f), 0, 0.7f, Vector4(1, 1, 0.0f, 1));	//hay block
	AddCubeToWorld(Vector3(-10, 1, -175), Vector3(1.5f, 1.5f, 1.5f), 0, 0.7f, Vector4(1, 1, 0.0f, 1));	//hay block
	AddCubeToWorld(Vector3(2, 1, -175), Vector3(1.5f, 1.5f, 1.5f), 0, 0.7f, Vector4(1, 1, 0.0f, 1));	//hay block
	
	AddCubeToWorld(Vector3(-15, 3, -10), Vector3(5, 3, 1), 0, 0, Vector4(0.1f, 0.1f, 0.1f, 1));	//fence
	AddCubeToWorld(Vector3(-20, 3, -10), Vector3(1, 3, 5), 0, 0, Vector4(0.1f, 0.1f, 0.1f, 1));	//fence
}

void TutorialGame::SpawnApples(int amount, float dt){
	if (world->GetEnableAppleThrower() || player->GetApplesToBeSpawned()>0) {
		if (dt * 1000.0f > 7) {
			if (repetitions % 5 == 0) {
				int random = rand() % 5000 + 1000;
				GameObject* appleInstance = AddAppleToWorld(appleThrowerPos + Vector3(0, 0, 0));
				appleInstance->GetPhysicsObject()->AddForce(Vector3(random * 3.0f, random * 1.3f, random * (repetitions / 13.f)));
				++itemsSpawned;
				if (player->GetApplesToBeSpawned() > 0) {
					player->SetApplesToBeSpawned(player->GetApplesToBeSpawned() - 1);
				}
			}
			++repetitions;
			if (repetitions > amount * 5) {
				world->SetEnableAppleThrower(false);
				repetitions = 0;
			}
		}
	}
}

void TutorialGame::ReSpawnItems(int amount, bool isApple) {
	if (isApple) {
		for (int i = 0; i < amount; ++i) {
			int random = rand() % 3000 + 1000;
			GameObject* appleInstance = AddAppleToWorld(appleThrowerPos + Vector3(0, 0, 0));
			appleInstance->GetPhysicsObject()->AddForce(Vector3(random, 1000, random));
			++itemsSpawned;
		}
		player->SetApplesToBeSpawned(0);
	}
	else {
		for (int i = 0; i < amount; ++i) {
			AddSphereToWorld(player->GetRespawningPositions()[i], 1.0f, 1, 0, LayerType::OBJECT, "bonusItem");
			++itemsSpawned;
		}
		player->SetBonusItemsToBeSpawned(0);
	}
}

GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float elasticity, float inverseMass, Vector4 colour, bool isAppleThrower) {
	GameObject* cube = new GameObject("cube");
	cube->SetLayer(LayerType::CUBE);

	AABBVolume* volume = new AABBVolume(dimensions);
	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform().SetWorldPosition(position);
	cube->GetTransform().SetWorldScale(dimensions);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetElasticity(elasticity);
	if (elasticity > 0.5f) {
		cube->GetRenderObject()->SetColour(Vector4(0, 0, 1, 1));
	}else {
		cube->GetRenderObject()->SetColour(Vector4(0, 1, 1, 1));
	}
	if (colour != Vector4(1, 1, 1, 1)) {
		cube->GetRenderObject()->SetColour(colour);
	}
	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();
	cube->GetPhysicsObject()->SetStiffness(8.f);
	world->AddGameObject(cube);

	return cube;
}

GameObject* TutorialGame::AddGooseToWorld(const Vector3& position)
{
	float size			= 1.0f;
	float inverseMass	= 1.0f;
	gooseInitPos = position;
	GameObject* goose = new GameObject("goose");
	goose->SetLayer(LayerType::PLAYER);

	SphereVolume* volume = new SphereVolume(size);
	goose->SetBoundingVolume((CollisionVolume*)volume);

	goose->GetTransform().SetWorldScale(Vector3(size,size,size) );
	goose->GetTransform().SetWorldPosition(position);

	goose->SetRenderObject(new RenderObject(&goose->GetTransform(), gooseMesh, nullptr, basicShader));
	goose->SetPhysicsObject(new PhysicsObject(&goose->GetTransform(), goose->GetBoundingVolume()));

	goose->GetPhysicsObject()->SetInverseMass(inverseMass);
	goose->GetPhysicsObject()->InitSphereInertia(false);
	goose->GetPhysicsObject()->SetStiffness(300.f);
	goose->GetPhysicsObject()->SetHandleLikeImpulse(true);
	goose->GetPhysicsObject()->SetHandleLikeSpring(true);
	world->AddGameObject(goose);

	player = goose;

	return goose;
}

AIAgent* TutorialGame::AddParkKeeperToWorld(const Vector3& position)
{
	float meshSize = 4.0f;
	float inverseMass = 0.1f;

	AIAgent* keeper = new AIAgent(position, "keeper");
	keeper->SetLayer(LayerType::ENEMY);

	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
	keeper->SetBoundingVolume((CollisionVolume*)volume);

	keeper->GetTransform().SetWorldScale(Vector3(meshSize, meshSize, meshSize));
	keeper->GetTransform().SetWorldPosition(position);

	keeper->SetRenderObject(new RenderObject(&keeper->GetTransform(), keeperMesh, nullptr, basicShader));
	keeper->SetPhysicsObject(new PhysicsObject(&keeper->GetTransform(), keeper->GetBoundingVolume()));

	keeper->GetPhysicsObject()->SetInverseMass(inverseMass);
	keeper->GetPhysicsObject()->InitCubeInertia();

	keeper->GetPhysicsObject()->SetStiffness(200.f);
	keeper->GetPhysicsObject()->SetHandleLikeImpulse(true);
	keeper->GetPhysicsObject()->SetHandleLikeSpring(true);

	keeper->GetRenderObject()->SetColour(Vector4(0.1f, 0.7f, 0.7f, 1));

	keeper->SetWorld(world);
	world->AddGameObject(keeper);
	parkKeeper = keeper;
	return keeper;
}

GameObject* TutorialGame::AddCharacterToWorld(const Vector3& position) {
	float meshSize = 4.0f;
	float inverseMass = 0.5f;

	auto pos = keeperMesh->GetPositionData();

	Vector3 minVal = pos[0];
	Vector3 maxVal = pos[0];

	for (auto& i : pos) {
		maxVal.y = max(maxVal.y, i.y);
		minVal.y = min(minVal.y, i.y);
	}

	GameObject* character = new GameObject("character");
	character->SetLayer(LayerType::ENEMY);
	float r = rand() / (float)RAND_MAX;


	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform().SetWorldScale(Vector3(meshSize, meshSize, meshSize));
	character->GetTransform().SetWorldPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), r > 0.5f ? charA : charB, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(character);

	return character;
}

GameObject* TutorialGame::AddAppleToWorld(const Vector3& position) {
	GameObject* apple = new GameObject("apple");
	apple->SetLayer(LayerType::OBJECT);

	SphereVolume* volume = new SphereVolume(0.7f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform().SetWorldScale(Vector3(4, 4, 4));
	apple->GetTransform().SetWorldPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), appleMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia(false);
	apple->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));

	world->AddGameObject(apple);

	return apple;
}

void TutorialGame::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius, const Vector3& positionTranslation) {
	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing) + positionTranslation;
			if (rand() % 2) {
				AddSphereToWorld(position, radius, 1.0f, 1.0f); // highly elastic material (like a rubber ball) 
			}
			else {
				AddSphereToWorld(position, radius, 0.01f, 1.0f); // low elasticity material (like steel)
			}
		}
	}
}


void TutorialGame::InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims, const Vector3& positionTranslation) {
	for (int x = 1; x < numCols+1; ++x) {
		for (int z = 1; z < numRows+1; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing) + positionTranslation;
			if (rand() % 2) {
				AddCubeToWorld(position, cubeDims, 0.99f, 1.0f); // highly elastic material (like a rubber ball) 
			}
			else {
				AddCubeToWorld(position, cubeDims, 0.02f, 1.0f); // low elasticity material (like steel)
			}
		}
	}
}

void TutorialGame::BridgeConstraintTest() {
	Vector3 cubeSize = Vector3(8, 8, 8);

	float	invCubeMass = 5;
	int		numLinks	= 25;
	float	maxDistance	= 30;
	float	cubeDistance = 20;

	Vector3 startPos = Vector3(0, 5, 0);

	GameObject* start = AddCubeToWorld(startPos + Vector3(0, 0, 0), cubeSize, 0);

	GameObject* end = AddCubeToWorld(startPos + Vector3((numLinks + 2) * cubeDistance, 0, 0), cubeSize, 0);

	GameObject* previous = start;

	for (int i = 0; i < numLinks; ++i) {
		GameObject* block = AddCubeToWorld(startPos + Vector3((i + 1) * cubeDistance, 0, 0), cubeSize, invCubeMass);
		PositionConstraint* constraint = new PositionConstraint(previous, block, maxDistance);
		world->AddConstraint(constraint);
		previous = block;
	}

	PositionConstraint* constraint = new PositionConstraint(previous, end, maxDistance);
	world->AddConstraint(constraint);
}

void TutorialGame::SimpleGJKTest() {
	Vector3 dimensions		= Vector3(5, 5, 5);
	Vector3 floorDimensions = Vector3(100, 2, 100);

	GameObject* fallingCube = AddCubeToWorld(Vector3(0, 20, 0), dimensions, 10.0f);
	GameObject* newFloor	= AddCubeToWorld(Vector3(0, 0, 0), floorDimensions, 0.0f);

	delete fallingCube->GetBoundingVolume();
	delete newFloor->GetBoundingVolume();

	fallingCube->SetBoundingVolume((CollisionVolume*)new OBBVolume(dimensions));
	newFloor->SetBoundingVolume((CollisionVolume*)new OBBVolume(floorDimensions));

}



void InitializeStateMachine() {
	StateMachine* stateMachine = new StateMachine();
	int someData = 0;

	StateFunc AFunc = [](void* data) {
		int* realData = (int*)data;
		(*realData)++;
		std::cout << "In State A!" << std::endl;
	};
	StateFunc BFunc = [](void* data) {
		int* realData = (int*)data;
		(*realData)--;
		std::cout << "In State B!" << std::endl;
	};

	GenericState* stateA = new GenericState(AFunc, (void*)&someData);
	GenericState* stateB = new GenericState(BFunc, (void*)&someData);
	stateMachine->AddState(stateA);
	stateMachine->AddState(stateB);

	GenericTransition<int&, int>* transitionA =
		new GenericTransition<int&, int>(GenericTransition<int&, int>::GreaterThanTransition, someData, 10, stateA, stateB); // if greater than 10 , A to B

	GenericTransition<int&, int>* transitionB =
		new GenericTransition<int&, int>(GenericTransition<int&, int>::EqualsTransition, someData, 0, stateB, stateA); // if equals 0 , B to A

	stateMachine->AddTransition(transitionA);
	stateMachine->AddTransition(transitionB);

	for (int i = 0; i < 100; ++i) {
		stateMachine->Update(); // run the state machine !
	}
	delete stateMachine;
}
