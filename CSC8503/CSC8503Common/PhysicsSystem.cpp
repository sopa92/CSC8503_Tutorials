#include "PhysicsSystem.h"
#include "PhysicsObject.h"
#include "GameObject.h"
#include "CollisionDetection.h"
#include "../../Common/Quaternion.h"

#include "Constraint.h"

#include "Debug.h"

#include <functional>
using namespace NCL;
using namespace CSC8503;

PhysicsSystem::PhysicsSystem(GameWorld& g) : gameWorld(g)	{
	applyGravity	= false;
	useBroadPhase	= true;	
	dTOffset		= 0.0f;
	globalDamping	= 0.95f;
	SetGravity(Vector3(0.0f, -9.8f, 0.0f));
}

PhysicsSystem::~PhysicsSystem()	{
}

void PhysicsSystem::SetGravity(const Vector3& g) {
	gravity = g;
}

void PhysicsSystem::CollectObject(GameObject* collectable) {	
	++gameWorld.collectedObjects;
	gameWorld.carryingObjects.push_back(collectable);
	collectable->SetAsCollected(true);
}

/*

If the 'game' is ever reset, the PhysicsSystem must be
'cleared' to remove any old collisions that might still
be hanging around in the collision list. If your engine
is expanded to allow objects to be removed from the world,
you'll need to iterate through this collisions list to remove
any collisions they are in.

*/
void PhysicsSystem::Clear() {
	allCollisions.clear();
}

/*

This is the core of the physics engine update

*/
void PhysicsSystem::Update(float dt) {
	GameTimer testTimer;
	testTimer.GetTimeDeltaSeconds();

	frameDT = dt;

	dTOffset += dt; //We accumulate time delta here - there might be remainders from previous frame!

	float iterationDt = 1.0f / 120.0f; //Ideally we'll have 120 physics updates a second 

	//if (dTOffset > 8 * iterationDt) { //the physics engine cant catch up!
	//	iterationDt = 1.0f / 15.0f; //it'll just have to run bigger timesteps...
	//	//std::cout << "Setting physics iterations to 15" << iterationDt << std::endl;
	//}
	//else if (dTOffset > 4  * iterationDt) { //the physics engine cant catch up!
	//	iterationDt = 1.0f / 30.0f; //it'll just have to run bigger timesteps...
	//	//std::cout << "Setting iteration dt to 4 case " << iterationDt << std::endl;
	//}
	//else if (dTOffset > 2* iterationDt) { //the physics engine cant catch up!
	//	iterationDt = 1.0f / 60.0f; //it'll just have to run bigger timesteps...
	//	//std::cout << "Setting iteration dt to 2 case " << iterationDt << std::endl;
	//}
	//else {
	//	//std::cout << "Running normal update " << iterationDt << std::endl;
	//}

	int constraintIterationCount = 10;
	//iterationDt = dt;

	if (useBroadPhase) {
		UpdateObjectAABBs();
	}

	while(dTOffset > iterationDt *0.5) {
		IntegrateAccel(iterationDt); //Update accelerations from external forces
		if (useBroadPhase) {
			BroadPhase();
			NarrowPhase();
		}
		else {
			BasicCollisionDetection();
		}

		//This is our simple iterative solver - 
		//we just run things multiple times, slowly moving things forward
		//and then rechecking that the constraints have been met		
		float constraintDt = iterationDt /  (float)constraintIterationCount;

		for (int i = 0; i < constraintIterationCount; ++i) {
			UpdateConstraints(constraintDt);	
		}
		
		IntegrateVelocity(iterationDt); //update positions from new velocity changes

		dTOffset -= iterationDt; 
	}
	ClearForces();	//Once we've finished with the forces, reset them to zero
	
	UpdateCollisionList(); //Remove any old collisions
	//std::cout << iteratorCount << " , " << iterationDt << std::endl;
	float time = testTimer.GetTimeDeltaSeconds();
	//std::cout << "Physics time taken: " << time << std::endl;
}

/*
Later on we're going to need to keep track of collisions
across multiple frames, so we store them in a set.

The first time they are added, we tell the objects they are colliding.
The frame they are to be removed, we tell them they're no longer colliding.

From this simple mechanism, we we build up gameplay interactions inside the
OnCollisionBegin / OnCollisionEnd functions (removing health when hit by a 
rocket launcher, gaining a point when the player hits the gold coin, and so on).
*/
void PhysicsSystem::UpdateCollisionList() {

	for (std::set<CollisionDetection::CollisionInfo>::iterator i = allCollisions.begin(); i != allCollisions.end(); ) {
		if ((*i).framesLeft == numCollisionFrames) {
			i->a->OnCollisionBegin(i->b);
			i->b->OnCollisionBegin(i->a);
		}
		(*i).framesLeft = (*i).framesLeft - 1;
		if ((*i).framesLeft < 0) {
			i->a->OnCollisionEnd(i->b);	// when an object has stopped colliding with something
			i->b->OnCollisionEnd(i->a);
			i = allCollisions.erase(i);
		}
		else {
			++i;
		}
	}
}

void PhysicsSystem::UpdateObjectAABBs() {	//this will let the PhysicsSystem see the latest correct bounding boxes for the game objects every frame
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i) {
		(*i)->UpdateBroadphaseAABB();
	}
}

/* This is how we'll be doing collision detection in tutorial 4. We step thorugh every pair of objects once (the inner for loop offset 
ensures this), and determine whether they collide, and if so, add them to the collision set for later processing. The set will guarantee that
a particular pair will only be added once, so objects colliding for multiple frames won't flood the set with duplicates. */
void PhysicsSystem::BasicCollisionDetection() {
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i) {
		if ((*i)->GetPhysicsObject() == nullptr) {
			continue;
		}
		// we are starting the inner for loop at one element past the outer for loop, so that we don't resolve identical collisions multiple times in a frame
		for (auto j = i + 1; j != last; ++j) {
			if ((*j)->GetPhysicsObject() == nullptr) {
				continue;
			}
			CollisionDetection::CollisionInfo info;

			if (CollisionDetection::ObjectIntersection(*i, *j, info)) {	// will return true if a collision has taken place
				
				//std::cout << " Collision between " << (*i)->GetName() << " and " << (*j)->GetName() << std::endl;
				//will add the impulses to instantaneously change our linear and angular velocity such that the objects will move apart
				PhysicsObject* physA = info.a->GetPhysicsObject();
				PhysicsObject* physB = info.b->GetPhysicsObject();

				
				ImpulseResolveCollision(*info.a, *info.b, info.point);
				
				info.framesLeft = numCollisionFrames;
				allCollisions.insert(info);	// we insert the successfully detected collision into an STL::List,	which will let us keep track of objects that are colliding
			}
		}
	}
}

void PhysicsSystem::ResolveSpringCollision(GameObject& a, GameObject& b, CollisionDetection::ContactPoint& p) const {

	if (a.GetPhysicsObject()->GetInverseMass() == 0 && b.GetPhysicsObject()->GetInverseMass() == 0)
		return;
	PhysicsObject* physA = a.GetPhysicsObject();
	PhysicsObject* physB = b.GetPhysicsObject();

	// model spring with resting length of 0, attached to collision points of the collision
	const Vector3 springPositionA = p.localA;
	const Vector3 springPositionB = p.localB;
	
	// temporary spring being extended along the collision normal n by penetration distance p
	const Vector3 springExtendDirection = p.normal;
	const float springExtendLength = p.penetration;

	Vector3 springExtension = springExtendDirection * springExtendLength;

	// apply force proportional to penetration distance, at collision point on each object, in direction of collision normal
	const Vector3 forceOnObjectA = -springExtension * physB->GetStiffness();
	physA->AddForceAroundPosition(forceOnObjectA, springPositionA);
	const Vector3 forceOnObjectB = springExtension * physA->GetStiffness();
	physB->AddForceAroundPosition(forceOnObjectB, springPositionB);
}

/*
In tutorial 5, we start determining the correct response to a collision,
so that objects separate back out. 
*/
void PhysicsSystem::ImpulseResolveCollision(GameObject& a, GameObject& b, CollisionDetection::ContactPoint& p) const {
	PhysicsObject* physA = a.GetPhysicsObject();
	PhysicsObject* physB = b.GetPhysicsObject();

	Transform& transformA = a.GetTransform();
	Transform& transformB = b.GetTransform();

	float totalMass = physA->GetInverseMass() + physB->GetInverseMass();	// determine the total inverse mass of the two objects (use this to calculate impulse J, and simple object projection)
	if (totalMass == 0.0f) {
		return;
	}
	
	// Separate them out using projection
	/* we push each object along the collision normal, by an amount proportional to the penetration distance, and the object’s inverse mass.
	By dividing each object’s mass by the totalMass variable, we make it such that between the two calculations, the object’s move by a total
	amount of p.penetration away from each other, but a ’heavier’ object will move by less, as it makes up less of the "total mass" */
	transformA.SetWorldPosition(transformA.GetWorldPosition() - (p.normal * p.penetration * (physA->GetInverseMass() / totalMass)));
	transformB.SetWorldPosition(transformB.GetWorldPosition() + (p.normal * p.penetration * (physB->GetInverseMass() / totalMass)));

	// collision points relative to each object’s position
	Vector3 relativeA = p.localA;
	Vector3 relativeB = p.localB;

	// we determine how much the object is moving via the cross product, much in the same way as we determine the amount of torque using the cross product when applying angular forces
	Vector3 angVelocityA = Vector3::Cross(physA->GetAngularVelocity(), relativeA);
	Vector3 angVelocityB = Vector3::Cross(physB->GetAngularVelocity(), relativeB);

	// we determine the velocities at which the two objects are colliding, via simple addition
	Vector3 fullVelocityA = physA->GetLinearVelocity() + angVelocityA;
	Vector3 fullVelocityB = physB->GetLinearVelocity() + angVelocityB;

	// we determine the relative velocity of the collision
	Vector3 contactVelocity = fullVelocityB - fullVelocityA;


	float impulseForce = Vector3::Dot(contactVelocity, p.normal);
	if (impulseForce > 0) {
		return;
	}

	// now to work out the effect of inertia ....
	Vector3 inertiaA = Vector3::Cross(physA->GetInertiaTensor() * Vector3::Cross(relativeA, p.normal), relativeA);
	Vector3 inertiaB = Vector3::Cross(physB->GetInertiaTensor() * Vector3::Cross(relativeB, p.normal), relativeB);
	float angularEffect = Vector3::Dot(inertiaA + inertiaB, p.normal);

	float cRestitution = physA->GetElasticity() * physB->GetElasticity(); //0.66f; // disperse some kinectic energy

	float j = (-(1.0f + cRestitution) * impulseForce) / (totalMass + angularEffect);

	Vector3 fullImpulse = p.normal * j;

	// apply the linear and angular impulses, in opposite directions
	physA->ApplyLinearImpulse(-fullImpulse);
	physB->ApplyLinearImpulse(fullImpulse);
	
	physA->ApplyAngularImpulse(Vector3::Cross(relativeA, -fullImpulse));
	physB->ApplyAngularImpulse(Vector3::Cross(relativeB, fullImpulse));
}

/*

Later, we replace the BasicCollisionDetection method with a broadphase
and a narrowphase collision detection method. In the broad phase, we
split the world up using an acceleration structure, so that we can only
compare the collisions that we absolutely need to. 

*/

void PhysicsSystem::BroadPhase() {

	broadphaseCollisions.clear();
	QuadTree<GameObject*> tree(Vector2(1024, 1024), 7, 6);
	
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);
	for (auto i = first; i != last; ++i) {	// iterating through all of the objects in the game world, Inserting them into the tree one after another
		Vector3 halfSizes;
		if (!(*i)->GetBroadphaseAABB(halfSizes)) {
			continue;
		}
		Vector3 pos = (*i)->GetConstTransform().GetWorldPosition();
		tree.Insert(*i, pos, halfSizes);
	}

	tree.OperateOnContents([&](std::list<QuadTreeEntry<GameObject*>>& data) {
		CollisionDetection::CollisionInfo info;
		for (auto i = data.begin(); i != data.end(); ++i) {
			for (auto j = std::next(i); j != data.end(); ++j) {
				// is this pair of items already in the collision set -
				// if the same pair is in another quadtree node together etc
				info.a = min((*i).object, (*j).object);
				info.b = max((*i).object, (*j).object);
				broadphaseCollisions.insert(info);
			}
		}
		});
}

/*
The broadphase will now only give us likely collisions, so we can now go through them,
and work out if they are truly colliding, and if so, add them into the main collision list
*/
void PhysicsSystem::NarrowPhase() {
	for (std::set<CollisionDetection::CollisionInfo>::iterator 	i = broadphaseCollisions.begin(); i != broadphaseCollisions.end(); ++i) {
		CollisionDetection::CollisionInfo info = *i;
		if (CollisionDetection::ObjectIntersection(info.a, info.b, info)) {
			if (info.a->IsActive() && info.b->IsActive()) {
				
				if (info.a->GetLayer() == LayerType::PLAYER && info.b->GetLayer() == LayerType::OBJECT) {
					CollectObject(info.b);
				}
				else if (info.b->GetLayer() == LayerType::PLAYER && info.a->GetLayer() == LayerType::OBJECT) {
					CollectObject(info.a);
				}
				if (info.a->GetLayer() == LayerType::PLAYER && info.b->GetLayer() == LayerType::ENEMY ||
					info.b->GetLayer() == LayerType::PLAYER && info.a->GetLayer() == LayerType::ENEMY) {
					int numberOfCarryingObjects = gameWorld.carryingObjects.size();
					int applesToBeSpawned = 0;
					int bonusItemsToBeSpawned = 0;
					if (numberOfCarryingObjects > 0) {						
						for (int i = 0; i < numberOfCarryingObjects; ++i) {
							if (gameWorld.carryingObjects[i]->GetName() == "apple") {
								++applesToBeSpawned;
							}
							else {
								++bonusItemsToBeSpawned;
								gameWorld.respawningPositions.push_back(gameWorld.carryingObjects[i]->GetTransform().GetWorldPosition());
							}
						}
						gameWorld.SetBonusItemsToBeSpawned(bonusItemsToBeSpawned);
						gameWorld.SetApplesToBeSpawned(applesToBeSpawned);		
						gameWorld.carryingObjects.clear();
						gameWorld.collectedObjects -= applesToBeSpawned + bonusItemsToBeSpawned;
					}
				}
				
				info.framesLeft = numCollisionFrames;
				/*if (info.a->GetLayer() == LayerType::WATER && info.b->GetLayer() == LayerType::ENEMY ||
					info.b->GetLayer() == LayerType::WATER && info.a->GetLayer() == LayerType::ENEMY) {
					ImpulseResolveCollision(*info.a, *info.b, info.point);
				}
				else */
					if ((info.a->GetPhysicsObject()->GetHandleLikeSpring() || info.b->GetPhysicsObject()->GetHandleLikeSpring()) && (
					(info.a->GetLayer() == LayerType::WATER && info.b->GetLayer() != LayerType::FLOOR) ||
					(info.b->GetLayer() == LayerType::WATER && info.a->GetLayer() != LayerType::FLOOR)
					))
					ResolveSpringCollision(*info.a, *info.b, info.point);
				else 
					ImpulseResolveCollision(*info.a, *info.b, info.point);
				allCollisions.insert(info); // insert into our main set
			}
		}
	}
}

/*
Integration of acceleration and velocity is split up, so that we can
move objects multiple times during the course of a PhysicsUpdate,
without worrying about repeated forces accumulating etc. 

This function will update both linear and angular acceleration,
based on any forces that have been accumulated in the objects during
the course of the previous game frame.
*/
void PhysicsSystem::IntegrateAccel(float dt) {
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i) {
		PhysicsObject* object = (*i)->GetPhysicsObject();
		if (object == nullptr) {
			continue; // No physics object for this GameObject !
		}
		float inverseMass = object->GetInverseMass();
		Vector3 linearVel = object->GetLinearVelocity();
		Vector3 force = object->GetForce();
		Vector3 accel = force * inverseMass;

		if (applyGravity && inverseMass > 0) {
			accel += gravity; // don't move infinitely heavy things
		}
		linearVel += accel * dt; // integrate accel !
		object->SetLinearVelocity(linearVel);

		// Angular stuff
		Vector3 torque = object->GetTorque();
		Vector3 angVel = object->GetAngularVelocity();
		
		object->UpdateInertiaTensor(); // update tensor vs orientation
		
		Vector3 angAccel = object->GetInertiaTensor() * torque;
		
		angVel += angAccel * dt; // integrate angular accel !
		object->SetAngularVelocity(angVel);
	}
}
/*
This function integrates linear and angular velocity into
position and orientation. It may be called multiple times
throughout a physics update, to slowly move the objects through
the world, looking for collisions.
*/
void PhysicsSystem::IntegrateVelocity(float dt) {
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);

	float dampingFactor = 1.0f - 0.95f;
	float frameDamping = powf(dampingFactor, dt);

	for (auto i = first; i != last; ++i) {
		PhysicsObject* object = (*i)->GetPhysicsObject();
		if (object == nullptr) {
			continue;
		}
		Transform& transform = (*i)->GetTransform();
		// Position Stuff
		Vector3 position = transform.GetLocalPosition();
		Vector3 linearVel = object->GetLinearVelocity();
		position += linearVel * dt;
		transform.SetLocalPosition(position);
		transform.SetWorldPosition(position);	
		// Linear Damping
		linearVel = linearVel * frameDamping;
		object->SetLinearVelocity(linearVel);

		// Orientation Stuff
		Quaternion orientation = transform.GetLocalOrientation();
		Vector3 angVel = object->GetAngularVelocity();
		
		orientation = orientation +	(Quaternion(angVel * dt * 0.5f, 0.0f) * orientation);
		orientation.Normalise();
		
		transform.SetLocalOrientation(orientation);
		
		// Damp the angular velocity too
		angVel = angVel * frameDamping;
		object->SetAngularVelocity(angVel);
	}
}

/*
Once we're finished with a physics update, we have to
clear out any accumulated forces, ready to receive new
ones in the next 'game' frame.
*/
void PhysicsSystem::ClearForces() {
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i) {
		//Clear our object's forces for the next frame
		(*i)->GetPhysicsObject()->ClearForces();
	}
}


/*

As part of the final physics tutorials, we add in the ability
to constrain objects based on some extra calculation, allowing
us to model springs and ropes etc. 

*/
void PhysicsSystem::UpdateConstraints(float dt) {
	std::vector<Constraint*>::const_iterator first;
	std::vector<Constraint*>::const_iterator last;
	gameWorld.GetConstraintIterators(first, last);

	for (auto i = first; i != last; ++i) {
		(*i)->UpdateConstraint(dt);
	}
}