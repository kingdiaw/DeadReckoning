#include <Wire.h>
#include <TwoWDController.h>
#include <DeadReckoning.h>

// Initialize your robot controller with your specific hardware configuration
TwoWDController robot(0x20, 4, 2, P2, P3, P0, P1, 18, 19, 25, 13);

DeadReckoning navigator(&robot);

void setup() {
  Serial.begin(115200);
  navigator.initCompass();
  
  Serial.println("Compass Debugger Initialized...");
}

void loop() {
  // Fetch the current heading in degrees
  float heading = navigator.getHeading();
  
  Serial.print("Current Heading: ");
  Serial.println(heading);
  
  // A 70ms delay perfectly matches the 15Hz continuous update rate of the HMC5883L
  delay(70); 
}