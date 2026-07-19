#include <Arduino.h>
#include <ESP32Servo.h>
#include "board_config.h"

// ---------------------------------------------------------------------------
//  Calibration du servo — met le servo à UNE position, puis ne fait plus rien.
//
//  Mode d'emploi :
//    1. Modifier ANGLE ci-dessous.
//    2. Reflasher :  python -m platformio run -e servocal -t upload
//    3. Observer la mécanique du pêne, puis recommencer.
//
//  Quand les deux bons angles sont trouvés, les reporter dans
//  include/board_config.h  ->  TS_SERVO_ANGLE_LOCKED / TS_SERVO_ANGLE_UNLOCKED
//
//  Les paramètres d'attache sont IDENTIQUES à ceux de ServoLock (50 Hz,
//  impulsions 500-2400 us) : l'angle trouvé ici se transpose donc tel quel
//  au firmware, sans décalage.
// ---------------------------------------------------------------------------

static const int ANGLE = 0;   // <<< SEULE VALEUR À CHANGER (0-180)

static Servo servo;

void setup() {
    Serial.begin(115200);
    delay(300);

    // Mêmes timers LEDC que ServoLock (les autres restent au rétroéclairage).
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    servo.setPeriodHertz(50);
    servo.attach(TS_SERVO_PIN, 500, 2400);

    servo.write(ANGLE);
    Serial.printf("[servo_calib] pin=%d  angle=%d\n", TS_SERVO_PIN, ANGLE);
}

void loop() {
    delay(1000);   // le servo tient sa position, rien d'autre à faire
}
