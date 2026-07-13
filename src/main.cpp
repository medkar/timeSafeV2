#include <Arduino.h>

void setup() {
    Serial.begin(115200);
    delay(300);
    Serial.println("TimeSafe boot OK");
}

void loop() {
    static uint32_t n = 0;
    Serial.printf("alive %lu\n", (unsigned long)n++);
    delay(2000);
}
