#include "NetworkedGame.h"
#include "NetworkPlayer.h"
#include "../CSC8503Common/GameServer.h"
#include "../CSC8503Common/GameClient.h"

#define COLLISION_MSG 30

void NetworkedGame::UpdateAsClient(float dt) {

	ClientPacket newPacket;

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::HOME)) {
		// fire button pressed !
		newPacket.buttonstates[0] = 1;
		newPacket.lastID = 0; // You ’ll need to work this out somehow ...
	}
	thisClient->SendPacket(newPacket);
}


/*  There’s currently no way for the server to know which packets have been successfully received by the client
and acknowledged - you could perhaps store this inside a map between a player number and an int, or directly inside 
some sort of ’Player’ class. This will then be used in place of the playerState variable on line 22. The other problem 
is that the server shouldn’t send the same data to every client (line 25) - you will need a method that can send a 
specific packet to a specific player, and then put the for loop starting on line 17 inside another for loop iterating over each player, so that
unique packets can be sent to each player based on what last state they received and acknowledged. */

/* GUIDES: Remember that when the eNet library receives an event (as in the GameServer::UpdateServer
method), it can determine a particular peer object - it you store this peer object when an event of
type ENET EVENT TYPE CONNECT is received, you can then gain a mapping of player to
eNet peer, which is pretty handy for then using in a server call to enet peer send to send a packet
to a particular player. */
void NetworkedGame::BroadcastSnapshot(bool deltaFrame) {

	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;

	world -> GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i) {
		NetworkObject* o = (*i)->GetNetworkObject();
		if (!o) {
			continue;
		}
		int playerState = 0; // You ’ll need to do this bit !
		GamePacket* newPacket = nullptr;
		if (o->WritePacket(&newPacket, deltaFrame, playerState)) {
			thisServer->SendGlobalPacket(*newPacket); // change ...
			delete newPacket;
		}
	}
}
