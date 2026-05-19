#ifndef DeadReckoning_h
#define DeadReckoning_h

#include <Arduino.h>
#include <Wire.h>
#include "TwoWDController.h" 

// HMC5883L I2C Address and Registers
#define HMC5883L_ADDRESS 0x1E
//#define HMC5883L_ADDRESS 0x0D
#define HMC5883L_REG_CONFIG_A 0x00
#define HMC5883L_REG_CONFIG_B 0x01
#define HMC5883L_REG_MODE 0x02
#define HMC5883L_REG_DATA 0x03

class DeadReckoning {
  public:
    // Constructor requiring only the robot controller
    DeadReckoning(TwoWDController* robot);

    // I2C and magnetometer initialization
    void initCompass();
    
    // Main navigation method
    void move(float targetHeading, long targetTicks);

    // Public method to read the compass for debugging or telemetry
    float getHeading(); 

  private:
    TwoWDController* _robot;
};

#endif