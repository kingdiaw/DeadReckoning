# DeadReckoning

An Arduino library for 2WD robots that implements **dead reckoning navigation** using wheel encoders and an HMC5883L 3-axis digital compass. 

By combining distance tracking (via encoder ticks) with dynamic heading correction (via magnetometer), this library uses a PID controller to significantly minimize the cumulative drift commonly seen in differential drive robots.

## Features
* **Built-in PID Controller:** Smoothly corrects motor speeds to maintain a straight heading.
* **Lightweight Compass Integration:** Bypasses bulky external libraries by writing hex commands directly to the HMC5883L I2C registers.
* **Non-Blocking Reads:** Uses a 70ms timer to match the compass's 15Hz data rate without slowing down your main motor control loop.

## Hardware Requirements
1. **2WD Robot Chassis** with DC motors and wheel encoders.
2. **QMC5883P Magnetometer** (GY-273 / GY-85 or similar breakout board).
3. **Motor Driver** compatible with your existing `TwoWDController` library.

### Magnetometer Wiring (I2C)
| HMC5883L Pin | Arduino Pin |
| :--- | :--- |
| VCC | 3.3V or 5V (Check your specific module) |
| GND | GND |
| SDA | SDA (D21 on ESP32) |
| SCL | SCL (D22 on ESP32) |

> **⚠️ Crucial Mounting Tip:** DC motors contain strong magnets and generate electromagnetic fields. Mount the HMC5883L **as far away from the motors as possible** and ensure it is perfectly flat (parallel to the ground).

## Installation
1. Download this repository as a `.zip` file.
2. Open the Arduino IDE.
3. Go to **Sketch > Include Library > Add .ZIP Library...** and select the downloaded file.
4. Make sure your custom `TwoWDController` library is also installed.

## Kaedah Umum (Public Methods)
### 1. void initCompass()
Memulakan sambungan I2C dengan sensor kompas dan menetapkan konfigurasi optimum (Continuous Mode, 50Hz ODR, 8 OSR).
* Penggunaan: Mesti dipanggil sekali di dalam fungsi setup().
* Contoh:
```
nav.initCompass();
```
### 2. float getHeading()
Membaca dan memproses data dari kompas magnetik. Ia menggunakan penentukuran Hard-Iron dan Soft-Iron, serta menapis hingar (noise) menggunakan Penapis Rendah (Low-Pass Filter - EMA).
* Pulangan (Returns): Nilai float yang mewakili sudut heading semasa dalam unit darjah (0.0° hingga 359.9°).
* Contoh:
```
float arahSemasa = nav.getHeading();
Serial.println(arahSemasa);
```
### 3. void startMove(float targetHeading, long targetTicks)
Memberi arahan kepada robot untuk mula bergerak dalam garis lurus mengikut sudut sasaran sehingga mencapai jarak yang ditetapkan. Ia juga akan menetapkan semula (reset) nilai enkoder motor dan pembolehubah kawalan PID.
* Parameter:
    * targetHeading (float): Sudut arah yang ingin dituju (0 - 360 darjah).
    * targetTicks (long): Jarak sasaran yang diukur berdasarkan jumlah pergerakan (ticks) dari enkoder tayar.
* Contoh:
```
// Bergerak ke arah 180 darjah (Selatan) sejauh 3000 ticks
nav.startMove(180.0, 3000);
```
### 4. void update()
Nadi utama kepada sistem navigasi PID. Ia membaca heading semasa, mengira ralat (perbezaan antara arah semasa dan targetHeading), dan menyelaras kelajuan motor kiri dan kanan (melalui fungsi drive() pada TwoWDController). Ia juga menghentikan robot jika sasaran targetTicks telah dicapai.
* Penggunaan: **Mesti** dipanggil secara berterusan (tanpa halangan/delay) di dalam fungsi loop().
* Contoh:
```
void loop() {
    nav.update();
}
```
### 5. bool isMoving()
Menyemak status pergerakan robot sama ada ia masih dalam perjalanan ke sasaran atau telah berhenti.
* Pulangan (Returns):
    * true: Robot masih bergerak.
    * false: Robot telah sampai ke sasaran (targetTicks dicapai) dan telah berhenti.
* Contoh:
```
if (!nav.isMoving()) {
    Serial.println("Robot berjaya sampai ke destinasi!");
}
```
## Aliran Kerja Standard (Workflow)
Untuk menggunakan API ini dengan betul, ikut susunan aliran kerja ini:
1. **Global:** Cipta objek TwoWDController dan DeadReckoning.
2. **setup():** Panggil Wire.begin() dan nav.initCompass().
3. **Tentukan Sasaran:** Panggil nav.startMove(arah, jarak) apabila anda mahu robot mula bergerak.
4. **loop():** Sentiasa panggil nav.update() agar sistem PID dapat memperbetulkan arah tayar secara real-time. Anda boleh memantau status menggunakan
   ```
   nav.isMoving().
   ```


