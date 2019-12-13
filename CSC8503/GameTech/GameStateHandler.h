#pragma once

#include <stack>
#include <vector>
#include <iostream>
#include "../../Plugins/OpenGLRendering/OGLRenderer.h"
#include "../CSC8503Common/StateMachine.h"
#include "../CSC8503Common/State.h"
#include "../CSC8503Common/StateTransition.h"
#include "GameState.h"
#include "../CSC8503Common/Debug.h"

namespace NCL
{
	namespace CSC8503
	{
		class GameState;
		class StateMachine;

		class GameStateHandler
		{
		public:
			GameStateHandler(OGLRenderer* rend, int statesNumber);
			~GameStateHandler();
			void Update(float dt);
			int& GetNextChosenState() { return nextWantedState; }
			void SetNextStateSelected(int value) { nextWantedState = value; }
			std::stack<GameState*>& GetStateStack() { return stateStack; }
			std::vector<GameState*>& GetStateStorage() { return stateStorage; }
			void PushSelectedToMenuStack();
			void PopMenuStack();
		public:
			int currentState;
		private:
			void InitStateMachine();
			void InitStateMachineStorage();
		private:
			int stateAmount;
			int nextWantedState;

			GameState* activeState = nullptr;

			std::stack<GameState*> stateStack;
			std::vector<GameState*> stateStorage;

			StateMachine* gameStateMachine;
			OGLRenderer* renderer;
		};

	}
}

