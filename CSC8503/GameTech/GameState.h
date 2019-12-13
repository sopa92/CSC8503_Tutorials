#pragma once
#include <string>


namespace NCL
{
	namespace CSC8503
	{
		class TutorialGame;

		class GameState
		{
		public:
			GameState(int id = 0, const std::string& name = "State") {
				s_id = id;
				s_Name = name;
			}
			virtual ~GameState() = default;
		public:
			virtual void OnAwake() {};
			virtual void OnSleep() {};

			virtual int Update(float dt);
			void SetStateId(int newId) { s_id = newId; }
			int GetStateId() const { return s_id; }
			std::string& GetName() { return s_Name; }
		protected:
			std::string s_Name;
			int s_id;
		};


		class MenuState : public GameState
		{
		public:
			MenuState(int id = 0, int maxChoices = 3) {
				GameState(id, "Main Menu");
				chosenMenuOption = 0;
				maxOptions = maxChoices;
			}
			virtual ~MenuState() = default;

			virtual int Update(float dt);

		private:
			void DisplayMenu();
		private:
			int chosenMenuOption;
			int maxOptions;
		};

		class StartGameState : public GameState
		{
		public:
			StartGameState(int id = 1, int maxChoices = 2) {
				GameState(id, "Start Game");
				chosenMenuOption = 0;
				maxOptions = maxChoices;
				gooseGame = nullptr;
			}
			virtual ~StartGameState() = default;
			virtual int Update(float dt);
			virtual void OnAwake();
			virtual void OnSleep();
		private:
			void DisplayMenu();
		private:
			int chosenMenuOption;
			int maxOptions;
			TutorialGame* gooseGame;
		};

	}
}

