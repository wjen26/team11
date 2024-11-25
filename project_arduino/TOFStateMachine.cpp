#include "TOFStateMachine.h"
#include <math.h>

// Constructor
TOFStateMachine::TOFStateMachine() {}

void TOFStateMachine::updateState(bool TOF1_flag, bool TOF2_flag) {
    switch (currentState) {
        case TOFState::RESET:
            if (TOF1_flag == true && TOF2_flag == false)
                currentState = TOFState::ENTER1;
            else if (TOF1_flag == false && TOF2_flag == true)
                currentState = TOFState::EXIT1;
            else if (TOF1_flag == true && TOF2_flag == true)
                currentState = TOFState::INVALID; //maybe two in opposite?
            else if (TOF1_flag == false && TOF2_flag == false)
                currentState = currentState;
            break;
        case TOFState::ENTER1:
            if (TOF1_flag == true && TOF2_flag == false)
                currentState = currentState;
            else if (TOF1_flag == false && TOF2_flag == true)
                currentState = TOFState::INVALID; //maybe just go to enter 3??
            else if (TOF1_flag == true && TOF2_flag == true)
                currentState = TOFState::ENTER2; //check for how many people there, if two go to conga state
            else if (TOF1_flag == false && TOF2_flag == false)
                currentState = TOFState::RESET;
            break;
        case TOFState::ENTER2:
            if (TOF1_flag == true && TOF2_flag == false)
                currentState = TOFState::ENTER1;
            else if (TOF1_flag == false && TOF2_flag == true)
                currentState = TOFState::ENTER3;
            else if (TOF1_flag == true && TOF2_flag == true)
                currentState = currentState;
            else if (TOF1_flag == false && TOF2_flag == false)
                currentState = TOFState::INVALID; //maybe just increment?? actually no could have gone either way
            break;
        case TOFState::ENTER3:
            if (TOF1_flag == true && TOF2_flag == false)
                currentState = TOFState::INVALID; //maybe just back to enter 1
            else if (TOF1_flag == false && TOF2_flag == true)
                currentState = currentState;
            else if (TOF1_flag == true && TOF2_flag == true)
                currentState = TOFState::ENTER2;
            else if (TOF1_flag == false && TOF2_flag == false)
                currentState = TOFState::ENTEREND;
            break;
        case TOFState::ENTEREND:
            numPeople++;
            currentState = TOFState::RESET;
            break;
        case TOFState::EXIT1:
            if (TOF1_flag == true && TOF2_flag == false)
                currentState = TOFState::INVALID; //maybe just go to exit3
            else if (TOF1_flag == false && TOF2_flag == true)
                currentState = currentState;
            else if (TOF1_flag == true && TOF2_flag == true)
                currentState = TOFState::EXIT2; //go to conga (exit) probably need two conga states thinking about it
            else if (TOF1_flag == false && TOF2_flag == false)
                currentState = TOFState::RESET;
            break;
        case TOFState::EXIT2:
            if (TOF1_flag == true && TOF2_flag == false)
                currentState = TOFState::EXIT3;
            else if (TOF1_flag == false && TOF2_flag == true)
                currentState = TOFState::EXIT1;
            else if (TOF1_flag == true && TOF2_flag == true)
                currentState = currentState;
            else if (TOF1_flag == false && TOF2_flag == false)
                currentState = TOFState::INVALID; //same problem as enter2
            break;
        case TOFState::EXIT3:
            if (TOF1_flag == true && TOF2_flag == false)
                currentState = currentState;
            else if (TOF1_flag == false && TOF2_flag == true)
                currentState = TOFState::INVALID; //maybe just go back to exit 1
            else if (TOF1_flag == true && TOF2_flag == true)
                currentState = TOFState::EXIT2;
            else if (TOF1_flag == false && TOF2_flag == false)
                currentState = TOFState::EXITEND;
            break;
        case TOFState::EXITEND:
            numPeople--;
            currentState = TOFState::RESET;
            break;
        case TOFState::INVALID:
            //need to figure out what to do with this
            currentState = TOFState::RESET;
            break;
    }
}

// Print the current state
void TOFStateMachine::printState() {
    //print current state
    switch (currentState) {
        case TOFState::RESET:
            Serial.print("RESET State    ");
            break;
        case TOFState::ENTER1:
            Serial.print("ENTER1 State    ");
            break;
        case TOFState::ENTER2:
            Serial.print("ENTER2 State    ");
            break;
        case TOFState::ENTER3:
            Serial.print("ENTER3 State    ");
            break;
        case TOFState::ENTEREND:
            Serial.print("ENTEREND State    ");
            break;
        case TOFState::EXIT1:
            Serial.print("EXIT1 State    ");
            break;
        case TOFState::EXIT2:
            Serial.print("EXIT2 State    ");
            break;
        case TOFState::EXIT3:
            Serial.print("EXIT3 State    ");
            break;
        case TOFState::EXITEND:
            Serial.print("EXITEND State    ");
            break;
        case TOFState::INVALID:
            Serial.print("INVALID State    ");
            break;
    }

    

    
}
