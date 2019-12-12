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
#include "GameStateHandler.h"
#include <sstream>

using namespace NCL;
using namespace CSC8503;

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

void DisplayGrid()
{
	NavigationGrid grid("GridMap.txt");

	for (int i = 0; i < 1225; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (grid.GetAllNodes()[i].connected[j])
			{
				Debug::DrawLine(grid.GetAllNodes()[i].position + Vector3(0, 7, 0), grid.GetAllNodes()[i].connected[j]->position + Vector3(0, 7, 0), Vector4(1, 0, 0, 0.2f));
			}
		}
	}
}

GameWorld* world;
OGLRenderer* renderer;
GameStateHandler* stateHandler;

int main() {
	Vector2 screenDimensions = Vector2(1280, 720);
	Window*w = Window::CreateGameWindow("CSC8503 Game technology!", screenDimensions.x, screenDimensions.y);
	float countdownInMS = 180000;
	if (!w->HasInitialised()) {
		return -1;
	}	
	w->ShowOSPointer(false);
	w->LockMouseToWindow(true);
	
	world = new GameWorld();
	renderer = new GameTechRenderer(*world);
	Debug::SetRenderer(renderer);
	stateHandler = new GameStateHandler(renderer, 2);
	stateHandler->Update(0);
	stateHandler->Update(0);
	
	//GooseGameNetworking(g);

	//while (w->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KeyboardKeys::ESCAPE)) {
	while (w->UpdateWindow() && !stateHandler->GetStateStack().empty()) {
		
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

		stateHandler->Update(dt);

		//if (!(stateHandler->currentState == 1 || stateHandler->currentState == 2))
		//{
			Debug::FlushRenderables();
			renderer->Update(dt);
			renderer->Render();
		//}
		//DisplayGrid();

		//w->SetTitle("Gametech frame time:" + std::to_string(1000.0f * dt));

		std::ostringstream out;
		out.precision(0);
		out << std::fixed << ((--countdownInMS));		
		w->SetTitle("Gametech frame time:" + out.str());
		
	}
	
	delete world;
	delete renderer;
	delete stateHandler;
	Window::DestroyGameWindow();
}
