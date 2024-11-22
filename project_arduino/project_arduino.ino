#include <Arduino.h>
#include <Wire.h>
#include <vl53l4cx_class.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <Adafruit_AMG88xx.h>
#include "TOFProcessor.h"
#include "TOFStateMachine.h"

#define DEV_I2C Wire

// address we will assign if dual sensor is present
#define L4CX1_ADDRESS 0x29
#define L4CX2_ADDRESS 0x31

// set the pins to shutdown
#define SHT_L4CX1 0
#define SHT_L4CX2 4

Adafruit_AMG88xx amg;

float pixels[AMG88xx_PIXEL_ARRAY_SIZE];

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

TOFProcessor tofprocessor;
TOFStateMachine tofstatemachine;

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

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  //Serial.println("Starting...");
  
  // wait until serial port opens for native USB devices
  while (! Serial) {
    delay(1);
  }

  DEV_I2C.begin();

  pinMode(SHT_L4CX1, OUTPUT);
  pinMode(SHT_L4CX2, OUTPUT);

  setID();

  
  /*bool status;
  status = amg.begin();
  if (!status) {
      Serial.println("Could not find a valid AMG88xx sensor, check wiring!");
      while (1);
  }*/
    
  l4cx1.VL53L4CX_StartMeasurement();
  l4cx2.VL53L4CX_StartMeasurement();


  delay(100);
}


void loop() {
  // put your main code here, to run repeatedly:
  float TOF1;
  float TOF2;
  read_dual_sensors(TOF1, TOF2);
  //Serial.print(TOF1);
  //Serial.print("     ");
  //Serial.print(TOF2);
  //Serial.print("     ");

  if (TOF1 == -1 || TOF1 > 2000) {
    TOF1_flag = false;
  }
  else if (TOF1 != -2) {
    TOF1_flag = true;
  }
  if (TOF2 == -1 || TOF2 > 2000) {
    TOF2_flag = false;
  }
  else if (TOF2 != -2) {
    TOF2_flag = true;
  }
  //Serial.print(TOF1_flag);
  //Serial.print("     ");
  //Serial.print(TOF2_flag);
  //Serial.print("     ");

  /*tofprocessor.process(TOF1, TOF2);
  Serial.print("number of people in the room: ");
  Serial.println(tofprocessor.numPeople);*/
  
  tofstatemachine.updateState(TOF1_flag, TOF2_flag);
  //tofstatemachine.printState();
  //Serial.print("number of people in the room: ");
  Serial.println(tofstatemachine.numPeople);

  //read all the pixels
  /*amg.readPixels(pixels);

  //Serial.print("[");
  for(int i=0; i<AMG88xx_PIXEL_ARRAY_SIZE; i++){
    Serial.print(pixels[i]);
    if (i < AMG88xx_PIXEL_ARRAY_SIZE - 1) {
      Serial.print(",");
    }
  }*/
  //Serial.println("]");

  //tofprocessor.printStatus();

  //delay a second
  delay(10);
}
