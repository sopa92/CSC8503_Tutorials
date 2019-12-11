#include "../../Common/Window.h"

#include "../CSC8503Common/StateMachine.h"
#include "../CSC8503Common/StateTransition.h"
#include "../CSC8503Common/State.h"

#include "../CSC8503Common/GameServer.h"
#include "../CSC8503Common/GameClient.h"

#include "../CSC8503Common/NavigationGrid.h"

#include "TutorialGame.h"
#include "NetworkedGame.h"
#include <fstream>
#include "../../Common/Assets.h"

using namespace NCL;
using namespace CSC8503;

void TestStateMachine() {
	StateMachine* testMachine = new StateMachine();
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
	testMachine->AddState(stateA);
	testMachine->AddState(stateB);

	GenericTransition<int&, int>* transitionA =
		new GenericTransition<int&, int>(GenericTransition<int&, int>::GreaterThanTransition, someData, 10, stateA, stateB); // if greater than 10 , A to B

	GenericTransition<int&, int>* transitionB =
		new GenericTransition<int&, int>(GenericTransition<int&, int>::EqualsTransition, someData, 0, stateB, stateA); // if equals 0 , B to A

	testMachine->AddTransition(transitionA);
	testMachine->AddTransition(transitionB);

	for (int i = 0; i < 100; ++i) {
		testMachine->Update(); // run the state machine !
	}
	delete testMachine;
}

char ShowMenu(int screenWidth, int screenHeight) {

	Debug::Print("Welcome to the GOOSE GAME!", Vector2(screenWidth / 2, screenHeight / 2), Vector4(0, 0, 1, 1));
	Debug::Print("Are you ready to Honk?", Vector2(screenWidth / 2, (screenHeight / 2) - 15), Vector4(0, 0, 1, 1));

	char userInput;
	std::cin >> userInput;
	return userInput;
}

void MyStateMachine(int screenWidth, int screenHeight) {

	StateMachine* testMachine = new StateMachine();
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
	testMachine->AddState(stateA);
	testMachine->AddState(stateB);

	GenericTransition<int&, int>* transitionA =
		new GenericTransition<int&, int>(GenericTransition<int&, int>::GreaterThanTransition, someData, 10, stateA, stateB); // if greater than 10 , A to B

	GenericTransition<int&, int>* transitionB =
		new GenericTransition<int&, int>(GenericTransition<int&, int>::EqualsTransition, someData, 0, stateB, stateA); // if equals 0 , B to A

	testMachine->AddTransition(transitionA);
	testMachine->AddTransition(transitionB);

	for (int i = 0; i < 2000; ++i) {
		testMachine->Update(); // run the state machine !
	}
	delete testMachine;
}

//---------start of NETWORKING------------
class GooseGamePacketReceiver : public PacketReceiver {
public:
	GooseGamePacketReceiver(string name, TutorialGame* g) {
		this->name = name;
	}
	void ReceivePacket(int type, GamePacket* payload, int source) {
		if (type == String_Message) {
			StringPacket* realPacket = (StringPacket*)payload;

			string msg = realPacket->GetStringFromData();
			if (msg == "HUGE") {
				//make the goose huge				
			}
			std::cout << name << " received message: " << msg << std::endl;
		}
	}
protected:
	string name;
};
//
//void TestNetworking() {
//	NetworkBase::Initialise();
//
//	TestPacketReceiver serverReceiver("Server");
//	TestPacketReceiver clientReceiver("Client");
//
//	int port = NetworkBase::GetDefaultPort();
//
//	GameServer* server = new GameServer(port, 1);
//	GameClient* client = new GameClient();
//
//	server->RegisterPacketHandler(String_Message, &serverReceiver);
//	client->RegisterPacketHandler(String_Message, &clientReceiver);
//
//	bool canConnect = client->Connect(127, 0, 0, 1, port);
//
//	for (int i = 0; i < 100; ++i) {
//		server->SendGlobalPacket(
//			StringPacket("Server says hello! " + std::to_string(i)));
//		client->SendPacket(
//			StringPacket("Client says hello! " + std::to_string(i)));
//
//		server->UpdateServer();
//		client->UpdateClient();
//
//		std::this_thread::sleep_for(std::chrono::milliseconds(10));
//	}
//	NetworkBase::Destroy();
//}


void GooseGameNetworking(TutorialGame* g) {
	NetworkBase::Initialise();

	GooseGamePacketReceiver serverReceiver("Server", g);
	GooseGamePacketReceiver clientReceiver("Client", g);

	int port = NetworkBase::GetDefaultPort();

	GameServer* server = new GameServer(port, 1);
	GameClient* client = new GameClient();

	server->RegisterPacketHandler(String_Message, &serverReceiver);
	client->RegisterPacketHandler(String_Message, &clientReceiver);

	bool canConnect = client->Connect(127, 0, 0, 1, port);

	std::ifstream infile(Assets::DATADIR + "CheatCommand.txt");
	string commandFromFile;
	infile >> commandFromFile;

	//if (commandFromFile == "HUGE") {}
	for (int i = 0; i < 100; ++i) {
		server->SendGlobalPacket(StringPacket(commandFromFile));
		
		server->UpdateServer();
		client->UpdateClient();

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	NetworkBase::Destroy();
}
//----------end of NETWORKING-------------

/*
The main function should look pretty familar to you!
We make a window, and then go into a while loop that repeatedly
runs our 'game' until we press escape. Instead of making a 'renderer'
and updating it, we instead make a whole game, and repeatedly update that,
instead. 

This time, we've added some extra functionality to the window class - we can
hide or show the 
*/


void DisplayGrid()
{
	NavigationGrid grid("GridMap.txt");

	for (int i = 0; i < 1225; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (grid.GetAllNodes()[i].connected[j])
			{
				Debug::DrawLine(grid.GetAllNodes()[i].position + Vector3(0, 8, 0), grid.GetAllNodes()[i].connected[j]->position + Vector3(0, 8, 0), Vector4(1, 0, 0, 1));
			}
		}
	}
}

int main() {
	int screenWidth = 1280;
	int screenHeight = 720;
	Window*w = Window::CreateGameWindow("CSC8503 Game technology!", screenWidth, screenHeight);

	if (!w->HasInitialised()) {
		return -1;
	}	

	//TestStateMachine(); //enable state machine
	
	w->ShowOSPointer(false);
	w->LockMouseToWindow(true);
	
	TutorialGame* g = new TutorialGame();

	//GooseGameNetworking(g);

	while (w->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KeyboardKeys::ESCAPE)) {
		float dt = w->GetTimer()->GetTimeDeltaSeconds();

		if (dt > 1.0f) {
			std::cout << "Skipping large time delta" << std::endl;
			continue; //must have hit a breakpoint or something to have a 1 second frame time!
		}
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::PRIOR)) {
			w->ShowConsole(true);
		}
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NEXT)) {
			w->ShowConsole(false);
		}

		//DisplayPathfinding();
		//DisplayGrid();
		w->SetTitle("Gametech frame time:" + std::to_string(1000.0f * dt));

		g->UpdateGame(dt);
	}
	Window::DestroyGameWindow();
}
