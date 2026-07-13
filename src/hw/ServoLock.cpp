#include "ServoLock.h"
#include <Arduino.h>

namespace tsafe {

ServoLock::ServoLock(int pin, int lockedAngle, int unlockedAngle)
    : pin_(pin), locked_(lockedAngle), unlocked_(unlockedAngle), angle_(lockedAngle) {}

void ServoLock::begin() {
    // Deux timers LEDC pour le servo (les autres restent au rétroéclairage LCD).
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    servo_.setPeriodHertz(50);
    servo_.attach(pin_, 500, 2400);
    forceLock();
}

void ServoLock::writeAngle(int a) {
    angle_ = a;
    servo_.write(a);
    delay(500);  // laisser le temps au servo d'atteindre la position
}

void ServoLock::forceLock() { writeAngle(locked_); }
void ServoLock::unlock()    { writeAngle(unlocked_); }

} // namespace tsafe
