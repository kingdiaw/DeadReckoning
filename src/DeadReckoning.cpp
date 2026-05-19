#include "DeadReckoning.h"

DeadReckoning::DeadReckoning(TwoWDController* robot) {
    _robot = robot;
}

void DeadReckoning::initCompass() {
    Wire.begin(); // Start the I2C bus

    // 1. Configure Register A: 1 Sample, 15Hz
    Wire.beginTransmission(HMC5883L_ADDRESS);
    Wire.write(HMC5883L_REG_CONFIG_A);
    Wire.write(0x10); 
    Wire.endTransmission();

    // 2. Configure Register B: Range 1.3 Ga
    Wire.beginTransmission(HMC5883L_ADDRESS);
    Wire.write(HMC5883L_REG_CONFIG_B);
    Wire.write(0x20); 
    Wire.endTransmission();

    // 3. Configure Mode: Continuous Measurement
    Wire.beginTransmission(HMC5883L_ADDRESS);
    Wire.write(HMC5883L_REG_MODE);
    Wire.write(0x00); 
    Wire.endTransmission();
}

float DeadReckoning::getHeading() {
    // Tell the compass we want to read starting from the first data register
    Wire.beginTransmission(HMC5883L_ADDRESS);
    Wire.write(HMC5883L_REG_DATA); 
    Wire.endTransmission();

    // Request the 6 bytes of data (X, Z, Y MSB and LSB)
    Wire.requestFrom(HMC5883L_ADDRESS, 6);

    if (Wire.available() >= 6) {
        // Read the registers. Note the order: X, Z, Y!
        int16_t x = (Wire.read() << 8) | Wire.read();
        int16_t z = (Wire.read() << 8) | Wire.read();
        int16_t y = (Wire.read() << 8) | Wire.read();

        // Apply the 1.3 Ga scale factor (0.92 mG/LSb)
        float normX = x * 0.92;
        float normY = y * 0.92;

        // Calculate heading
        float heading = atan2(normY, normX);

        // Apply Magnetic Declination (Kuala Lumpur = -0.0038 rad)
        float declinationAngle = -0.0038; 
        heading += declinationAngle;

        // Correct for reversed signs
        if (heading < 0) {
            heading += 2 * PI;
        }
        // Correct for addition of declination
        if (heading > 2 * PI) {
            heading -= 2 * PI;
        }

        return heading * 180.0 / PI; // Return in degrees
    }

    return 0.0; // Fallback if I2C fails
}

void DeadReckoning::move(float targetHeading, long targetTicks) {
    _robot->resetEncoders(); 
    long currentTicks = 0;
    
    // Tuning Parameters
    float Kp = 0.003;
    float Ki = 0.0;
    float Kd = 0.0;
    float baseSpeed = 0.15; // Speed in m/s

    float previousError = 0.0;
    float integral = 0.0;
    float error = 0.0;      
    float correction = 0.0; 

    // Timer variables for the compass
    unsigned long lastCompassRead = 0;
    const unsigned long compassInterval = 70; // 70ms interval for 15Hz

    while (currentTicks < targetTicks) {
        // Only read the compass and calculate PID if 70ms have passed
        if (millis() - lastCompassRead >= compassInterval) {
            lastCompassRead = millis(); // Reset timer

            float currentHeading = getHeading();

            // Calculate heading error
            error = targetHeading - currentHeading;

            // Normalize the error to the shortest path (-180 to +180 degrees)
            while (error > 180.0) error -= 360.0;
            while (error < -180.0) error += 360.0;

            // Calculate PID terms
            integral += error;
            float derivative = error - previousError;
            
            // Calculate total correction
            correction = (Kp * error) + (Ki * integral) + (Kd * derivative);
            previousError = error;
        }

        // Apply correction to base speeds
        float leftSpeed = baseSpeed + correction;
        float rightSpeed = baseSpeed - correction;

        // Command the motors in m/s
        _robot->setMotorSpeeds(leftSpeed, rightSpeed);

        // Update loop variables
        currentTicks = _robot->getAverageTicks();
        
        // Small delay to prevent the loop from hard-locking the CPU
        delay(5); 
    }

    // Target distance reached
    _robot->stop();
}