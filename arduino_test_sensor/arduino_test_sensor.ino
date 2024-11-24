#include <Wire.h>
#include <Adafruit_AMG88xx.h>
#include <math.h>

Adafruit_AMG88xx amg;

float Tamb= 0;
float TH1_1 = 0;
float TH1_2 = 0;
float TH2 = 30.52;
int binary_TH = 14;

int cal_count = 1;

const float h = 15.0;  // Convective heat transfer coefficient 
const float sigma = 5.67e-8;  // Stefan-Boltzmann constant (W/m^2·K^4)
const float epsilon = 0.98;  // Emissivity (adjustable)

int blob_count_top = 0;
int blob_count_bottom = 0;

float max_values1[10];
float max_values2[10];

void setup() {
  Serial.begin(9600);
  while (! Serial) {
    delay(1000);
  }
  if (!amg.begin()) {
    Serial.println("Could not find a valid AMG88xx sensor, check wiring!");
    while (1);
  }
  Serial.println("AMG88xx sensor found!");
}

void loop() {
  float pixels[64];  // Array to hold the 8x8 thermal data

  // Read pixels from sensor
  amg.readPixels(pixels);
  

  // // Print out the temperature data in an 8x8 grid format
  // for (int i = 0; i < 64; i++) {
  //   Serial.print(pixels[i]);
  //   Serial.print("\t");
  //   if ((i + 1) % 8 == 0) {
  //     Serial.println();  // New line after every 8 values
  //   }
  // }

  float topPixels[32];
  float bottomPixels[32];

  

  // 1.) calculate mean of array, set it to Tamb (ambient temperature)
  if (cal_count == 1) {
    Serial.print("Calibration step: ");
    Serial.println(cal_count);
    Tamb = calculateMean(pixels, 64);
    Serial.print("Tamb = ");
    Serial.println(Tamb);
    // Tamb1 = calculateMean(topPixels, 32);
    // Serial.print("Tamb1 = ");
    // Serial.println(Tamb1);
    // Tamb2 = calculateMean(bottomPixels, 32);
    // Serial.print("Tamb2 = ");
    // Serial.println(Tamb2);
  }

  // 2-11.) change matrices to real temperatures based on formula
  if (cal_count > 1 && cal_count < 12) {
    Serial.print("Calibration step: ");
    Serial.println(cal_count);
    for (int i = 0; i < 64; i++) {
      pixels[i] = applyEquation(pixels[i]);
      // topPixels[i] = applyEquation(topPixels[i], Tamb1);
      // bottomPixels[i] = applyEquation(bottomPixels[i], Tamb2);
    }
    for (int i = 0; i < 32; i++) {
      topPixels[i] = pixels[i];  // Top half (first 32 pixels)
      bottomPixels[i] = pixels[i + 32];  // Bottom half (next 32 pixels)
    }
    // 12.) calculate TH1 by getting the means of the 2-11 matrices and finding the mean and standard devs of the means, and TH1 is 2.5 standard devs above mean
    // max_values[cal_count-2] = calculateMax(pixels, 64);
    // Serial.print("Max value of current matrix: ");
    // Serial.println(max_values[cal_count-2]);
    max_values1[cal_count-2] = calculateMax(topPixels, 32);
    Serial.print("Max value of current top matrix: ");
    Serial.println(max_values1[cal_count-2]);
    max_values2[cal_count-2] = calculateMax(bottomPixels, 32);
    Serial.print("Max value of current bottom matrix: ");
    Serial.println(max_values2[cal_count-2]);
    if (cal_count == 11) {
      // float m = calculateMean(max_values, 10);
      // TH1 = m + 2.5 * calculateStandardDeviation(max_values, 10, m);
      // Serial.print("Threshold 1 = ");
      // Serial.println(TH1);
      float m1 = calculateMean(max_values1, 10);
      TH1_1 = m1 + 2.5 * calculateStandardDeviation(max_values1, 10, m1);
      Serial.print("Threshold 1 for top half = ");
      Serial.println(TH1_1);
      float m2 = calculateMean(max_values2, 10);
      TH1_2 = m2 + 2.5 * calculateStandardDeviation(max_values2, 10, m2);
      Serial.print("Threshold 1 for bottom half = ");
      Serial.println(TH1_2);
    }
  }

  // actual detection logic
  if (cal_count < 12) { cal_count++; }
  else {
    // memcpy(topPixels, pixels, sizeof(float) * 32);
    // memcpy(bottomPixels, &pixels[32], sizeof(float) * 32);
    for (int i = 0; i < 32; i++) {
      topPixels[i] = pixels[i];  // Top half (first 32 pixels)
      bottomPixels[i] = pixels[i + 32];  // Bottom half (next 32 pixels)
    }

    // int TH2_count = 0;
    // for (int i = 0; i < 64; i++) {
    //   pixels[i] = applyEquation(pixels[i]);
    //   if (pixels[i] > TH2) {
    //     TH2_count++;
    //   }
    // }
    // float avgValue = calculateMean(pixels, 64);
    // if (avgValue < TH1) {
    //   blob_count = 0;
    // }
    // else if (TH2_count > binary_TH) {
    //   blob_count = 2;
    // }
    // else {
    //   blob_count = 1;
    // }
    // Serial.print("Blob count: ");
    // Serial.println(blob_count);


    int TH1_count_top = 0;
    for (int i = 0; i < 32; i++) {
      topPixels[i] = applyEquation(topPixels[i]);
      if (topPixels[i] > TH1_1) {
        TH1_count_top++;
      }
    }
    // float avgValue1 = calculateMean(topPixels, 32);
    // if (avgValue1 < TH1_1) {
    //   blob_count_top = 0;
    // }
    // else if (TH2_count_top > binary_TH) {
    //   blob_count_top = 2;
    // }
    // else {
    //   blob_count_top = 1;
    if (TH1_count_top < 12) {
      blob_count_top = 0;
    }
    // else if (TH2_count_top > binary_TH) {
    //   blob_count_top = 2;
    // }
    else {
      blob_count_top = 1;

      int maxIndex;
      int maxValue = findMaxAndIndex(topPixels, 32, maxIndex);
      Serial.print("Top max value at [");
      Serial.print(maxIndex / 8);
      Serial.print("][");
      Serial.print(maxIndex % 8);
      Serial.print("] = ");
      Serial.println(maxValue);
    }
    Serial.print("Blob count for top half: ");
    Serial.println(blob_count_top);
    


    int TH1_count_bottom = 0;
    for (int i = 0; i < 32; i++) {
      bottomPixels[i] = applyEquation(bottomPixels[i]);
      if (bottomPixels[i] > TH1_2) {
        TH1_count_bottom++;
      }
    }
    // float avgValue2 = calculateMean(bottomPixels, 32);
    // if (avgValue2 < TH1_2) {
    //   blob_count_bottom = 0;
    // }
    // else if (TH2_count_bottom > binary_TH) {
    //   blob_count_bottom = 2;
    // }
    // else {
    //   blob_count_bottom = 1;
    if (TH1_count_bottom < 12) {
      blob_count_bottom = 0;
    }
    // else if (TH2_count_bottom > binary_TH) {
    //   blob_count_bottom = 2;
    // }
    else {
      blob_count_bottom = 1;

      int maxIndex2;
      int maxValue2 = findMaxAndIndex(bottomPixels, 32, maxIndex2);
      Serial.print("Bottom max value at [");
      Serial.print(maxIndex2 / 8);
      Serial.print("][");
      Serial.print(maxIndex2 % 8);
      Serial.print("] = ");
      Serial.println(maxValue2);
    }
    Serial.print("Blob count for bottom half: ");
    Serial.println(blob_count_bottom);
  }

  Serial.println();
  delay(100);

}

float calculateMean(float arr[], int size) {
  float sum = 0.0;
  for (int i = 0; i < size; i++) {
    sum += arr[i];
  }
  return sum / size;
}

float calculateStandardDeviation(float arr[], int size, float mean) {
  float sumOfSquares = 0.0;
  for (int i = 0; i < size; i++) {
    sumOfSquares += pow(arr[i] - mean, 2);  // Square of differences from the mean
  }
  return sqrt(sumOfSquares / size);  // Standard deviation formula
}

float calculateMax(float arr[], int size) {
  float maxValue = arr[0];
  for (int i = 1; i < size; i++) {
    if (arr[i] > maxValue) {
      maxValue = arr[i];
    }
  }
  return maxValue;
}

float applyEquation(float Tsa) {
  // Apply the equation to convert the temperature
  float Tv = pow(pow(Tsa + 273, 4) + (h * (Tsa - Tamb)) / (sigma * epsilon), 0.25) - 273;
  return Tv;
}

float findMaxAndIndex(float arr[], int size, int &maxIndex) {
  float maxValue = arr[0];  // Assume the first element is the maximum initially
  maxIndex = 0;  // Index of the first element

  for (int i = 1; i < size; i++) {
    if (arr[i] > maxValue) {
      maxValue = arr[i];
      maxIndex = i;  // Update the index of the maximum value
    }
  }

  return maxValue;
}
