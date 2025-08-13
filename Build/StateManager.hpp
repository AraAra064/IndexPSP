#include <deque>

#include "ConsoleGraphics.hpp"

#ifndef STATE_MANAGER
#define STATE_MANAGER

//Base class for states to be derived from
class StateBase
{
	protected:
		cg::ConsoleGraphics *graphics;
		void *globalStateData;
		uint16 *currentState; //When the current state var != prevState, it'll change to that state
		uint8 *stateManagerState;
		
		uint8 subState; //0 = Startup, 1 = Running, 2 = Shutdown
	public:
		virtual ~StateBase()
		{
			graphics = nullptr;
			globalStateData = nullptr;
			currentState = nullptr;
			stateManagerState = nullptr;
		}
		
		void setupState(cg::ConsoleGraphics *graphics, void* globalStateData, uint16 *currentState, uint8 *stateManagerState)
		{
			this->graphics = graphics;
			this->globalStateData = globalStateData;
			this->currentState = currentState;
			this->stateManagerState = stateManagerState;
			return;
		}
		uint8 getSubState(void){return subState;
		}
		
		virtual void onStartup(void){return;
		}
		virtual void update(float deltaTime){return;
		}
		virtual void render(void){return;
		}
		virtual void onExit(void){return;
		}
};

//Name does what it says
class StateManager
{
	cg::ConsoleGraphics *graphics;
	std::deque<StateBase*> states;
	uint16 currentState, prevState;
	uint8 state;
	
	void *globalStateData;
	protected:
	public:
		StateManager()
		{
			graphics = nullptr;
			currentState = 0;
			prevState = -1;
			state = 0;
			globalStateData = nullptr;
		}
		StateManager(cg::ConsoleGraphics *graphics, void *globalStateData = nullptr, uint16 startingState = 0)
		{
			this->graphics = graphics;
			this->globalStateData = globalStateData;
			currentState = startingState;
			prevState = -1;
			state = 0;
		}
		~StateManager()
		{
			for (StateBase *state : states)
			{
				if (state != nullptr)
				{
					delete state; //<---Fix this later
					state = nullptr;
				}
			}
			states.clear();
		}
		
		void addState(StateBase *state)
		{
			state->setupState(graphics, globalStateData, &currentState, &this->state);
			states.push_back(state);
			return;
		}
		
		uint8 getState(void){return state;
		}
		StateBase *getStatePtr(uint32 i){return states[i];
		}
		uint32 getNStates(void){return states.size();
		}
		
		void update(float deltaTime)
		{
			if (currentState != prevState)
			{
				if (prevState != (uint16)-1){states[prevState]->onExit();
				}
				states[currentState]->onStartup();
			}
			prevState = currentState;
			states[currentState]->update(deltaTime);
			return;
		}
		void render(void)
		{
			if (currentState == prevState){states[currentState]->render();
			}
			graphics->display();
			return;
		}
};

#endif
