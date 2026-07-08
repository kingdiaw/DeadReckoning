#include "DeadReckoning.h"

DeadReckoning::DeadReckoning(TwoWDController* robot, uint8_t compassAddress) {
    _robot = robot;
    _compassAddress = compassAddress;
    _isMoving = false;

    _Kp = 0.003;
    _Ki = 0.0;
    _Kd = 0.0;
    _baseSpeed = 0.15;

    // Inisialisasi nilai awal penapis (filter)
    _filteredX = 0.0;
    _filteredY = 0.0;
    _alpha = 0.2;       // Nilai alpha sesuai contoh Anda (0.0 - 1.0)
    _lastHeading = 0.0;
}

void DeadReckoning::initCompass() {
    if (!_compass.begin(_compassAddress, &Wire)) {
        Serial.println("Gagal mengesan QMC5883! Semak pendawaian.");
    } else {
        // 1. Tukar kepada CONTINUOUS mode
        _compass.setMode(QMC5883P_MODE_CONTINUOUS);

        // 2. ODR tetapkan ke 50Hz (sesuai dengan sela masa 70ms Anda)
        _compass.setODR(QMC5883P_ODR_50HZ); 

        // 3. Set OSR ke 8 untuk maksimumkan oversampling (mengurangkan noise dalaman)
        _compass.setOSR(QMC5883P_OSR_8);

        // 4. Tetapkan DSR, Range, dan SetResetMode sesuai contoh baru
        _compass.setDSR(QMC5883P_DSR_2);
        _compass.setRange(QMC5883P_RANGE_8G);
        _compass.setSetResetMode(QMC5883P_SETRESET_ON);
        
        Serial.println("QMC5883P Berjaya Dikonfigurasi dengan Filter!");
    }
}

float DeadReckoning::getHeading() {
    float x, y, z;
    
    if (_compass.getGaussField(&x, &y, &z)) {
        
        // --- MASUKKAN NILAI KALIBRASI ANDA DI SINI ---
        float offsetX = 0.244; // Ganti dengan Offset X dari Serial Monitor
        float offsetY = 0.143; // Ganti dengan Offset Y dari Serial Monitor
        float scaleX  = 1.074; // Ganti dengan Scale X dari Serial Monitor
        float scaleY  = 0.935; // Ganti dengan Scale Y dari Serial Monitor

        // Terapkan pembetulan Hard Iron dan Soft Iron
        float calX = (x - offsetX) * scaleX;
        float calY = (y - offsetY) * scaleY;
        
        // --- PENAPIS (LOW-PASS FILTER) ---
        // Guna calX dan calY, bukan x dan y yang mentah
        if (_filteredX == 0.0 && _filteredY == 0.0) {
            _filteredX = calX;
            _filteredY = calY;
        } else {
            _filteredX = (_alpha * calX) + ((1.0 - _alpha) * _filteredX);
            _filteredY = (_alpha * calY) + ((1.0 - _alpha) * _filteredY);
        }
        
        // Kira sudut 
        float headingRadian = atan2(_filteredY, _filteredX);
        float headingDarjah = headingRadian * (180.0 / PI);
        
        if (headingDarjah < 0) {
            headingDarjah += 360.0;
        }
        
        _lastHeading = headingDarjah;
        return headingDarjah;
    }
    
    return _lastHeading; 
}

void DeadReckoning::startMove(float targetHeading, long targetTicks) {
    _targetHeading = targetHeading;
    _targetTicks = targetTicks;
    
    _robot->resetEncoders(); 
    
    _previousError = 0.0;
    _integral = 0.0;
    _lastCompassRead = millis();
    
    _isMoving = true;
}

void DeadReckoning::update() {
    if (!_isMoving) return;

    long currentTicks = (_robot->getLeftPosition() + _robot->getRightPosition()) / 2.0;

    if (currentTicks >= _targetTicks) {
        _robot->stop();
        _isMoving = false;
        return;
    }

    if (millis() - _lastCompassRead >= _compassInterval) {
        _lastCompassRead = millis(); 

        float currentHeading = getHeading();
        float error = _targetHeading - currentHeading;

        if (error > 180.0) error -= 360.0;
        if (error < -180.0) error += 360.0;

        _integral += error;
        float derivative = error - _previousError;
        
        float correction = (_Kp * error) + (_Ki * _integral) + (_Kd * derivative);
        _previousError = error;

        float leftSpeed = _baseSpeed - correction;
        float rightSpeed = _baseSpeed + correction;

        _robot->drive(leftSpeed, rightSpeed);
    }
}

bool DeadReckoning::isMoving() {
    return _isMoving;
}
