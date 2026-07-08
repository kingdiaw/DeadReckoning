#include <Arduino.h>
#include <Wire.h>
#include <TwoWDController.h>
#include <DeadReckoning.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Initialize your robot controller with your specific hardware configuration
TwoWDController robot(0x20, 4, 2, P2, P3, P0, P1, 18, 19, 25, 13);

// Pass the robot reference to the navigator
DeadReckoning nav(&robot, 0x2C);

byte currentState = 0xFF;
int targetHeading = 0;

void setup() {
  Serial.begin(115200);
  robot.begin();
  robot.resetPID();
  robot.startAutoUpdate(0.033);

  // Initialize the I2C bus and the hardcoded QMC5883P settings
  nav.initCompass();

  Serial.println("Menstabilkan sensor...");
  for (int i = 0; i < 20; i++) {
    nav.getHeading();
    delay(50);
  }

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for (;;)
      ;  // Loop forever if allocation fails
  }

  display.clearDisplay();
  display.setTextSize(5);
  display.setTextColor(WHITE);
  display.display();
}

void loop() {

  targetHeading = (int)nav.getHeading();
  display.clearDisplay();
  display.setTextSize(5);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print(targetHeading);
  display.display();
  delay(100);
}
