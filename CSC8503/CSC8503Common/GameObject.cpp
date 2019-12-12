#include "GameObject.h"
#include "CollisionDetection.h"

using namespace NCL::CSC8503;

GameObject::GameObject(string objectName)	{
	name			= objectName;
	isActive		= true;
	boundingVolume	= nullptr;
	physicsObject	= nullptr;
	renderObject	= nullptr;
	networkObject	= nullptr;
	world = nullptr;
	playerIsInNest = false;
}

GameObject::~GameObject()	{
	delete boundingVolume;
	delete physicsObject;
	delete renderObject;
	delete networkObject;
}

bool GameObject::GetBroadphaseAABB(Vector3&outSize) const {
	if (!boundingVolume) {
		return false;
	}
	outSize = broadphaseAABB;
	return true;
}

//These would be better as a virtual 'ToAABB' type function, really...
void GameObject::UpdateBroadphaseAABB() {
	if (!boundingVolume) {
		return;
	}
	if (boundingVolume->type == VolumeType::AABB) {
		broadphaseAABB = ((AABBVolume&)*boundingVolume).GetHalfDimensions();
	}
	else if (boundingVolume->type == VolumeType::Sphere) {
		float r = ((SphereVolume&)*boundingVolume).GetRadius();
		broadphaseAABB = Vector3(r, r, r);
	}
	else if (boundingVolume->type == VolumeType::OBB) {
		Matrix3 mat = Matrix3(transform.GetWorldOrientation());
		mat = mat.Absolute();
		Vector3 halfSizes = ((OBBVolume&)*boundingVolume).GetHalfDimensions();
		broadphaseAABB = mat * halfSizes;
	}
}

void GameObject::DrawDebug(const Vector4& color)
{
	renderObject->SetColour(color);
}

void GameObject::DrawDebugVolume()
{
	if (!boundingVolume)
		return;

	if (boundingVolume->type == VolumeType::AABB)
	{
		boundingVolume->DrawDebug(transform.GetWorldPosition(), Vector4(1, 0, 0, 1));
	}
	else if (boundingVolume->type == VolumeType::Sphere)
	{
		boundingVolume->DrawDebug(transform.GetWorldPosition(), Vector4(0, 1, 0, 1));
	}	
}

void GameObject::CollectObject(GameObject* collectable){
	++collectedObjects;
	carryingObjects.push_back(collectable);
	collectable->SetAsCollected(true);
}


void GameObject::DropCarryingItems()
{
	int numberOfCarryingObjects = carryingObjects.size();
	int applesToBeSpawned = 0;
	int bonusItemsToBeSpawned = 0;
	if (numberOfCarryingObjects > 0) {
		for (int i = 0; i < numberOfCarryingObjects; ++i) {
			if (carryingObjects[i]->GetName() == "apple") {
				++applesToBeSpawned;
			}
			else {
				++bonusItemsToBeSpawned;
				
				respawningPositions.push_back(carryingObjects[i]->GetTransform().GetWorldPosition());
			}
		}
		SetBonusItemsToBeSpawned(bonusItemsToBeSpawned);
		SetApplesToBeSpawned(applesToBeSpawned);
		carryingObjects.clear();
		collectedObjects -= applesToBeSpawned + bonusItemsToBeSpawned;
	}
}
