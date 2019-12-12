#include "GameState.h"
#include "TutorialGame.h"

namespace NCL
{
	namespace CSC8503
	{
		int GameState::Update(float dt)
		{
			//std::cout << "" << s_Name << "\n";
			return -1;
		}

		int MenuState::Update(float dt) {

			if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::UP))
			{
				if (chosenMenuOption == 0)		// if the first option is the selected one
					chosenMenuOption = maxOptions;	// go to the last
				else
					chosenMenuOption--;				// go to the one above
			}

			if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::DOWN))
			{
				if (chosenMenuOption == maxOptions)		// if the last option is the selected one
					chosenMenuOption = 0;			// go to the first
				else
					chosenMenuOption++;				// go to the one below
			}

			DisplayMenu();

			if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::RETURN))
			{
				if (chosenMenuOption == 2)
					return -2;
				return (chosenMenuOption + 1);
			}

			return -1;
		}
		void MenuState::DisplayMenu() {

			const Vector4 selectedColor(1, 0, 1, 1);
			const Vector4 defaultColor(0.3f, 0.3f, 0.3f, 1);

			const Vector2 pos1(150, 480);
			const Vector2 pos2(150, 540);

			if (chosenMenuOption == 0)
			{
				Debug::Print("Start Game", pos2, selectedColor);
				Debug::Print("Exit", pos1, defaultColor);
			}
			else if (chosenMenuOption == 1)
			{
				Debug::Print("Start Game", pos2, defaultColor);
				Debug::Print("Exit", pos1, selectedColor);
			}
		}


		int StartGameState::Update(float dt)
		{
			if (gooseGame) {
				gooseGame->UpdateGame(dt);
				DisplayMenu();
			}
			if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::ESCAPE)) {
				return -2;
			}
			return -1;
		}

		void StartGameState::OnAwake() {
			gooseGame = new TutorialGame();
		}

		void StartGameState::OnSleep() {
			delete gooseGame;
			gooseGame = nullptr;
		}

		void StartGameState::DisplayMenu() {
			const Vector4 selectedColor(1, 0, 1, 1);
			const Vector2 pos2(150, 800);
			Debug::Print("Start Game", pos2, selectedColor);
		}

	}
}