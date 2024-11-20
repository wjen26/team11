#include "TOFProcessor.h"
#include <math.h>

// Constructor
TOFProcessor::TOFProcessor() {}

// Check if all values in a vector are NaN
bool TOFProcessor::allNaN(const std::vector<float>& vec) {
    for (float val : vec) {
        if (!isnan(val)) return false;
    }
    return true;
}

// Clear flags and arrays
void TOFProcessor::clearWithFlag() {
    TOF1_arr.clear();
    TOF2_arr.clear();
    TOF1_flag = false;
    TOF2_flag = false;
    TOF1_timerRunning = false;
    TOF2_timerRunning = false;
    Serial.println("Cleared TOF flags");
}

// Process incoming TOF data
void TOFProcessor::process(float TOF1, float TOF2) {
    // Handle TOF1
    if (TOF1 == 8191 || TOF1 == -1) {
        TOF1_arr.push_back(NAN);
    } else {
        TOF1_arr.push_back(TOF1);
    }
    TOF1_flag = !allNaN(TOF1_arr);

    // Handle TOF2
    if (TOF2 == 8191 || TOF2 == -1) {
        TOF2_arr.push_back(NAN);
    } else {
        TOF2_arr.push_back(TOF2);
    }
    TOF2_flag = !allNaN(TOF2_arr);

    // Check timers
    if (TOF1_flag && isnan(TOF1_arr.back()) && !TOF1_timerRunning) {
        TOF1_timerStart = millis();
        TOF1_timerRunning = true;
    }
    if (TOF2_flag && isnan(TOF2_arr.back()) && !TOF2_timerRunning) {
        TOF2_timerStart = millis();
        TOF2_timerRunning = true;
    }

    // Check timer expiration
    if (TOF1_timerRunning && millis() - TOF1_timerStart > flagClearTimeout) {
        clearWithFlag();
    }
    if (TOF2_timerRunning && millis() - TOF2_timerStart > flagClearTimeout) {
        clearWithFlag();
    }

    // Process person detection
    if (TOF1_flag && TOF2_flag) {
        int minTOF1_index = -1, minTOF2_index = -1;
        float minTOF1 = 1e9, minTOF2 = 1e9;

        // Find indices of minimum valid values
        for (size_t i = 0; i < TOF1_arr.size(); i++) {
            if (!isnan(TOF1_arr[i]) && TOF1_arr[i] < minTOF1) {
                minTOF1 = TOF1_arr[i];
                minTOF1_index = i;
            }
        }
        for (size_t i = 0; i < TOF2_arr.size(); i++) {
            if (!isnan(TOF2_arr[i]) && TOF2_arr[i] < minTOF2) {
                minTOF2 = TOF2_arr[i];
                minTOF2_index = i;
            }
        }

        // Determine entry/exit
        if (minTOF1_index < minTOF2_index) {
            Serial.println("Person entering");
            numPeople++;
        } else {
            Serial.println("Person exiting");
            numPeople--;
        }

        clearWithFlag();
    }
}
