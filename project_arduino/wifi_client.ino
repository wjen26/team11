/*
 * Example Program that counts the number of times the 
 * boot button is pressed and prints it to a server
 * 
 * It also stores the count in some non-volatile storage.
 * The count will persist through reboots and re-uploads.
 * 
 * If you want to reset the count, you must upload the nonvolatile eraser sketch
 */
#include "WirelessCommunication.h"
#include "sharedVariable.h"
#include "Preferences.h"

uint32_t is_pressed();
void init_non_vol_storage();
void update_non_vol_count();
void update_people_count();

volatile uint32_t num_people = 0;
volatile shared_uint32 x;
Preferences nonVol;//used to store the count in nonvolatile memory

void setup()
{
  Serial.begin(115200);
  init_wifi_task();
  init_non_vol_count();//initializes nonvolatile memory and retrieves latest count
  INIT_SHARED_VARIABLE(x, num_people);//init shared variable used to tranfer info to WiFi core
}

void loop()
{           
  //check if Boot button has been pressed and update values if needed
  if(is_pressed())
  {
    ++num_people;
    update_people_count();//update shared variable x (shared with WiFi task)
    update_non_vol_count();//updates nonvolatile count 
  }
  Serial.println(num_people);
}

uint32_t is_pressed()
{
  if(!digitalRead(BUTTON_PIN))
  {
    delay(10);//software debouncing
    if(!digitalRead(BUTTON_PIN))
    {
      while(!digitalRead(BUTTON_PIN));//make sure button is depressed
      return 1;
    }
  }
  return 0;
}

//initializes nonvolatile memory and retrieves latest count
void init_non_vol_count()
{
  nonVol.begin("nonVolData", false);//Create a “storage space” in the flash memory called "nonVolData" in read/write mode
  num_people = nonVol.getUInt("num_people", 0);//attempts to retrieve "count" from nonVolData, sets it 0 if not found
}

//updates nonvolatile memery with lates value of count
void update_non_vol_count()
{
  nonVol.putUInt("num_people", num_people);//write count to nonvolatile memory
}

//example code that updates a shared variable (which is printed to server)
//under the hood, this implementation uses a semaphore to arbitrate access to x.value
void update_people_count()
{
  //minimized time spend holding semaphore
  LOCK_SHARED_VARIABLE(x);
  x.value = num_people;
  UNLOCK_SHARED_VARIABLE(x);   
}
