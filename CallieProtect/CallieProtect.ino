//Callie Protect™
//Uses Servo Motor Controlled Food Cover To Automatically Cover And Uncover Callie (The Cat)'s Food To Prevent Our Other Cat (Casper) From Eating It

// Hardware:
// -ESP32
// -9g Micro Servo
// -Pushbutton
// -BLE Beacon (IBeacon)


//LIBRARYS
#include <BLEAdvertisedDevice.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <Servo.h>  //ESP Servo Library

//PINS
const int BuiltInLED = 2;              //Always 2 For Build In LED On Board
const int OverrideButton = 4;          //GPIO, This Program Uses Internal Pullup Resistor
static const int ServoSignalPin = 13;  //Must Be PWM GPIO

//SETTINGS
const int CUTOFF = -80;              //Cutoff RSSI Signal, Anything Beacon Above This Threshold Will Trigger Cover To Open
const int ServoClosedPosition = 70;  //Angle In Degrees The Servo Is Considered Closed
const int ServoOpenPosition = 180;   //Angle In Degrees The Servo Is Considered Open
const int ChecksBeforeClosing = 7; //Number of times the device will check for callie, before closing the lid

//VARIABLES
bool ServoOpen = false;
Servo FoodCoverServo;
bool hasName = false;
String beaconName = "";

void setup() {
  pinMode(BuiltInLED, OUTPUT);
  pinMode(OverrideButton, INPUT_PULLUP);
  FoodCoverServo.attach(ServoSignalPin);
  FoodCoverServo.write(ServoClosedPosition);
  BLEDevice::init("");

  Serial.begin(115200);  //For Debugging/Testing
}

bool scanBLE() {
  //Scan For Nearby BLE Beacons
  //THANKS To Davy Wybiral For This Section Of The Code (I Modified This Slighly) (https://gist.github.com/wybiral/2a96c1d1605af7efa11b690586c4b13e)
  BLEScan *scan = BLEDevice::getScan();
  scan->setActiveScan(true);  //Turn On Active Scanning Mode, Uses More Power But More Accurate
  BLEScanResults results = scan->start(1);
  int best = CUTOFF;
  for (int i = 0; i < results.getCount(); i++) {
    Serial.print("Device: ");
    Serial.println(i);
    BLEAdvertisedDevice device = results.getDevice(i);
    int rssi = device.getRSSI();

    hasName = false;
    if (device.haveName()) {
      beaconName = device.getName().c_str();
      hasName = true;
    }

    Serial.print("RSSI: ");
    Serial.println(rssi);
    Serial.print("Name: ");
    Serial.println(beaconName);

    if (rssi > best) {
      if (hasName == true) {
        if (beaconName == "Callie Tag") {
          best = rssi;
        }
      }
    }
  }
  Serial.println("--------------------");
  if (best > CUTOFF || digitalRead(OverrideButton) == LOW) {
    return true;
  } else {
    return false;
  }
}

void loop() {
  if (scanBLE() == true) {
    digitalWrite(BuiltInLED, HIGH);
    if (ServoOpen == false) {
      //Open Food Cover
      for (int posDegrees = ServoClosedPosition; posDegrees <= ServoOpenPosition; posDegrees++) {
        FoodCoverServo.write(posDegrees);
        delay(10);
      }
      ServoOpen = true;
    }
  } else {
    bool CallieLeft = true;
    for (int i = 0; i < ChecksBeforeClosing; i++) {
      if (scanBLE() == true) {
        CallieLeft = false;
      }
    }
    if (CallieLeft == true) {
      digitalWrite(BuiltInLED, LOW);
      //Callie Not Detected AND Override Button Not Pressed
      if (ServoOpen == true) {
        //Close Food Cover
        for (int posDegrees = ServoOpenPosition; posDegrees >= ServoClosedPosition; posDegrees--) {
          FoodCoverServo.write(posDegrees);
          delay(20);
        }
        ServoOpen = false;
      }
    }
  }
}