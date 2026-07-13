#include "HttpsTimeClient.h"
#include <WiFiClientSecure.h>
#include <Arduino.h>
#include <cstring>

namespace tsafe {

// epoch UTC à partir d'une date civile (algorithme de Howard Hinnant).
static int64_t toEpochUTC(int y, unsigned mo, unsigned d, int h, int mi, int s) {
    y -= mo <= 2;
    int64_t era = (y >= 0 ? y : y - 399) / 400;
    unsigned yoe = (unsigned)(y - era * 400);
    unsigned doy = (153 * (mo + (mo > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    int64_t days = era * 146097 + (int64_t)doe - 719468;
    return days * 86400 + (int64_t)h * 3600 + mi * 60 + s;
}

// Parse "Sun, 06 Nov 1994 08:49:37 GMT" (partie après "Date:").
static int64_t parseHttpDate(const String& v) {
    char mon[4] = {0};
    int d, y, h, mi, s;
    if (sscanf(v.c_str(), " %*3s, %d %3s %d %d:%d:%d", &d, mon, &y, &h, &mi, &s) != 6)
        return 0;
    static const char* M = "JanFebMarAprMayJunJulAugSepOctNovDec";
    const char* p = strstr(M, mon);
    if (!p) return 0;
    unsigned mo = (unsigned)((p - M) / 3 + 1);
    return toEpochUTC(y, mo, (unsigned)d, h, mi, s);
}

HttpsTimeClient::HttpsTimeClient(const char* host) : host_(host) {}

TimeSample HttpsTimeClient::read() {
    TimeSample out;
    WiFiClientSecure client;
    client.setInsecure();     // TEMPORAIRE : non épinglé (étape 1)
    client.setTimeout(10);

    if (!client.connect(host_, 443)) return out;
    client.printf("HEAD / HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", host_);

    int64_t epoch = 0;
    uint32_t start = millis();
    while (client.connected() && (millis() - start) < 10000) {
        String line = client.readStringUntil('\n');
        if (line.startsWith("Date:") || line.startsWith("date:")) {
            epoch = parseHttpDate(line.substring(5));
        }
        if (line == "\r" || line.length() == 0) break;  // fin des en-têtes
    }
    client.stop();

    if (epoch > 0) { out.present = true; out.epoch = epoch; }
    return out;
}

} // namespace tsafe
