#include <Arduino.h>
#include <Wire.h>
#include <vl53l4cx_class.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>

#define DEV_I2C Wire
//#define SerialPort Serial

// address we will assign if dual sensor is present
#define L4CX1_ADDRESS 0x29
#define L4CX2_ADDRESS 0x31

// set the pins to shutdown
#define SHT_L4CX1 0
#define SHT_L4CX2 4

VL53L4CX l4cx1(&DEV_I2C, SHT_L4CX1);
VL53L4CX l4cx2(&DEV_I2C, SHT_L4CX2);


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Starting...");
  
  // wait until serial port opens for native USB devices
  while (! Serial) {
    delay(1);
  }

  DEV_I2C.begin();

  pinMode(SHT_L4CX1, OUTPUT);
  pinMode(SHT_L4CX2, OUTPUT);

  digitalWrite(SHT_L4CX1, LOW);
  digitalWrite(SHT_L4CX2, LOW);
  delay(10);

/*
  digitalWrite(SHT_L4CX1, HIGH);
  digitalWrite(SHT_L4CX2, HIGH);
  delay(10);
*/
  digitalWrite(SHT_L4CX1, HIGH);
  //digitalWrite(SHT_L4CX2, LOW);
  delay(10);

  // Configure VL53L4CX satellite component.
  l4cx1.begin();

  // Switch off VL53L4CX satellite component.
  l4cx1.VL53L4CX_Off();

  //Initialize VL53L4CX satellite component.
  l4cx1.InitSensor(L4CX1_ADDRESS);
  l4cx1.VL53L4CX_SetDeviceAddress(L4CX1_ADDRESS);

  /*
  digitalWrite(SHT_L4CX1, LOW);
  delay(10);
  digitalWrite(SHT_L4CX1, HIGH);
  delay(10);
  */

  digitalWrite(SHT_L4CX2, HIGH);
  //digitalWrite(SHT_L4CX1, LOW);
  delay(10);

  // Configure VL53L4CX satellite component.
  l4cx2.begin();

  // Switch off VL53L4CX satellite component.
  l4cx2.VL53L4CX_Off();

  //Initialize VL53L4CX satellite component.
  l4cx2.InitSensor(L4CX2_ADDRESS);
  l4cx2.VL53L4CX_SetDeviceAddress(L4CX2_ADDRESS);

  /*
  digitalWrite(SHT_L4CX2, LOW);
  delay(10);
  digitalWrite(SHT_L4CX2, HIGH);
  delay(10);
  */
  //digitalWrite(SHT_L4CX2, HIGH);
  //digitalWrite(SHT_L4CX1, HIGH);
  delay(10);

  // Start Measurements
  l4cx1.VL53L4CX_StartMeasurement();
  l4cx2.VL53L4CX_StartMeasurement();
  //count = 0;

  delay(100);

}

void loop() {
  // put your main code here, to run repeatedly:
  VL53L4CX_MultiRangingData_t MultiRangingData1;
  VL53L4CX_MultiRangingData_t *pMultiRangingData1 = &MultiRangingData1;
  uint8_t NewDataReady1 = 0;
  int no_of_object_found1 = 0, j;
  char report1[64];
  int status1;

  do {
    status1 = l4cx1.VL53L4CX_GetMeasurementDataReady(&NewDataReady1);
  } while (!NewDataReady1);


  if ((!status1) && (NewDataReady1 != 0)) {
    status1 = l4cx1.VL53L4CX_GetMultiRangingData(pMultiRangingData1);
    no_of_object_found1 = pMultiRangingData1->NumberOfObjectsFound;
    snprintf(report1, sizeof(report1), "VL53L4CX Satellite 1: Count=%d, #Objs=%1d ", pMultiRangingData1->StreamCount, no_of_object_found1);
    Serial.print(report1);
    for (j = 0; j < no_of_object_found1; j++) {
      if (j != 0) {
        Serial.print("\r\n                               ");
      }
      Serial.print("status=");
      Serial.print(pMultiRangingData1->RangeData[j].RangeStatus);
      Serial.print(", D=");
      Serial.print(pMultiRangingData1->RangeData[j].RangeMilliMeter);
      Serial.print("mm");
      //Serial.print(", Signal=");
      //Serial.print((float)pMultiRangingData1->RangeData[j].SignalRateRtnMegaCps / 65536.0);
      //Serial.print(" Mcps, Ambient=");
      //Serial.print((float)pMultiRangingData1->RangeData[j].AmbientRateRtnMegaCps / 65536.0);
      //Serial.print(" Mcps");
    }
    //Serial.println("");
    if (status1 == 0) {
      status1 = l4cx1.VL53L4CX_ClearInterruptAndStartMeasurement();
    }
  }

  VL53L4CX_MultiRangingData_t MultiRangingData2;
  VL53L4CX_MultiRangingData_t *pMultiRangingData2 = &MultiRangingData2;
  uint8_t NewDataReady2 = 0;
  int no_of_object_found2 = 0, i;
  char report2[64];
  int status2;

  do {
    status2 = l4cx2.VL53L4CX_GetMeasurementDataReady(&NewDataReady2);
  } while (!NewDataReady2);


  if ((!status2) && (NewDataReady2 != 0)) {
    status2 = l4cx2.VL53L4CX_GetMultiRangingData(pMultiRangingData2);
    no_of_object_found2 = pMultiRangingData2->NumberOfObjectsFound;
    snprintf(report2, sizeof(report2), "    VL53L4CX Satellite 2: Count=%d, #Objs=%1d ", pMultiRangingData2->StreamCount, no_of_object_found2);
    Serial.print(report2);
    for (i = 0; i < no_of_object_found2; i++) {
      if (i != 0) {
        Serial.print("\r\n                               ");
      }
      Serial.print("status=");
      Serial.print(pMultiRangingData2->RangeData[i].RangeStatus);
      Serial.print(", D=");
      Serial.print(pMultiRangingData2->RangeData[i].RangeMilliMeter);
      Serial.print("mm");
      //Serial.print(", Signal=");
      //Serial.print((float)pMultiRangingData2->RangeData[i].SignalRateRtnMegaCps / 65536.0);
      //Serial.print(" Mcps, Ambient=");
      //Serial.print((float)pMultiRangingData2->RangeData[i].AmbientRateRtnMegaCps / 65536.0);
      //Serial.print(" Mcps");
    }
    Serial.println("");
    if (status2 == 0) {
      status2 = l4cx2.VL53L4CX_ClearInterruptAndStartMeasurement();
    }
  }
  
  delay(100);
  
}
