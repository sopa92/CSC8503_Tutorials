/***********************************************
Course: CSC8503
Project: Coursework part A + part B
Author: Sofia Papadopoulou - 190338075
Acad. Year:	2019-2020
Git Repo: https://github.com/sopa92/CSC8503_Tutorials.git
References: Sound for honk 
http://soundbible.com/1187-Goose.html
***********************************************/


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


GameWorld* world;
OGLRenderer* renderer;
GameStateHandler* stateHandler;

void GooseGameNetworking();
void DisplayGrid();
void ReturnToMenuAndRestart();

int main() {
	Vector2 screenDimensions = Vector2(1280, 720);
	Window*w = Window::CreateGameWindow("CSC8503 Game technology!", screenDimensions.x, screenDimensions.y, true);
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

	float countdownInSec = 180;

	//while (w->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KeyboardKeys::ESCAPE)) {
	while (w->UpdateWindow() && !stateHandler->GetStateStack().empty()) {
		
		float dt = w->GetTimer()->GetTimeDeltaSeconds();
		stateHandler->Update(dt);
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

		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::N)) {
			GooseGameNetworking();
		}
		

		if (!(stateHandler->currentState == 1))
		{
			Debug::FlushRenderables();
			renderer->Update(dt);
			renderer->Render();
		}
		//DisplayGrid();
		
		float gameDuration = w->GetTimer()->GetTotalTimeSeconds();
		float timeLeft = countdownInSec - gameDuration;
		if (timeLeft > 0) {
			std::ostringstream out;
			out.precision(0);
			out << std::fixed << (timeLeft);
			w->SetTitle("THE GOOSE GAME - Time Left (seconds): " + out.str());
			if (timeLeft < 20)
				Debug::Print("Time Left : " + out.str(), Vector2(screenDimensions.x / 2, screenDimensions.y - 100), Vector4(1, 0, 0, 1));
			else
				Debug::Print("Time Left : " + out.str(), Vector2(screenDimensions.x / 2, screenDimensions.y - 100), Vector4(0, 1, 0, 1));
		}
		else {

			Debug::Print("       Time's up...", Vector2(screenDimensions.x / 4, screenDimensions.y / 2), Vector4(1,0,0,1));

			Debug::Print("Press SPACE to continue", Vector2((screenDimensions.x / 4), (screenDimensions.y / 2 )-20), Vector4(1, 0, 0, 1));

			if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::SPACE)) {

				Debug::FlushRenderables();
				ReturnToMenuAndRestart();
				countdownInSec = 180 + countdownInSec;
			}
		}
	}
	
	delete world;
	delete renderer;
	delete stateHandler;
	Window::DestroyGameWindow();
}

void ReturnToMenuAndRestart() {
	delete world;
	delete renderer;
	delete stateHandler;
	world = new GameWorld();
	renderer = new GameTechRenderer(*world);
	Debug::SetRenderer(renderer);
	stateHandler = new GameStateHandler(renderer, 2);
	stateHandler->Update(0);
	stateHandler->Update(0);
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

class GooseGamePacketReceiver : public PacketReceiver {
public:
	GooseGamePacketReceiver(string name) {
		this->name = name;
	}
	void ReceivePacket(int type, GamePacket* payload, int source) {
		if (type == String_Message) {
			StringPacket* realPacket = (StringPacket*)payload;

			string msg = realPacket->GetStringFromData();
			if (msg != "") {
				if (msg == "POWER")
					Debug::Print("Power Up Activated!", Vector2(50, 400), Vector4(1,0,0,1));
				std::cout << name << " received message: " << msg << std::endl;
			}

			//clean the file with cheat
			std::ofstream outfile(Assets::DATADIR + "CheatCommand.txt");
			outfile.close();
		}
	}
protected:
	string name;
};

void GooseGameNetworking() {
	NetworkBase::Initialise();

	GooseGamePacketReceiver serverReceiver("Server");
	GooseGamePacketReceiver clientReceiver("Client");

	int port = NetworkBase::GetDefaultPort();

	GameServer* server = new GameServer(port, 1);
	GameClient* client = new GameClient();

	server->RegisterPacketHandler(String_Message, &serverReceiver);
	client->RegisterPacketHandler(String_Message, &clientReceiver);

	bool canConnect = client->Connect(127, 0, 0, 1, port);

	

	for (int i = 0; i < 10; ++i) {
		std::ifstream infile(Assets::DATADIR + "CheatCommand.txt");
		string commandFromFile;
		infile >> commandFromFile;
		infile.close();

		server->SendGlobalPacket(StringPacket(commandFromFile));

		server->UpdateServer();
		client->UpdateClient();

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	NetworkBase::Destroy();
}