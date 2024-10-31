#include "Adafruit_VL53L0X.h"
#include <Wire.h>
#include <Adafruit_AMG88xx.h>

Adafruit_AMG88xx amg;

float pixels[AMG88xx_PIXEL_ARRAY_SIZE];

Adafruit_VL53L0X lox = Adafruit_VL53L0X();

void setup() {
  Serial.begin(9600);

  bool status;
  
  // wait until serial port opens for native USB devices
  while (! Serial) {
    delay(1);
  }
  
  //Serial.println("Adafruit VL53L0X test");
  if (!lox.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
    while(1);
  }
  status = amg.begin();
  if (!status) {
      Serial.println("Could not find a valid AMG88xx sensor, check wiring!");
      while (1);
  }
    
  // power 
  //Serial.println(F("VL53L0X API Simple Ranging example\n\n")); 
  delay(100);
}


void loop() {
  VL53L0X_RangingMeasurementData_t measure;
    
  //Serial.print("Reading a measurement... ");
  lox.rangingTest(&measure, false); // pass in 'true' to get debug data printout!

  if (measure.RangeStatus != 4) {  // phase failures have incorrect data
    //Serial.print("Distance (mm): ");
    Serial.print(measure.RangeMilliMeter);

  } else {
    Serial.print(-1);
  }
  Serial.print(",");

    
  //read all the pixels
    amg.readPixels(pixels);

    //Serial.print("[");
    for(int i=0; i<AMG88xx_PIXEL_ARRAY_SIZE; i++){
      Serial.print(pixels[i]);
      if (i < AMG88xx_PIXEL_ARRAY_SIZE - 1) {
        Serial.print(",");
      }
    }
    //Serial.println("]");
    Serial.println();

    //delay a second
    delay(25);
}
