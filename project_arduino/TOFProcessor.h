#ifndef TOF_PROCESSOR_H
#define TOF_PROCESSOR_H

#include <Arduino.h>
#include <vector>

class TOFProcessor {
private:
    std::vector<float> TOF1_arr;
    std::vector<float> TOF2_arr;

    unsigned long TOF1_timerStart = 0;
    unsigned long TOF2_timerStart = 0;
    const unsigned long flagClearTimeout = 5000; // 5 seconds

    bool TOF1_flag = false;
    bool TOF2_flag = false;
    bool TOF1_timerRunning = false;
    bool TOF2_timerRunning = false;

    // Check if all values in a vector are NaN
    bool allNaN(const std::vector<float>& vec);

    // Clear flags and arrays
    void clearWithFlag();

public:
    int numPeople = 0;

    // Constructor
    TOFProcessor();

    // Process incoming TOF data
    void process(float TOF1, float TOF2);
};

#endif // TOF_PROCESSOR_H
