#ifndef TOF_STATEMACHINE_H
#define TOF_STATEMACHINE_H

#include <Arduino.h>
#include <vector>

enum class TOFState {
    RESET,
    ENTER1,
    ENTER2,
    ENTER3,
    ENTEREND,
    EXIT1,
    EXIT2,
    EXIT3,
    EXITEND,
    EDGE_CASE,
    INVALID
};

class TOFStateMachine {
private:

    TOFState currentState = TOFState::RESET;


public:
    int numPeople = 0;

    // Constructor
    TOFStateMachine();

    // Update state based on TOF1 and 2 flags
    void updateState(bool TOF1_flag, bool TOF2_flag);

    // Print the status of all member variables
    void printState();
};

#endif // TOF_STATEMACHINE_H
