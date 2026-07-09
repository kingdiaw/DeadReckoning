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

uint8_t state = 0;
int targetHeading = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin();

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
    Serial.println("Gagal mengesan paparan OLED!");
    for (;;) { delay(100); }  // Berhenti selama-lamanya jika OLED gagal
  }

  display.clearDisplay();
  display.setTextSize(5);
  display.setTextColor(WHITE);
  display.display();

  pinMode(39, INPUT);
  //Menunggu butang ditekan di pin 39...
  while (digitalRead(39)) {
    targetHeading = (int)nav.getHeading();
    OLED_display(targetHeading);
    delay(100);
  }
}

void loop() {
  nav.update();

  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 100) {
    lastPrint = millis();

    if (nav.isMoving()) {
      int heading = (int)nav.getHeading();
      OLED_display(heading);
    }
  }

  switch (state) {
    case 0:
      //Mula pergerakan lurus seterusnya ke arah 185.0 darjah sejauh 11260 tick
      nav.startMove(185.0, 11260);
      state = 1;
      break;
    case 1:
      // Tunggu sehingga pergerakan lurus tamat
      if (!nav.isMoving()) {
        robot.resetEncoders();  // PENTING: Reset encoder supaya kiraan belokan di state 2 mula dari 0
        state = 2;
      }
      break;
    case 2:
      // Pusing ke kanan (belokan mekanikal biasa tanpa kompas)
      robot.drive(0.12, -0.12);

      // Berhenti pusing setelah tayar kiri capai 2400 tick
      if (robot.getLeftPosition() > 2400) {
        robot.stop();
        delay(1000);
        state = 3;
      }
      break;
    case 3:
      // Mula pergerakan lurus seterusnya ke arah 130.0 darjah sejauh 16880 tick
      nav.startMove(130.0, 16880);
      state = 6;
      break;

    case 4:

      break;
    case 5:
      break;

    case 6:
      // Tunggu sehingga destinasi dicapai
      if (!nav.isMoving()) {
        Serial.println("Dah Sampai Destinasi");
        while (true) {
          delay(100);  // Berhenti selama-lamanya dengan selamat
        }
      }
      break;
  }
}

void OLED_display(int d) {
  display.clearDisplay();
  display.setTextSize(5);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print(d);
  display.display();
}