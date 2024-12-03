#include <Wire.h>
#include <Adafruit_AMG88xx.h>

Adafruit_AMG88xx amg;

float pixels[AMG88xx_PIXEL_ARRAY_SIZE];

int cal_count = 0;


const float h = 15.0;         // Convective heat transfer coefficient
const float sigma = 5.67e-8;  // Stefan-Boltzmann constant (W/m^2·K^4)
const float epsilon = 0.98;   // Emissivity (adjustable)
float Tamb;

float means[20];

float TH2;

int count = 0;

//is there a person in the first 2 rows
bool flag1 = false;
//one person = false, two people = true
bool flag2 = false;

void setup() {
  Serial.begin(9600);
  //Serial.println(F("AMG88xx pixels"));

  bool status;

  // default settings
  status = amg.begin();
  if (!status) {
    Serial.println("Could not find a valid AMG88xx sensor, check wiring!");
    while (1)
      ;
  }

  //Serial.println("-- Pixels Test --");

  //Serial.println();

  delay(100);  // let sensor boot up
}


void loop() {
  //read all the pixels
  amg.readPixels(pixels);

  //setting Tamb
  if (cal_count == 1) {
    delay(5000);
    Serial.println(cal_count);
    Tamb = calculateMean(pixels, 64);
    Serial.print("THIS IS AMBIENT::");
    Serial.println(Tamb);
  }
  //getting top 2 rows and calibrating TH2 based on that
  else if (cal_count > 1 && cal_count < 22) {
    delay(1000);
    Serial.println(cal_count);
    float topPixels[16];
    bool personFlag = false;
    for (int i = 0; i < 16; i++) {
      topPixels[i] = applyEquation(pixels[i]);  // Top half (first 16 pixels) (1st and 2nd rows)
      if (topPixels[i] > 34) {
        personFlag = true;
      }
    }

    if (!personFlag) {
      Serial.println("need someone in view");
      delay(5000);
      cal_count--;
    } else {
      insertionSort(topPixels, 16);
      float top8[8];
      for (int i = 0; i < 8; i++) {
        top8[i] = topPixels[i + 8];
      }
      means[cal_count - 2] = calculateMean(top8, 8);
    }

    if (cal_count == 21) {
      float m = calculateMean(means, 20);
      TH2 = m + 3;
      Serial.print("Threshold 2: ");
      Serial.println(TH2);
      Serial.println("Calibration done!");
    }
  }
  //start checking for ppl
  else if (cal_count >= 22) {
    float topPixels[16];
    //the previous grid had a person there
    if (flag1 == true) {
      flag1 = false;
      for (int i = 0; i < 16; i++) {
        topPixels[i] = applyEquation(pixels[i]);  // Top half (first 16 pixels) (1st and 2nd rows)
        if (topPixels[i] > 34) {
          flag1 = true;
        }
      }

      //if there was a person there and now there isn't
      if (flag1 == false) {
        //there was one person there previously
        if (flag2 == false) {
          count++;
        }
        //there were two ppl there previously
        else {
          count += 2;
        }
        //setting flag2 back to false for default
        flag2 = false;
      }
      //there was a person there and there's still a person there
      else {
        insertionSort(topPixels, 16);
        float top8[8];
        for (int i = 0; i < 8; i++) {
          top8[i] = topPixels[i + 8];
        }
        float mean = calculateMean(top8, 8);
        Serial.print("Mean of top 8: ");
        Serial.println(mean);
        //currently there are two ppl there
        if (mean > TH2) {
          flag2 = true;
        }
        //currently there is one person there
        else {
          Serial.println("wow hi debug printing");
          //if there were 2 ppl there previously, add to count
          if (flag2 == true) {
            Serial.println("it should be reaching here but probably not");
            count++;
          }
          //set flag2 back to false (meaning there's one person there)
          flag2 = false;
        }
      }
    }
    //the previous grid had no person
    else {
      Serial.println("there was nobody previously loop");
      //checking for person
      for (int i = 0; i < 16; i++) {
        topPixels[i] = applyEquation(pixels[i]);  // Top half (first 16 pixels) (1st and 2nd rows)
        if (topPixels[i] > 34) {
          flag1 = true;
        }
      }
      //if there is a person here currently
      if (flag1 == true) {
        insertionSort(topPixels, 16);
        float top8[8];
        for (int i = 0; i < 8; i++) {
          top8[i] = topPixels[i + 8];
        }
        float mean = calculateMean(top8, 8);
        Serial.print("Mean of top 8: ");
        Serial.println(mean);
        //two ppl there currently
        if (mean > TH2) {
          flag2 = true;
        }
        //one person there currently
        else {
          flag2 = false;
        }
      }
    }
  }

  //adding to cal_count so it keeps going
  if (cal_count < 22) {
    cal_count++;
  }
  Serial.print(flag1);
  Serial.print(flag2);
  Serial.println(count);
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

float applyEquation(float Tsa) {
  // Apply the equation to convert the temperature
  float Tv = pow(pow(Tsa + 273, 4) + (h * (Tsa - Tamb)) / (sigma * epsilon), 0.25) - 273;
  return Tv;
}

// Insertion Sort function
void insertionSort(float arr[], int n) {
  for (int i = 1; i < n; i++) {
    float key = arr[i];  // The current element to be inserted into the sorted section
    int j = i - 1;

    // Move elements of arr[0..i-1] that are greater than key
    // to one position ahead of their current position
    while (j >= 0 && arr[j] > key) {
      arr[j + 1] = arr[j];
      j = j - 1;  
    }

    // Insert the key into the correct position
    arr[j + 1] = key;
  }
}
