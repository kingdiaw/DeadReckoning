# DeadReckoning

An Arduino library for 2WD robots that implements **dead reckoning navigation** using wheel encoders and an HMC5883L 3-axis digital compass. 

By combining distance tracking (via encoder ticks) with dynamic heading correction (via magnetometer), this library uses a PID controller to significantly minimize the cumulative drift commonly seen in differential drive robots.

## Features
* **Built-in PID Controller:** Smoothly corrects motor speeds to maintain a straight heading.
* **Lightweight Compass Integration:** Bypasses bulky external libraries by writing hex commands directly to the HMC5883L I2C registers.
* **Non-Blocking Reads:** Uses a 70ms timer to match the compass's 15Hz data rate without slowing down your main motor control loop.

## Hardware Requirements
1. **2WD Robot Chassis** with DC motors and wheel encoders.
2. **HMC5883L Magnetometer** (GY-273 / GY-85 or similar breakout board).
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

## Setup: Magnetic Declination (Mandatory)
The compass measures *Magnetic North*, but navigation usually relies on *True North*. The difference between these two is called **Magnetic Declination**, and it changes depending on your geographical location.

To ensure your robot drives accurately, you must update the declination angle in the library:
1. Visit [Magnetic-Declination.com](http://www.magnetic-declination.com/).
2. Find your city (e.g., Kuala Lumpur).
3. Convert the given angle to decimal degrees, and then to radians.
   * *Example: -0° 13' -> -0.216 degrees -> **-0.0038 rad***
4. Open `src/DeadReckoning.cpp` and update this line in the `getHeading()` method:
   ```cpp
   float declinationAngle = -0.0038; // Update this for your location!

```

## Quick Start Example

```cpp
#include <Wire.h>
#include <TwoWDController.h>
#include <DeadReckoning.h>

// Initialize your robot controller with your specific hardware pins
TwoWDController robot(0x20, 4, 2, P2, P3, P0, P1, 18, 19, 25, 13);
DeadReckoning navigator(&robot);

void setup() {
  Serial.begin(115200);
  
  // Initialize the I2C bus and the HMC5883L
  navigator.initCompass();
  delay(500); // Give the compass time to stabilize
}

void loop() {
  // Move to a heading of 90.0 degrees (East) for 1000 encoder ticks
  navigator.move(90.0, 1000);
  
  while(true) { delay(10); } // Stop forever
}

```

## Author

**kingdiaw** - [kingdiawehsut@gmail.com](mailto:kingdiawehsut@gmail.com)

```
