#ifndef __STATEMACHINE_HPP
#define __STATEMACHINE_HPP

#include "pico/stdlib.h"

class State 
{
    public:
        State();
        ~State();

        virtual void onEnter(State *lastState);
        virtual void onState() = 0;
        virtual void onExit(State *nextState);

        void addTransition(bool (*c)(), State *state);
        void addTransition(bool (*c)(), int stateNumber);

        int evalTransitions();
};

class StateMachine
{
    private:
        State *_currentState = NULL;

    public:
        StateMachine();
        ~StateMachine();
        void init();
        void run();

        State* addState(void(*functionPointer)());
        State* transitionTo(State *state);
};

#endif