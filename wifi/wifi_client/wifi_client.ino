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
bool flag = false;

void setup()
{
  Serial.begin(115200);
  init_wifi_task();
  INIT_SHARED_VARIABLE(x, num_people);//init shared variable used to tranfer info to WiFi core
}

void loop()
{           
  //check if Boot button has been pressed and update values if needed
  if(millis() % 5000 > 2500 && flag)
  {
    ++num_people;
    update_people_count();//update shared variable x (shared with WiFi task)
    flag = false;
  }
  if (millis() % 5000 <= 2500 && !flag) {
    flag  = true;
  }
  Serial.println(num_people);
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
