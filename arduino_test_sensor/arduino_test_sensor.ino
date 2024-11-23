#include <Wire.h>
#include <Adafruit_AMG88xx.h>
#include <math.h>

Adafruit_AMG88xx amg;

float Tamb1 = 0; // Ambient temperature (in Celsius)
float Tamb2 = 0;
float TH1_1 = 0;
float TH1_2 = 0;
float TH2 = 30.52;
int binary_TH = 17;

int cal_count = 1;

const float h = 15.0;  // Convective heat transfer coefficient 
const float sigma = 5.67e-8;  // Stefan-Boltzmann constant (W/m^2·K^4)
const float epsilon = 0.98;  // Emissivity (adjustable)

int blob_count_top = 0;
int blob_count_bottom = 0;

float means1[10];
float means2[10];

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

  float topPixels[32];
  float bottomPixels[32];

  memcpy(topPixels, pixels, sizeof(float) * 32);
  memcpy(bottomPixels, &pixels[32], sizeof(float) * 32);

  // // Print out the temperature data in an 8x8 grid format
  // for (int i = 0; i < 64; i++) {
  //   Serial.print(pixels[i]);
  //   Serial.print("\t");
  //   if ((i + 1) % 8 == 0) {
  //     Serial.println();  // New line after every 8 values
  //   }
  // }

  // 1.) calculate mean of array, set it to Tamb (ambient temperature)
  if (cal_count == 1) {
    Serial.print("Calibration step: ");
    Serial.println(cal_count);
    // Tamb = calculateMean(pixels, 64);
    // Serial.print("Tamb = ");
    // Serial.println(Tamb);
    Tamb1 = calculateMean(topPixels, 32);
    Serial.print("Tamb1 = ");
    Serial.println(Tamb1);
    Tamb2 = calculateMean(bottomPixels, 32);
    Serial.print("Tamb2 = ");
    Serial.println(Tamb2);
  }

  // 2-11.) change matrices to real temperatures based on formula
  if (cal_count > 1 && cal_count < 12) {
    Serial.print("Calibration step: ");
    Serial.println(cal_count);
    for (int i = 0; i < 32; i++) {
      // pixels[i] = applyEquation(pixels[i]);
      topPixels[i] = applyEquation(topPixels[i], Tamb1);
      bottomPixels[i] = applyEquation(bottomPixels[i], Tamb2);
    }
    // 12.) calculate TH1 by getting the means of the 2-11 matrices and finding the mean and standard devs of the means, and TH1 is 2.5 standard devs above mean
    // means[cal_count-2] = calculateMean(pixels, 64);
    // Serial.print("Mean of current matrix: ");
    // Serial.println(means[cal_count-2]);
    means1[cal_count-2] = calculateMean(topPixels, 32);
    Serial.print("Mean of current matrix 1: ");
    Serial.println(means1[cal_count-2]);
    means2[cal_count-2] = calculateMean(bottomPixels, 32);
    Serial.print("Mean of current matrix 2: ");
    Serial.println(means2[cal_count-2]);
    if (cal_count == 11) {
      // float m = calculateMean(means, 10);
      // TH1 = m + 3 * calculateStandardDeviation(means, 10, m);
      // Serial.print("Threshold 1 = ");
      // Serial.println(TH1);
      float m1 = calculateMean(means1, 10);
      TH1_1 = m1 + 3 * calculateStandardDeviation(means1, 10, m1);
      Serial.print("Threshold 1 for top half = ");
      Serial.println(TH1_1);
      float m2 = calculateMean(means2, 10);
      TH1_2 = m2 + 3 * calculateStandardDeviation(means2, 10, m2);
      Serial.print("Threshold 1 for bottom half = ");
      Serial.println(TH1_2);
    }
  }

  if (cal_count < 12) { cal_count++; }
  else {
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
    int TH2_count_1 = 0;
    for (int i = 0; i < 32; i++) {
      topPixels[i] = applyEquation(topPixels[i], Tamb1);
      if (topPixels[i] > TH2) {
        TH2_count_1++;
      }
    }
    float avgValue1 = calculateMean(topPixels, 32);
    if (avgValue1 < TH1_1) {
      blob_count_top = 0;
    }
    else if (TH2_count_1 > binary_TH) {
      blob_count_top = 2;
    }
    else {
      blob_count_top = 1;
    }
    Serial.print("Blob count for top half: ");
    Serial.println(blob_count_top);
    
    int TH2_count_2 = 0;
    for (int i = 0; i < 32; i++) {
      bottomPixels[i] = applyEquation(bottomPixels[i], Tamb2);
      if (bottomPixels[i] > TH2) {
        TH2_count_2++;
      }
    }
    float avgValue2 = calculateMean(bottomPixels, 32);
    if (avgValue2 < TH1_2) {
      blob_count_bottom = 0;
    }
    else if (TH2_count_2 > binary_TH) {
      blob_count_bottom = 2;
    }
    else {
      blob_count_bottom = 1;
    }
    Serial.print("Blob count for bottom half: ");
    Serial.println(blob_count_bottom);
  }

  Serial.println();
  delay(1000);

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

float applyEquation(float Tsa, float Tamb) {
  // Apply the equation to convert the temperature
  float Tv = pow(pow(Tsa + 273, 4) + (h * (Tsa - Tamb)) / (sigma * epsilon), 0.25) - 273;
  return Tv;
}
