#include <Wire.h>
#include <vl53l4cx_class.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <Adafruit_AMG88xx.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "WirelessCommunication.h"
#include "sharedVariable.h"

#define DEV_I2C Wire

// address we will assign if dual sensor is present
#define L4CX1_ADDRESS 0x29
#define L4CX2_ADDRESS 0x31

// set the pins to shutdown
#define SHT_L4CX1 4
#define SHT_L4CX2 0

Adafruit_AMG88xx amg;


// objects for the TOF sensors
VL53L4CX l4cx1(&DEV_I2C, SHT_L4CX1);
VL53L4CX l4cx2(&DEV_I2C, SHT_L4CX2);


VL53L4CX_MultiRangingData_t MultiRangingData1;
VL53L4CX_MultiRangingData_t *pMultiRangingData1 = &MultiRangingData1;
uint8_t NewDataReady1 = 0;
int no_of_object_found1 = 0;
int status1;

VL53L4CX_MultiRangingData_t MultiRangingData2;
VL53L4CX_MultiRangingData_t *pMultiRangingData2 = &MultiRangingData2;
uint8_t NewDataReady2 = 0;
int no_of_object_found2 = 0;
int status2;

bool TOF1_flag = false;
bool TOF2_flag = false;
bool TOF1_flag_prev = false;
bool TOF2_flag_prev = false;

int invalid1 = 0;
int invalid2 = 0;

bool thermal_entering = false;
bool thermal_exiting = false;
int thermal_entering_counter = 0;
int thermal_exiting_counter = 0;

int count = 0;

/*
    Reset all sensors by setting all of their XSHUT pins low for delay(10), then set all XSHUT high to bring out of reset
    Keep sensor #1 awake by keeping XSHUT pin high
    Put all other sensors into shutdown by pulling XSHUT pins low
    Initialize sensor #1 with lox.begin(new_i2c_address) Pick any number but 0x29 and it must be under 0x7F. Going with 0x30 to 0x3F is probably OK.
    Keep sensor #1 awake, and now bring sensor #2 out of reset by setting its XSHUT pin high.
    Initialize sensor #2 with lox.begin(new_i2c_address) Pick any number but 0x29 and whatever you set the first sensor to
 */
void setID() {
  digitalWrite(SHT_L4CX1, LOW);
  digitalWrite(SHT_L4CX2, LOW);
  delay(10);

  digitalWrite(SHT_L4CX1, HIGH);
  delay(10);

  // Configure VL53L4CX satellite component.
  l4cx1.begin();

  // Switch off VL53L4CX satellite component.
  l4cx1.VL53L4CX_Off();

  //Initialize VL53L4CX satellite component.
  l4cx1.InitSensor(L4CX1_ADDRESS);
  l4cx1.VL53L4CX_SetDeviceAddress(L4CX1_ADDRESS);


  digitalWrite(SHT_L4CX2, HIGH);
  delay(10);

  // Configure VL53L4CX satellite component.
  l4cx2.begin();

  // Switch off VL53L4CX satellite component.
  l4cx2.VL53L4CX_Off();

  //Initialize VL53L4CX satellite component.
  l4cx2.InitSensor(L4CX2_ADDRESS);
  l4cx2.VL53L4CX_SetDeviceAddress(L4CX2_ADDRESS);

  delay(10);
}

void read_dual_sensors(float &TOF1, float &TOF2) {
  //sensing TOF1
  do {
    status1 = l4cx1.VL53L4CX_GetMeasurementDataReady(&NewDataReady1);
  } while (!NewDataReady1);

  if ((!status1) && (NewDataReady1 != 0)) {
    status1 = l4cx1.VL53L4CX_GetMultiRangingData(pMultiRangingData1);
    no_of_object_found1 = pMultiRangingData1->NumberOfObjectsFound;
    if (no_of_object_found1 == 0) {
      TOF1 = -1;
    }
    else if (pMultiRangingData1->RangeData[0].RangeStatus == 0) {
      TOF1 = pMultiRangingData1->RangeData[0].RangeMilliMeter;
      //maybe check status???
    }
    else {
      TOF1 = -2;
    }
    if (status1 == 0) {
      status1 = l4cx1.VL53L4CX_ClearInterruptAndStartMeasurement();
    }
  }


  //sensing TOF2
  do {
    status2 = l4cx2.VL53L4CX_GetMeasurementDataReady(&NewDataReady2);
  } while (!NewDataReady2);

  if ((!status2) && (NewDataReady2 != 0)) {
    status2 = l4cx2.VL53L4CX_GetMultiRangingData(pMultiRangingData2);
    no_of_object_found2 = pMultiRangingData2->NumberOfObjectsFound;
    if (no_of_object_found2 == 0) {
      TOF2 = -1;
    }
    else if (pMultiRangingData2->RangeData[0].RangeStatus == 0) {
      TOF2 = pMultiRangingData2->RangeData[0].RangeMilliMeter;
      //maybe check status???
    }
    else {
      TOF2 = -2;
    }
    if (status2 == 0) {
      status2 = l4cx2.VL53L4CX_ClearInterruptAndStartMeasurement();
    }
  }
}

// Constants
const float TH2 = 40;  // Mean threshold for detecting two people
const float PERSON_TEMP_THRESHOLD = 33.0;
const float Tamb = 18.86;

// Variables
float pixels[AMG88xx_PIXEL_ARRAY_SIZE];
bool flag1_Ent = false;  // Indicates if a person was previously detected
bool flag2_Ent = false;  // Indicates if two people were previously detected
bool flag1_Ext = false;  // Indicates if a person was previously detected
bool flag2_Ext = false;  // Indicates if two people were previously detected

volatile uint32_t person_count = 0;
volatile shared_uint32 x;

// RTOS Task Handles
TaskHandle_t thermalTaskHandle;

void setup() {
  Serial.begin(115200);
  Wire.setClock(400000);
  // Initialize AMG8833 sensor
  if (!amg.begin()) {
    Serial.println("Could not find a valid AMG88xx sensor, check wiring!");
    while (1)
      ;
  }

  DEV_I2C.begin();

  pinMode(SHT_L4CX1, OUTPUT);
  pinMode(SHT_L4CX2, OUTPUT);

  setID();

    
  l4cx1.VL53L4CX_StartMeasurement();
  l4cx2.VL53L4CX_StartMeasurement();

  init_wifi_task();
  INIT_SHARED_VARIABLE(x, person_count);

  // Create a task for thermal data processing
  xTaskCreatePinnedToCore(
    processData,  // Task function
    "DoorTask",       // Task name
    4096,                // Stack size (bytes)
    NULL,                // Parameters
    1,                   // Task priority
    &thermalTaskHandle,  // Task handle
    0                    // Core to run on
  );
}

void loop() {
  // Main loop remains idle or can handle other tasks
  delay(1000);
}

void processData(void *param) {
  while (true) {
    // put your main code here, to run repeatedly:
    update_people_count();
  float TOF1;
  float TOF2;
  read_dual_sensors(TOF1, TOF2);
  // Serial.print(TOF1);
  // Serial.print("     ");
  // Serial.print(TOF2);
  // Serial.print("     ");

  if (TOF1 == -1 || TOF1 > 2000) {
    TOF1_flag_prev = TOF1_flag;
    TOF1_flag = false;
    invalid1 = 0;
  }
  else if (TOF1 != -2) {
    TOF1_flag_prev = TOF1_flag;
    TOF1_flag = true;
    invalid1 = 0;
  }
  else if (invalid1 > 3) {
    TOF1_flag_prev = TOF1_flag;
    TOF1_flag = false;
    invalid1++;
  }
  else {
    invalid1++;
  }
  if (TOF2 == -1 || TOF2 > 2000) {
    TOF2_flag_prev = TOF2_flag;
    TOF2_flag = false;
    invalid2 = 0;
  }
  else if (TOF2 != -2) {
    TOF2_flag_prev = TOF2_flag;
    TOF2_flag = true;
    invalid2 = 0;
  }
  else if (invalid2 > 3) {
    TOF2_flag_prev = TOF2_flag;
    TOF2_flag = false;
    invalid2++;
  }
  else {
    invalid2++;
  }
  Serial.print(TOF1_flag);
  Serial.print("     ");
  Serial.print(TOF2_flag);
  Serial.print("     ");

  if (!thermal_entering && !thermal_exiting && !TOF1_flag_prev && !TOF2_flag_prev && TOF1_flag && !TOF2_flag) {
    thermal_entering_counter = 0;
    thermal_entering = true;
    thermal_exiting = false;
  }
  else if (thermal_entering && (thermal_entering_counter < 30) && !TOF1_flag && !TOF2_flag && !TOF1_flag_prev && !TOF2_flag_prev) {
    thermal_entering_counter++;
  }
  else if (thermal_entering && (thermal_entering_counter >= 30) && !TOF1_flag && !TOF2_flag && !TOF1_flag_prev && !TOF2_flag_prev) {
    Serial.println("leaving Thermal entering");
    thermal_entering_counter = 0;
    thermal_entering = false;
    flag1_Ent = false;
    flag2_Ent = false;
  }
  else {
    thermal_entering_counter = 0;
  }
  if (!thermal_exiting && !thermal_entering && !TOF1_flag_prev && !TOF2_flag_prev && !TOF1_flag && TOF2_flag) {
    thermal_exiting_counter = 0;
    thermal_exiting = true;
    thermal_entering = false;
  }
  else if (thermal_exiting && (thermal_exiting_counter < 30) && !TOF1_flag && !TOF2_flag && !TOF1_flag_prev && !TOF2_flag_prev) {
    thermal_exiting_counter++;
  }
  else if (thermal_exiting && (thermal_exiting_counter >= 30) && !TOF1_flag && !TOF2_flag && !TOF1_flag_prev && !TOF2_flag_prev) {
    Serial.println("leaving Thermal exiting");
    thermal_exiting_counter = 0;
    thermal_exiting = false;
    flag1_Ext = false;
    flag2_Ext = false;
  }
  else {
    thermal_exiting_counter = 0;
  }

  if (thermal_entering) {
    Serial.print("Thermal Entering      ");
    // Read pixel data
    amg.readPixels(pixels);

    // Process top half of the grid
    float topPixels[16];
    for (int i = 0; i < 16; i++) {
      topPixels[i] = applyEquation(pixels[i]);  // Convert raw temperatures
    }

    // Check for changes in presence and update the person count
    if (flag1_Ent) {
      Serial.print("Previously there was a person there        ");
      // Check if a person is still present
      flag1_Ent = false;
      for (int i = 0; i < 16; i++) {
        if (topPixels[i] > PERSON_TEMP_THRESHOLD) {
          flag1_Ent = true;
          break;
        }
      }

      if (!flag1_Ent) {
        // Person(s) have left
       Serial.println("The guy left crazy");
        if (flag2_Ent) {
          person_count += 2;
        } else {
          person_count++;
        }
        flag2_Ent = false;  // Reset the two-person flag
      } else {
        // Person is still present
        Serial.print("The person is still there     ");
        detectTwoPeopleEntering(topPixels);
      }
    } else {
      // No one was detected previously
      Serial.print("There was nobody previously in the grid           ");
      flag1_Ent = false;
      for (int i = 0; i < 16; i++) {
        if (topPixels[i] > PERSON_TEMP_THRESHOLD) {
          flag1_Ent = true;
          break;
        }
      }

      if (flag1_Ent) {
        // A person is now detected
        detectTwoPeopleEntering(topPixels);
      }
    }
  }
  //if thermal exiting
  if (thermal_exiting) {
    Serial.print("Thermal Exiting      ");
    // Read pixel data
    amg.readPixels(pixels);

    // Process bottom 2 rows of the grid
    float bottomPixels[16];
    for (int i = 0; i < 16; i++) {
      bottomPixels[i] = applyEquation(pixels[i + 48]);  // Convert raw temperatures
    }

    // Check for changes in presence and update the person count
    if (flag1_Ext) {
      Serial.print("Previously there was a person there        ");
      // Check if a person is still present
      flag1_Ext = false;
      for (int i = 0; i < 16; i++) {
        if (bottomPixels[i] > PERSON_TEMP_THRESHOLD) {
          flag1_Ext = true;
          break;
        }
      }

      if (!flag1_Ext) {
        // Person(s) have left
        Serial.println("The guy left crazy");
        if (flag2_Ext) {
          person_count -= 2;
        } else {
          person_count--;
        }
        flag2_Ext = false;  // Reset the two-person flag
      } else {
        // Person is still present
        Serial.print("The person is still there     ");
        detectTwoPeopleExiting(bottomPixels);
      }
    } else {
      // No one was detected previously
      Serial.print("There was nobody previously in the grid           ");
      flag1_Ext = false;
      for (int i = 0; i < 16; i++) {
        if (bottomPixels[i] > PERSON_TEMP_THRESHOLD) {
          flag1_Ext = true;
          break;
        }
      }

      if (flag1_Ext) {
        // A person is now detected
        detectTwoPeopleExiting(bottomPixels);
      }
    }
  }
    // Print the current person count
    Serial.printf("Person Count: %d\n", person_count);

    // Match sensor refresh rate
    vTaskDelay(pdMS_TO_TICKS(25));
  }
}

void insertionSort(float arr[], int size) {
  for (int i = 1; i < size; i++) {
    float key = arr[i];
    int j = i - 1;

    // Move elements of arr[0..i-1], that are greater than key, to one position ahead
    while (j >= 0 && arr[j] > key) {
      arr[j + 1] = arr[j];
      j--;
    }
    arr[j + 1] = key;
  }
}

float calculateMean(float arr[], int size) {
  float sum = 0.0f;
  for (int i = 0; i < size; i++) {
    sum += arr[i];
  }
  return sum / size;
}

void detectTwoPeopleEntering(float *topPixels) {
  // Sort the top half of the grid
  insertionSort(topPixels, 16);

  // Calculate the mean of the top 8 highest temperatures
  float top8[8];
  for (int i = 0; i < 8; i++) {
    top8[i] = topPixels[i + 8];
  }
  float mean = calculateMean(top8, 8);

   Serial.print("Mean of top 8: ");
   Serial.println(mean);

  if (mean > TH2) {
    Serial.println("Two people detected    ");
    flag2_Ent = true;  // Two people are detected
  } else {
    Serial.println("One person detected    ");
    if (flag2_Ent) {
      Serial.println("Uh oh......    ");
      person_count++;  // Increment count for one person leaving
    }
    flag2_Ent = false;  // Reset the two-person flag
  }
}

void detectTwoPeopleExiting(float *bottomPixels) {
  // Sort the bottom 2 rows of the grid
  insertionSort(bottomPixels, 16);

  // Calculate the mean of the top 8 highest temperatures
  float top8[8];
  for (int i = 0; i < 8; i++) {
    top8[i] = bottomPixels[i + 8];
  }
  float mean = calculateMean(top8, 8);

  Serial.print("Mean of top 8: ");
  Serial.println(mean);

  if (mean > TH2) {
    Serial.println("Two people detected    ");
    flag2_Ext = true;  // Two people are detected
  } else {
    Serial.println("One person detected    ");
    if (flag2_Ext) {
      Serial.println("Uh oh......    ");
      person_count--;  // Increment count for one person leaving
    }
    flag2_Ext = false;  // Reset the two-person flag
  }
}

float applyEquation(float Tsa) {
  // Apply the optimized temperature conversion equation
  return pow(pow(Tsa + 273.15f, 4) + (15.0 * (Tsa - Tamb)) / (5.67e-8 * 0.98), 0.25f) - 273.15f;
}

static inline void update_people_count()
{
  //minimized time spend holding semaphore
  LOCK_SHARED_VARIABLE(x);
  x.value = person_count;
  UNLOCK_SHARED_VARIABLE(x);   
}
