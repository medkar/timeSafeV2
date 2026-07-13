#include <Arduino.h>
#include "hw/DisplayLGFX.h"

static DisplayLGFX gfx;

void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println("TimeSafe display test");

    gfx.init();
    gfx.setRotation(1);        // paysage 480x320
    gfx.setBrightness(200);

    const uint16_t cyan  = gfx.color565(56, 189, 248);
    const uint16_t white = gfx.color565(230, 232, 236);

    gfx.fillScreen(0x0000);
    gfx.setTextColor(cyan);
    gfx.setTextSize(4);
    gfx.setCursor(30, 50);
    gfx.print("TimeSafe");

    gfx.setTextColor(white);
    gfx.setTextSize(2);
    gfx.setCursor(30, 120);
    gfx.print("Ecran OK - touche l'ecran");

    // Barres R / G / B pour verifier l'ordre des couleurs (doivent etre
    // ROUGE, VERT, BLEU de gauche a droite).
    gfx.fillRect(0,   210, 160, 70, gfx.color565(255, 0, 0));
    gfx.fillRect(160, 210, 160, 70, gfx.color565(0, 255, 0));
    gfx.fillRect(320, 210, 160, 70, gfx.color565(0, 0, 255));

    Serial.println("Display init done");
}

void loop() {
    int32_t x, y;
    if (gfx.getTouch(&x, &y)) {
        gfx.fillCircle(x, y, 4, gfx.color565(255, 226, 77)); // point jaune
        Serial.printf("touch x=%d y=%d\n", (int)x, (int)y);
    }
    delay(10);
}
