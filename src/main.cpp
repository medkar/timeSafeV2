#include <Arduino.h>
#include <lvgl.h>
#include "hw/lv_port.h"
#include "hw/WiFiProvisioner.h"
#include "hw/HttpsTimeClient.h"
#include <ThemeRegistry.h>
#include "ui/ThemeStyle.h"
#include "secrets.h"
#include <sys/time.h>
#include <cstring>

using namespace tsafe;

static WiFiProvisioner wifi;
static HttpsTimeClient httpsTime("www.google.com");
static lv_obj_t* lblStatus;
static lv_obj_t* lblClock;
static lv_obj_t* lblSrc;
static int64_t   baseEpoch = 0;
static uint32_t  baseMs = 0;
static bool      haveTime = false;

// epoch UTC -> "YYYY-MM-DD  HH:MM:SS UTC" (civil_from_days de Hinnant).
static void fmtEpoch(int64_t epoch, char* out, size_t n) {
    int64_t days = epoch / 86400;
    int secs = (int)(epoch % 86400);
    days += 719468;
    int64_t era = (days >= 0 ? days : days - 146096) / 146097;
    unsigned doe = (unsigned)(days - era * 146097);
    unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    int y = (int)yoe + (int)era * 400;
    unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    unsigned mp = (5 * doy + 2) / 153;
    unsigned d = doy - (153 * mp + 2) / 5 + 1;
    unsigned m = mp + (mp < 10 ? 3 : -9);
    y += (m <= 2);
    snprintf(out, n, "%04d-%02u-%02u  %02d:%02d:%02d UTC",
             y, m, d, secs / 3600, (secs % 3600) / 60, secs % 60);
}

static void tick(lv_timer_t*) {
    if (!haveTime) return;
    int64_t now = baseEpoch + (int64_t)((millis() - baseMs) / 1000);
    char b[40];
    fmtEpoch(now, b, sizeof(b));
    lv_label_set_text(lblClock, b);
}

static int64_t civilToEpoch(int y, unsigned mo, unsigned d, int h, int mi, int s) {
    y -= mo <= 2;
    int64_t era = (y >= 0 ? y : y - 399) / 400;
    unsigned yoe = (unsigned)(y - era * 400);
    unsigned doy = (153 * (mo + (mo > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    int64_t days = era * 146097 + (int64_t)doe - 719468;
    return days * 86400 + (int64_t)h * 3600 + mi * 60 + s;
}

// Plancher temporel = date de compilation. Résout le paradoxe : mbedTLS a besoin
// d'une heure pour valider le certificat épinglé, or c'est justement ce qu'on
// cherche. Le temps réel étant toujours >= date de build, on l'utilise comme base.
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
    setClockFloor();   // base horloge = date de build (pour valider le cert épinglé)

    const Theme& t = themeById(0);
    lv_obj_t* scr = lv_screen_active();
    themeApplyBg(scr, t);

    lblStatus = themeLabel(scr, "Connexion WiFi...", t.palette.muted, 14);
    lv_obj_align(lblStatus, LV_ALIGN_TOP_MID, 0, 26);
    lblClock = themeLabel(scr, "--:--:--", t.palette.accent, 28, 1);
    lv_obj_align(lblClock, LV_ALIGN_CENTER, 0, 0);
    lblSrc = themeLabel(scr, "heure de confiance", t.palette.muted, 14);
    lv_obj_align(lblSrc, LV_ALIGN_BOTTOM_MID, 0, -16);
    lv_timer_handler();   // peindre l'état initial

    if (wifi.connect(WIFI_SSID, WIFI_PASS)) {
        char s[72];
        snprintf(s, sizeof(s), "WiFi OK  -  %s", wifi.ip().c_str());
        lv_label_set_text(lblStatus, s);

        TimeSample ts = httpsTime.read();
        if (ts.present) {
            baseEpoch = ts.epoch;
            baseMs = millis();
            haveTime = true;
            struct timeval tv = { (time_t)ts.epoch, 0 };
            settimeofday(&tv, nullptr);   // cale l'horloge sur l'heure validée
            lv_label_set_text(lblSrc, "source : www.google.com  (HTTPS epingle GTS Root R1)");
            Serial.printf("epoch=%lld (epingle)\n", (long long)ts.epoch);
        } else {
            lv_label_set_text(lblClock, "echec heure (cert refuse ?)");
        }
    } else {
        lv_label_set_text(lblStatus, "WiFi : echec (verifie src/secrets.h)");
    }

    lv_timer_create(tick, 1000, nullptr);
    Serial.println("Test WiFi + heure pret");
}

void loop() {
    lvport_loop();
    delay(5);
}
