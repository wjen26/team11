#ifndef THERMALDOORSENSOR_H
#define THERMALDOORSENSOR_H

#include <Adafruit_AMG88xx.h>

class ThermalDoorSensor {
private:
    Adafruit_AMG88xx sensor;
    float Tamb1, Tamb2;
    float TH1_1, TH1_2, TH2;
    int binary_TH, cal_count;
    float means1[20], means2[20];
    const float h, sigma, epsilon;

    int blob_count_top, blob_count_bottom;

    float calculateMean(float arr[], int size);
    float calculateStandardDeviation(float arr[], int size, float mean);
    float applyEquation(float Tsa, float Tamb);
    float findMaxAndIndex(float arr[], int size, int &maxIndex);
    int detectBlobs(float pixels[], float threshold);

public:
    ThermalDoorSensor();

    bool begin();
    void calibrate(float pixels[]);
    void detect(float pixels[]);
    bool isCalibrating() const;
};

#endif
