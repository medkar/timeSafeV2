#include <Arduino.h>
#include <ESP32Servo.h>
#include "board_config.h"

// ---------------------------------------------------------------------------
//  Calibration du servo — alterne entre DEUX positions, indéfiniment.
//
//  Mode d'emploi :
//    1. Régler ANGLE_A / ANGLE_B (et HOLD_MS si besoin).
//    2. Reflasher :  python -m platformio run -e servocal -t upload
//    3. Observer la mécanique du pêne sur plusieurs cycles.
//
//  Astuce : mettre ANGLE_A == ANGLE_B fige le servo sur une seule position
//  (utile pour tester un angle isolé).
//
//  Quand les deux bons angles sont trouvés, les reporter dans
//  include/board_config.h  ->  TS_SERVO_ANGLE_LOCKED / TS_SERVO_ANGLE_UNLOCKED
//
//  Les paramètres d'attache sont IDENTIQUES à ceux de ServoLock (50 Hz,
//  impulsions 500-2400 us) : les angles trouvés ici se transposent tels quels
//  au firmware, sans décalage.
// ---------------------------------------------------------------------------

static const int      ANGLE_A = 180;     // 1re position
static const int      ANGLE_B = 50;      // 2e position
static const uint32_t HOLD_MS = 10000;   // maintien de chaque position (ms)

static Servo servo;

static void goTo(int angle) {
    servo.write(angle);
    Serial.printf("[servo_calib] angle=%d\n", angle);
    delay(HOLD_MS);
}

void setup() {
    Serial.begin(115200);
    delay(300);

    // Mêmes timers LEDC que ServoLock (les autres restent au rétroéclairage).
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    servo.setPeriodHertz(50);
    servo.attach(TS_SERVO_PIN, 500, 2400);

    Serial.printf("[servo_calib] pin=%d  cycle %d <-> %d  (%lu ms par position)\n",
                  TS_SERVO_PIN, ANGLE_A, ANGLE_B, (unsigned long)HOLD_MS);
}

void loop() {
    goTo(ANGLE_A);
    goTo(ANGLE_B);
}
