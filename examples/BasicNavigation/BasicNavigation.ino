#include <Wire.h>
#include <TwoWDController.h>
#include <DeadReckoning.h>

// Initialize your robot controller with your specific hardware configuration
TwoWDController robot(0x20, 4, 2, P2, P3, P0, P1, 18, 19, 25, 13);

// Pass the robot reference to the navigator
DeadReckoning navigator(&robot);

void setup() {
  Serial.begin(115200);
  
  // Initialize the I2C bus and the hardcoded HMC5883L settings
  navigator.initCompass();
  
  // Give the compass a brief moment to stabilize
  delay(500);
}

void loop() {
  // Move to a heading of 90.0 degrees (East) for 1000 encoder ticks
  navigator.move(90.0, 1000);
  
  // Trap the code in an infinite loop so it doesn't repeat the movement
  while(true) {
    delay(10);
  }
}