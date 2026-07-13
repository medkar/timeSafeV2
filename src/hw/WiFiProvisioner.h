#pragma once
#include <string>
#include <cstdint>

namespace tsafe {

class WiFiProvisioner {
public:
    bool connect(const char* ssid, const char* pass, uint32_t timeoutMs = 15000);
    bool isConnected();
    std::string ip();
};

} // namespace tsafe
