#include <Arduino.h>
#include <lvgl.h>
#include <sys/time.h>
#include <time.h>
#include <cstring>
#include <cstdio>
#include <Wire.h>

#include "hw/lv_port.h"
#include "hw/ServoLock.h"
#include "hw/WiFiProvisioner.h"
#include "hw/HttpsTimeClient.h"
#include "hw/SystemClock.h"
#include "hw/RtcClock.h"
#include "hw/StubHasher.h"
#include "hw/NvsStore.h"
#include "hw/MonotonicClock.h"
#include "ui/LvglUiView.h"
#include "ui/ThemeStyle.h"
#include "ui/Views.h"
#include <ThemeRegistry.h>
#include <AppStateMachine.h>
#include "secrets.h"

using namespace tsafe;

static WiFiProvisioner  wifi;
static HttpsTimeClient  httpsBoot("www.google.com");
static SystemClock      sysClock;
static RtcClock         rtcClock;    // DS3231 (HW-111) sur Wire1 (GPIO16/17)
static ServoLock        servo(13, 0, 90);
static MonotonicClock   mono;
static StubHasher       hasher;
static NvsStore         store;       // persistance flash (survit à la coupure)
static LvglUiView       ui(0);       // thème Coffre
static AppStateMachine* app = nullptr;

static int64_t civilToEpoch(int y, unsigned mo, unsigned d, int h, int mi, int s) {
    y -= mo <= 2;
    int64_t era = (y >= 0 ? y : y - 399) / 400;
    unsigned yoe = (unsigned)(y - era * 400);
    unsigned doy = (153 * (mo + (mo > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    int64_t days = era * 146097 + (int64_t)doe - 719468;
    return days * 86400 + (int64_t)h * 3600 + mi * 60 + s;
}

// Plancher horloge = date de build (pour valider le certificat épinglé).
static void setClockFloor() {
    char mon[4] = {0};
    int d = 1, y = 2026, h = 0, mi = 0, s = 0;
    sscanf(__DATE__, "%3s %d %d", mon, &d, &y);
    sscanf(__TIME__, "%d:%d:%d", &h, &mi, &s);
    static const char* M = "JanFebMarAprMayJunJulAugSepOctNovDec";
    const char* p = strstr(M, mon);
    unsigned mo = p ? (unsigned)((p - M) / 3 + 1) : 1;
    int64_t be = civilToEpoch(y, mo, (unsigned)d, h, mi, s);
    struct timeval nowv;
    gettimeofday(&nowv, nullptr);
    if ((int64_t)nowv.tv_sec < be) {
        struct timeval tv = { (time_t)be, 0 };
        settimeofday(&tv, nullptr);
    }
}

void setup() {
    Serial.begin(115200);
    lvport_init();
    setClockFloor();

    // Écran de synchro pendant la connexion.
    viewSync(lv_screen_active(), themeById(0));
    lv_timer_handler();

    servo.begin();                          // attache + verrouille (fail-closed)
    bool rtcOk = rtcClock.begin(32, 19);    // RTC sur bus I²C dédié (Wire1) : SDA=32, SCL=19

    Serial.print("I2C scan Wire1:");        // diagnostic
    for (uint8_t a = 1; a < 127; ++a) {
        Wire1.beginTransmission(a);
        if (Wire1.endTransmission() == 0) Serial.printf(" 0x%02X", a);
    }
    Serial.println();

    wifi.connect(WIFI_SSID, WIFI_PASS);

    TimeSample ts = httpsBoot.read();        // heure de confiance épinglée
    if (ts.present) {
        struct timeval tv = { (time_t)ts.epoch, 0 };
        settimeofday(&tv, nullptr);          // cale l'horloge système
        sysClock.synced = true;
        rtcClock.setUtc(ts.epoch);           // synchronise le RTC depuis HTTPS
    }
    Serial.printf("RTC detecte=%d osf=%d\n", (int)rtcOk, (int)rtcClock.lostPower());
    {
        TimeSample r = rtcClock.read();
        Serial.printf("RTC read present=%d epoch=%lld\n",
                      (int)r.present, (long long)r.epoch);
    }

    // Ouvre la NVS et diagnostique ce qui a survécu à la dernière coupure.
    bool nvsOk = store.begin();
    {
        StoredConfig probe;
        bool had = store.load(probe);
        Serial.printf("NVS begin=%d had=%d armed=%d hasDate=%d hasPwd=%d openDate=%lld\n",
                      (int)nvsOk, (int)had, (int)probe.box.armed,
                      (int)probe.box.hasDate, (int)probe.box.hasPassword,
                      (long long)probe.box.openDate);
    }

    // La config persistée (si elle existe) est rechargée par begin() -> la boîte
    // reprend sa capsule ; sinon elle démarre sur l'accueil (configuration).
    app = new AppStateMachine(store, sysClock, rtcClock, servo, mono, hasher, ui);
    app->begin();
    Serial.printf("Demarrage. now=%lld\n", (long long)time(nullptr));
}

void loop() {
    static uint32_t last = 0;
    if (millis() - last >= 500) {   // évaluer la politique 2x/s
        last = millis();
        if (app) app->tick();
    }
    lvport_loop();
    delay(5);
}
