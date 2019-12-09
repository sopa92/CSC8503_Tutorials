#include "NetworkObject.h"

using namespace NCL;
using namespace CSC8503;

NetworkObject::NetworkObject(GameObject& o, int id) : object(o)	{
	deltaErrors = 0;
	fullErrors  = 0;
	networkID   = id;
}

NetworkObject::~NetworkObject()	{
}

bool NetworkObject::ReadPacket(GamePacket& p) {
	if (p.type == Delta_State) {
		return ReadDeltaPacket((DeltaPacket&)p);
	}
	if (p.type == Full_State) {
		return ReadFullPacket((FullPacket&)p);
	}
	return false; //this isn't a packet we care about!
}

bool NetworkObject::WritePacket(GamePacket** p, bool deltaFrame, int stateID) {	// the server tells a NetworkObject to write a new GamePacket containing its state
	if (deltaFrame) {
		if (!WriteDeltaPacket(p, stateID)) {
			return WriteFullPacket(p);
		}
	}
	return WriteFullPacket(p);
}
//Client objects recieve these packets
bool NetworkObject::ReadDeltaPacket(DeltaPacket &p) {
	if (p.fullID != lastFullState.stateID) {	// we examine the fullID, and see whether its the same as the last accepted ’full’ state
		deltaErrors++; //can't delta this frame
		return false;
	}

	UpdateStateHistory(p.fullID);	//we dump any old NetworkState objects that the NetworkObject is holding

	Vector3		fullPos			= lastFullState.position;
	Quaternion  fullOrientation = lastFullState.orientation;

	// reconstitute the change in position and orientation
	fullPos.x += p.pos[0];
	fullPos.y += p.pos[1];
	fullPos.z += p.pos[2];

	// we now divide the quaternion	bytes by 127 and turn them back into floats
	fullOrientation.x += ((float)p.orientation[0]) / 127.0f;
	fullOrientation.y += ((float)p.orientation[1]) / 127.0f;
	fullOrientation.z += ((float)p.orientation[2]) / 127.0f;
	fullOrientation.w += ((float)p.orientation[3]) / 127.0f;

	object.GetTransform().SetWorldPosition(fullPos);
	object.GetTransform().SetLocalOrientation(fullOrientation);

	return true;
}

bool NetworkObject::ReadFullPacket(FullPacket &p) {
	if (p.fullState.stateID < lastFullState.stateID) {	//make sure that it wasn’t an old, or delayed packet, that contains an out of date NetworkState
		return false; // received an 'old' packet, ignore!
	}
	lastFullState = p.fullState;

	object.GetTransform().SetWorldPosition(lastFullState.position);
	object.GetTransform().SetLocalOrientation(lastFullState.orientation);

	stateHistory.emplace_back(lastFullState);

	return true;
}

bool NetworkObject::WriteDeltaPacket(GamePacket**p, int stateID) {	//the server will try to write a delta packet against an existing NetworkState, selected by the stateID variable
	DeltaPacket* dp = new DeltaPacket();

	dp->objectID = networkID;	//which object this is a delta of

	NetworkState state;
	if (!GetNetworkState(stateID, state)) {	// if this NetworkObject doesn’t have that particular existing state
		return false; //can't delta!
	}

	dp->fullID = stateID;	// what state this is a delta of

	Vector3		currentPos			= object.GetTransform().GetWorldPosition();
	Quaternion  currentOrientation  = object.GetTransform().GetWorldOrientation();

	/* work out the difference between the current game state’s position and orientation, and subtract the 
	selected states variables, giving us the difference since that previously written state */
	currentPos			-= state.position;
	currentOrientation  -= state.orientation;

	// to efficiently represent the change in position and orientation, we store just a single byte per field 
	// (we just hope the object has moved less than 127 units on any particular axis)
	dp->pos[0] = (char)currentPos.x;
	dp->pos[1] = (char)currentPos.y;
	dp->pos[2] = (char)currentPos.z;

	// since each variable inside the quaternion will be within the range -1 to 1, we can expand that out to the full range of a byte by multiplying it by 127, so that it’s now -127 to 127
	dp->orientation[0] = (char)(currentOrientation.x * 127.0f);
	dp->orientation[1] = (char)(currentOrientation.y * 127.0f);
	dp->orientation[2] = (char)(currentOrientation.z * 127.0f);
	dp->orientation[3] = (char)(currentOrientation.w * 127.0f);

	*p = dp;
	return true;
}

bool NetworkObject::WriteFullPacket(GamePacket**p) {
	FullPacket* fp = new FullPacket();	//we’re creating a new ’snapshot’ of the whole world, which individual clients can then try and catch up to

	fp->objectID				= networkID;
	fp->fullState.position		= object.GetTransform().GetWorldPosition();
	fp->fullState.orientation	= object.GetTransform().GetWorldOrientation();
	fp->fullState.stateID		= lastFullState.stateID++;	//The packet gets an increasing stateID integer

	*p = fp;
	return true;
}

// get the latest state received from a server
NetworkState& NetworkObject::GetLatestNetworkState() {
	return lastFullState;
}

// get a particular saved state on either the client or server side
bool NetworkObject::GetNetworkState(int stateID, NetworkState& state) {
	for (auto i = stateHistory.begin(); i < stateHistory.end(); ++i) {
		if ((*i).stateID == stateID) {
			state = (*i);
			return true;
		}
	}
	return false;
}

/* As the client receives NetworkStates, it stores them in the stateHistroy vector temporarily, as
it may need them later, to calculate a ’delta’ from. When a client accepts a delta packet, or the
server receives acknowledgement from a client that a NetworkState has been received, we can get
rid of old NetworkState objects, based on their stateID variable. We can do so by iterating over
the stateHistory vector, and removing any entries that have a stateID less than some parameter value. */
void NetworkObject::UpdateStateHistory(int minID) {
	for (auto i = stateHistory.begin(); i < stateHistory.end(); ) {
		if ((*i).stateID < minID) {
			i = stateHistory.erase(i);
		}
		else {
			++i;
		}
	}
}