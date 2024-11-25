#include <Wire.h>
#include <Adafruit_AMG88xx.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

Adafruit_AMG88xx amg;

// Constants
const float h = 15.0;  // Convective heat transfer coefficient
const float sigma = 5.67e-8;  // Stefan-Boltzmann constant
const float epsilon = 0.98;  // Emissivity
const int binary_TH = 17;

// Variables
float pixels[64];       // Array to hold thermal data
float topPixels[32];    // Top half of the grid
float bottomPixels[32]; // Bottom half of the grid
float Tamb1 = 0, Tamb2 = 0;
float TH1_1 = 0, TH1_2 = 0;
float TH2 = 30.52;
float means1[20], means2[20];
int cal_count = 0;
int blob_count_top = 0, blob_count_bottom = 0;

// Function prototypes
float calculateMean(const float arr[], int size);
float calculateStandardDeviation(const float arr[], int size, float mean);
float applyEquation(float Tsa, float Tamb);
float findMaxAndIndex(const float arr[], int size, int &maxIndex);
void processThermalData();

void setup() {
  Serial.begin(115200);
  Wire.setClock(400000);  // Set I2C to Fast Mode (400 kHz)

  if (!amg.begin()) {
    Serial.println("Could not find a valid AMG88xx sensor, check wiring!");
    while (1);
  }
  Serial.println("AMG88xx sensor initialized!");

  // Create a task for processing on core 0
  xTaskCreatePinnedToCore(processThermalData, "ThermalDataTask", 4096, NULL, 1, NULL, 0);
}

void loop() {
  // Main loop does nothing; all work is on core 0
  delay(1000);
}

void processThermalData() {
  while (true) {
    // Read pixels from sensor
    amg.readPixels(pixels);

    // Divide into top and bottom arrays
    for (int i = 0; i < 32; i++) {
      topPixels[i] = pixels[i];
      bottomPixels[i] = pixels[i + 32];
    }

    if (cal_count == 1) {
      // Calibration: Calculate ambient temperatures
      Tamb1 = calculateMean(topPixels, 32);
      Tamb2 = calculateMean(bottomPixels, 32);
      Serial.printf("Tamb1: %.2f, Tamb2: %.2f\n", Tamb1, Tamb2);
    } else if (cal_count > 1 && cal_count < 22) {
      // Calibration steps: Adjust matrices and calculate thresholds
      for (int i = 0; i < 32; i++) {
        topPixels[i] = applyEquation(topPixels[i], Tamb1);
        bottomPixels[i] = applyEquation(bottomPixels[i], Tamb2);
      }

      means1[cal_count - 2] = calculateMean(topPixels, 32);
      means2[cal_count - 2] = calculateMean(bottomPixels, 32);

      if (cal_count == 21) {
        // Final threshold calculations
        float mean1 = calculateMean(means1, 20);
        TH1_1 = mean1 + 3 * calculateStandardDeviation(means1, 20, mean1);

        float mean2 = calculateMean(means2, 20);
        TH1_2 = mean2 + 3 * calculateStandardDeviation(means2, 20, mean2);

        Serial.printf("TH1_1: %.2f, TH1_2: %.2f\n", TH1_1, TH1_2);
      }
    } else if (cal_count >= 22) {
      // Detection logic
      int TH2_count_top = 0, TH2_count_bottom = 0;

      for (int i = 0; i < 32; i++) {
        topPixels[i] = applyEquation(topPixels[i], Tamb1);
        bottomPixels[i] = applyEquation(bottomPixels[i], Tamb2);

        if (topPixels[i] > TH2) TH2_count_top++;
        if (bottomPixels[i] > TH2) TH2_count_bottom++;
      }

      float avgValue1 = calculateMean(topPixels, 32);
      float avgValue2 = calculateMean(bottomPixels, 32);

      blob_count_top = (avgValue1 < TH1_1) ? 0 : (TH2_count_top > binary_TH) ? 2 : 1;
      blob_count_bottom = (avgValue2 < TH1_2) ? 0 : (TH2_count_bottom > binary_TH) ? 2 : 1;

      int maxIndex1, maxIndex2;
      float maxValue1 = findMaxAndIndex(topPixels, 32, maxIndex1);
      float maxValue2 = findMaxAndIndex(bottomPixels, 32, maxIndex2);

      Serial.printf("Blob Count Top: %d, Max Value Top [%d][%d]: %.2f\n", blob_count_top, maxIndex1 / 8, maxIndex1 % 8, maxValue1);
      Serial.printf("Blob Count Bottom: %d, Max Value Bottom [%d][%d]: %.2f\n", blob_count_bottom, maxIndex2 / 8, maxIndex2 % 8, maxValue2);
    }

    cal_count = (cal_count < 22) ? cal_count + 1 : cal_count;
    vTaskDelay(pdMS_TO_TICKS(100)); // Match sensor refresh rate
  }
}

float calculateMean(const float arr[], int size) {
  float sum = 0.0f;
  for (int i = 0; i < size; i++) sum += arr[i];
  return sum / size;
}

float calculateStandardDeviation(const float arr[], int size, float mean) {
  float sumOfSquares = 0.0f;
  for (int i = 0; i < size; i++) sumOfSquares += (arr[i] - mean) * (arr[i] - mean);
  return sqrtf(sumOfSquares / size);
}

float applyEquation(float Tsa, float Tamb) {
  // Optimized equation
  return powf(powf(Tsa + 273.0f, 4) + (h * (Tsa - Tamb)) / (sigma * epsilon), 0.25f) - 273.0f;
}

float findMaxAndIndex(const float arr[], int size, int &maxIndex) {
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
