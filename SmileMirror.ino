/*
 * SmileMirror
 * 3/MAY/2020 - 9/MAY/2020 | Andrei Florian
 */

// Huskylens
#include "HUSKYLENS.h" // library to comunicate with the camera module
#include "SoftwareSerial.h"

// Wia and Yun
#include <Bridge.h>
#include <BlynkSimpleYun.h>
#include <WidgetRTC.h>

// objects
HUSKYLENS huskylens; // define object
SoftwareSerial mySerial(10, 11); // RX, TX
WidgetRTC rtc;

// globals
float waitTime = 120; // time to wait until sending message (seconds)
bool counting = false; // are we counting time?
time_t currentTime;
time_t startTime;

int errorNr = 0; 
int errorAdmital = 25; // quit on 25 (cannot be bigger than waitTime)

// CHANGE: Insert your Blynk token here
char auth[] = ""; // insert project token

// define messages
int numberOfMessages = 4; // set manually to reduce use of memory
String inspirationalMessages[] = // you can change the messages just be aware of the memory on the device
{
  "Keep Going!",
  "You can do this!",
  "Your'e doing great!",
  "Show them what you've got!"
};

// chose a random message from the list
String getMessage()
{
  int i = random(0, (numberOfMessages - 1));
  Serial.println("[loop] Sending " + inspirationalMessages[i]);
  return inspirationalMessages[i];
}

// process the data coming from the Huskylens module
bool processResult(HUSKYLENSResult result) // only runs if block detected
{
  if(result.command == COMMAND_RETURN_BLOCK) // if block returned
  {
    if((currentTime - startTime) > waitTime) // check if the waiting time is over
    {
      if(counting) // if so, send the phone notification
      {
        Serial.println("");
        Serial.println("[loop] Face Identified for " + String(waitTime) + " seconds");

        Blynk.notify(getMessage()); // send the inspirational message
        delay(30 * 60 * 1000); // Set to 30 minutes. If device does not resume running the code, consider using an alarm from time library
        
        errorNr = 0;
        counting = false;
      }
    }
    
    if(result.ID == 0) // if face not recognised
    {
      Serial.println("Face Not Recognised ID: " + String(result.ID));
      
      if(counting)
      {
        errorNr = 0;
      }
    }
    else // if face is recognised
    {
      Serial.println("Face Recognised ID: " + String(result.ID));

      if(!counting) // if the face was just noticed, start counting
      {
        startTime = now();
        Serial.println("face set at" + String(startTime));
      }
      
      counting = true;
      errorNr = 0;
    }
  }

  return true;
}

// get the data from the camera module
void processCamera()
{
  if(huskylens.request())
  {
    if(huskylens.available())
    {
      while(huskylens.available())
      {
          HUSKYLENSResult result = huskylens.read();
          processResult(result);
      }
    }
    else
    {
      errorNr++;

      if(errorNr > errorAdmital) // if face not detected for a while, stop counting
      {
        startTime = 0;
        counting = false;
      }

      if(errorNr > 1000) // reset value to prevent overspill
      {
        errorNr = 0;
      }
    }
  }
}

void setup()
{
  Serial.begin(9600);
  //while(!Serial); // uncomment if debugging
  delay(1000);
  
  Serial.println("SmileMirror");
  Serial.println("9/MAY/2020 | Andrei Florian");
  Serial.println("");

  Serial.println("[Setup] Starting Yun Bridge Connection to Linux");
  Blynk.begin(auth);
  Serial.println("[Setup] Connection Successful, Blynk Online");

  Serial.println("[Setup]Setting up Blynk RTC");
  rtc.begin();
  Serial.println("[Setup] Timer Setup Complete");
  
  Serial.println("[Setup] Beginning Serial");
  mySerial.begin(9600);
  Serial.println("[Setup] Serial Communication Online");

  while(!huskylens.begin(mySerial)) // start the serial connection to huskylens
  {
    Serial.println("[Setup] Huskylens setup returned error");
    delay(1000);
  }
  
  huskylens.writeAlgorithm(ALGORITHM_FACE_RECOGNITION); // prepare for facial recognition
  Serial.println("[Setup] Huskylens Online");

  // start by displaying some info
  if(huskylens.request())
  {
    if(!huskylens.isLearned())
    {
      Serial.println("[Info] WARNING: No IDs registered");
    }
    else
    {
      Serial.println("[Info] Count of Learned IDs: " + String(huskylens.countLearnedIDs()));
    }
  }

  Serial.println("");
  Serial.println("");
  delay(1000);
}

void loop()
{
  Blynk.run(); // periodic tasks with Blynk
  processCamera(); // get data from huskylens
  
  currentTime = now(); // get the time from Blynk
  delay(300);

  // comment if debugging. Dumps all variables to serial monitor.
  /*
  Serial.println(String(currentTime));
  Serial.println(String(errorNr));
  Serial.println(String(startTime));
  Serial.println(String(currentTime - startTime));
  Serial.println(counting);
  Serial.println("");
  */
}
