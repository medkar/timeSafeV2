#pragma once
#include <ILock.h>
#include <ESP32Servo.h>

namespace tsafe {

// Verrou à servo, fail-closed : verrouillé par défaut. Le servo ne fait que
// RETIRER le pêne pour ouvrir ; la mécanique doit tenir verrouillé sans courant.
class ServoLock : public ILock {
public:
    ServoLock(int pin, int lockedAngle, int unlockedAngle);
    void begin();                 // attache le servo + force le verrou
    void forceLock() override;
    void unlock() override;
    int currentAngle() const { return angle_; }

private:
    void writeAngle(int a);
    int pin_, locked_, unlocked_, angle_;
    Servo servo_;
};

} // namespace tsafe
