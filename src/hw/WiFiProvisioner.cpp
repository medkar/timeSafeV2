#include "WiFiProvisioner.h"
#include <WiFi.h>

namespace tsafe {

bool WiFiProvisioner::connect(const char* ssid, const char* pass, uint32_t timeoutMs) {
    if (!ssid || !ssid[0]) return false;
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - start) < timeoutMs) {
        delay(200);
    }
    return WiFi.status() == WL_CONNECTED;
}

bool WiFiProvisioner::isConnected() { return WiFi.status() == WL_CONNECTED; }

std::string WiFiProvisioner::ip() {
    return std::string(WiFi.localIP().toString().c_str());
}

} // namespace tsafe
