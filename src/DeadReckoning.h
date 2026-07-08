#ifndef DeadReckoning_h
#define DeadReckoning_h

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_QMC5883P.h> 
#include "TwoWDController.h" 

class DeadReckoning {
  public:
    DeadReckoning(TwoWDController* robot, uint8_t compassAddress);

    void initCompass();
    float getHeading(); 
    
    void startMove(float targetHeading, long targetTicks);
    void update(); 
    bool isMoving();

  private:
    TwoWDController* _robot;
    uint8_t _compassAddress;
    
    Adafruit_QMC5883P _compass; 

    bool _isMoving;
    float _targetHeading;
    long _targetTicks;

    float _Kp, _Ki, _Kd;
    float _baseSpeed;
    float _previousError;
    float _integral;

    unsigned long _lastCompassRead;
    const unsigned long _compassInterval = 70;

    // --- VARIABEL BARU UNTUK LOW-PASS FILTER (EMA) ---
    float _filteredX;
    float _filteredY;
    float _alpha;
    float _lastHeading; // Menyimpan heading terakhir jika pembacaan I2C gagal
};

#endif
