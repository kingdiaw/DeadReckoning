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

uint8_t state = 0;

const int trigPin = 23; 
const int echoPin = 26;

// Pembolehubah untuk pengurusan masa
unsigned long masaSebelumnya = 0;
const long interval = 20; // Baca sensor setiap 60ms (cukup pantas untuk aplikasi biasa)
float jarakTerakhir = 0;   // Menyimpan nilai jarak terakhir yang dibaca
float jarak;

void setup() {
  Serial.begin(115200);
  Wire.begin();

  robot.begin();
  robot.resetPID();
  robot.startAutoUpdate(0.033);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

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
    jarak = bacaJarak();
    OLED_display(jarak);
    delay(100);
  }
}

void loop() {
  jarak = bacaJarak();

  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 100) {
    lastPrint = millis();
    OLED_display(jarak);  
  }

  switch (state) {
    case 0:
      //Mula pergerakan lurus ke hadapan menuju ke check point A
      robot.drive(0.12, 0.12);
      if(jarak <= 30) {
        robot.stop();
        delay(1000);
        robot.resetEncoders();  
        Serial.println(robot.getLeftPosition());
        state = 1;
      }
      break;
    case 1:
    //pivot Clock wise
      robot.drive(0.12, -0.12); 
      if(robot.getLeftPosition() >= 1500){
        robot.stop();
        delay(1000);
        state = 2;
      }
      break;
    case 2:
    //gerak lurus ke hadapan menuju ke Check point B

      break;
    case 3:
    //Pivot Counter Clock wise

      break;

    case 4:
    //gerak lurus ke hadapan menuju ke STOP

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

// Fungsi non-blocking
float bacaJarak() {
  unsigned long masaSemasa = millis();

  // Hanya jalankan pembacaan jika interval telah sampai
  if (masaSemasa - masaSebelumnya >= interval) {
    masaSebelumnya = masaSemasa;

    // Trigger sensor
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // Baca ECHO dengan timeout 30000us (30ms).
    // Ini mengelakkan kod tersekat lama jika tiada objek di hadapan.
    long duration = pulseIn(echoPin, HIGH, 30000);

    // Kira jarak (jika duration = 0, bermaksud timeout atau tiada pantulan)
    if (duration > 0) {
      jarakTerakhir = duration * 0.034 / 2;
    } else {
      jarakTerakhir = 0; // Atau boleh set ke nilai maksimum
    }
  }

  // Fungsi sentiasa memulangkan nilai jarak terakhir yang diketahui
  return jarakTerakhir;
}