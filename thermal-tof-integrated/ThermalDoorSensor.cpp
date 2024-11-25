#include "ThermalDoorSensor.h"
#include <Arduino.h>
#include <math.h>

// Constructor to initialize constants
ThermalDoorSensor::ThermalDoorSensor()
    : Tamb1(0), Tamb2(0), TH1_1(0), TH1_2(0), TH2(30.52), binary_TH(17), 
      cal_count(0), h(15.0), sigma(5.67e-8), epsilon(0.98),
      blob_count_top(0), blob_count_bottom(0) {}

bool ThermalDoorSensor::begin() {
    return sensor.begin();
}

void ThermalDoorSensor::calibrate(float pixels[]) {
    float topPixels[32], bottomPixels[32];
    for (int i = 0; i < 32; i++) {
        topPixels[i] = pixels[i];
        bottomPixels[i] = pixels[i + 32];
    }

    if (cal_count == 1) {
        Tamb1 = calculateMean(topPixels, 32);
        Tamb2 = calculateMean(bottomPixels, 32);
    } else if (cal_count > 1 && cal_count < 22) {
        for (int i = 0; i < 32; i++) {
            topPixels[i] = applyEquation(topPixels[i], Tamb1);
            bottomPixels[i] = applyEquation(bottomPixels[i], Tamb2);
        }
        means1[cal_count - 2] = calculateMean(topPixels, 32);
        means2[cal_count - 2] = calculateMean(bottomPixels, 32);

        if (cal_count == 21) {
            float m1 = calculateMean(means1, 20);
            TH1_1 = m1 + 3 * calculateStandardDeviation(means1, 20, m1);
            float m2 = calculateMean(means2, 20);
            TH1_2 = m2 + 3 * calculateStandardDeviation(means2, 20, m2);
        }
    }
    cal_count++;
}

void ThermalDoorSensor::detect(float pixels[]) {
    if (cal_count < 22) return;  // Skip detection until calibration is done

    float topPixels[32], bottomPixels[32];
    for (int i = 0; i < 32; i++) {
        topPixels[i] = applyEquation(pixels[i], Tamb1);
        bottomPixels[i] = applyEquation(pixels[i + 32], Tamb2);
    }

    blob_count_top = detectBlobs(topPixels, TH1_1);
    blob_count_bottom = detectBlobs(bottomPixels, TH1_2);

    Serial.print("Blob count (top): ");
    Serial.println(blob_count_top);
    Serial.print("Blob count (bottom): ");
    Serial.println(blob_count_bottom);
}

bool ThermalDoorSensor::isCalibrating() const {
    return cal_count < 22;
}

float ThermalDoorSensor::calculateMean(float arr[], int size) {
    float sum = 0.0;
    for (int i = 0; i < size; i++) sum += arr[i];
    return sum / size;
}

float ThermalDoorSensor::calculateStandardDeviation(float arr[], int size, float mean) {
    float sumOfSquares = 0.0;
    for (int i = 0; i < size; i++)
        sumOfSquares += pow(arr[i] - mean, 2);
    return sqrt(sumOfSquares / size);
}

float ThermalDoorSensor::applyEquation(float Tsa, float Tamb) {
    return pow(pow(Tsa + 273, 4) + (h * (Tsa - Tamb)) / (sigma * epsilon), 0.25) - 273;
}

float ThermalDoorSensor::findMaxAndIndex(float arr[], int size, int &maxIndex) {
    float maxValue = arr[0];
    maxIndex = 0;
    for (int i = 1; i < size; i++) {
        if (arr[i] > maxValue) {
            maxValue = arr[i];
            maxIndex = i;
        }
    }
    return maxValue;
}

int ThermalDoorSensor::detectBlobs(float pixels[], float threshold) {
    int TH2_count = 0, maxIndex;
    for (int i = 0; i < 32; i++) {
        if (pixels[i] > TH2) TH2_count++;
    }

    float avgValue = calculateMean(pixels, 32);
    int blobCount = 0;
    if (avgValue < threshold) {
        blobCount = 0;
    } else if (TH2_count > binary_TH) {
        blobCount = 2;
    } else {
        blobCount = 1;
        float maxValue = findMaxAndIndex(pixels, 32, maxIndex);
        Serial.print("Max value at [");
        Serial.print(maxIndex / 8);
        Serial.print("][");
        Serial.print(maxIndex % 8);
        Serial.print("] = ");
        Serial.println(maxValue);
    }
    return blobCount;
}
