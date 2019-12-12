#include "GameStateHandler.h"
#include "../../Plugins/OpenGLRendering/OGLRenderer.h"

#include <iostream>

using namespace NCL::CSC8503;
using namespace NCL::Rendering;

GameStateHandler::GameStateHandler(OGLRenderer* rend, int statesNumber) : stateAmount(statesNumber), nextWantedState(0), renderer(rend) {
	InitStateMachine();
	InitStateMachineStorage();
}

GameStateHandler::~GameStateHandler() {
	delete gameStateMachine;

	for (int i = 0; i < stateStorage.size(); ++i)
		delete stateStorage[i];
	
	stateStorage.clear();
}

void GameStateHandler::Update(float dt) {

	if (activeState)
		nextWantedState = activeState->Update(dt);

	gameStateMachine->Update();
	gameStateMachine->Update();
}

void GameStateHandler::PushSelectedToMenuStack() {
	if (!(nextWantedState < stateStorage.size())) {
		nextWantedState = -1;	// go back to idle
		return;
	}
	stateStack.push(stateStorage[nextWantedState]);
	nextWantedState = -1;	// go back to idle

	if (activeState)
		activeState->OnSleep();
	activeState = stateStack.top();
	activeState->OnAwake();
	currentState = activeState->GetStateId();
}

void GameStateHandler::PopMenuStack() {
	if (!stateStack.empty())
		stateStack.pop();

	nextWantedState = -1;

	if (activeState)
		activeState->OnSleep();

	if (currentState == 1) {
		renderer->SetAsActiveRenderingContext();	//using the wglMakeCurrent
		Debug::SetRenderer(renderer);
	}

	if (!stateStack.empty()) {

		activeState = stateStack.top();
		activeState->OnAwake();

		currentState = activeState->GetStateId();
	}
	else {
		activeState = nullptr;
	}
}

void GameStateHandler::InitStateMachine() {
	gameStateMachine = new StateMachine();

	void* data = this;

	StateFunc IdleStateFunction = [](void* data) {	// when next state is -1
		//std::cout << "Current State : Idle" << std::endl;
	};

	StateFunc PushStateFunction = [](void* data) {	// when next state is >= 0 
		GameStateHandler* stateHandler = ((GameStateHandler*)data);
		std::cout << "Pushing state: " << stateHandler->GetNextChosenState() << " onto the state stack!";
		stateHandler->PushSelectedToMenuStack();
	};

	StateFunc PopStateFunction = [](void* data) { // next state selected is -2
		GameStateHandler* stateHandler = ((GameStateHandler*)data);
		stateHandler->PopMenuStack();
	};

	GenericState* stateIdle = new GenericState(IdleStateFunction, (void*)this);
	GenericState* statePush = new GenericState(PushStateFunction, (void*)this);
	GenericState* statePop = new GenericState(PopStateFunction, (void*)this);

	gameStateMachine->AddState(stateIdle);
	gameStateMachine->AddState(statePush);
	gameStateMachine->AddState(statePop);

	GenericTransition<int&, int>* transitionIdlePush = new GenericTransition<int&, int>(GenericTransition<int&, int>::GreaterThanTransition, 
		((GameStateHandler*)data)->GetNextChosenState(), -1, stateIdle, statePush);
	GenericTransition<int&, int>* transitionPushIdle = new GenericTransition<int&, int>(GenericTransition<int&, int>::LessThanTransition,
		((GameStateHandler*)data)->GetNextChosenState(), 0, statePush, stateIdle);
	GenericTransition<int&, int>* transitionIdlePop = new GenericTransition<int&, int>(GenericTransition<int&, int>::EqualsTransition,
		((GameStateHandler*)data)->GetNextChosenState(), -2, stateIdle, statePop);
	GenericTransition<int&, int>* transitionPopIdle = new GenericTransition<int&, int>(GenericTransition<int&, int>::GreaterThanTransition,
		((GameStateHandler*)data)->GetNextChosenState(), -2, statePop, stateIdle);

	gameStateMachine->AddTransition(transitionIdlePush);
	gameStateMachine->AddTransition(transitionPushIdle);
	gameStateMachine->AddTransition(transitionIdlePop);
	gameStateMachine->AddTransition(transitionPopIdle);
}

void GameStateHandler::InitStateMachineStorage() {
	stateStorage.reserve(stateAmount);
	MenuState* mainMenuState = new MenuState(0);
	stateStorage.push_back(mainMenuState);
	StartGameState* startGameState = new StartGameState(1);
	stateStorage.push_back(startGameState);
}
