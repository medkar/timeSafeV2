# TimeSafe

*A sealed box that opens only at a date you choose — a physical time capsule you
cannot open early — that doubles as an ordinary password/PIN safe.*

![platform](https://img.shields.io/badge/platform-ESP32-blue)
![tests](https://img.shields.io/badge/native%20tests-69%20passing-brightgreen)
![license](https://img.shields.io/badge/license-MIT-green)

🇫🇷 [Version française](README.fr.md)

<!-- DEMO: add assets/demo.gif (countdown → unlock) here once the enclosure is assembled -->

## What it is

TimeSafe is a 3D-printed, sealed ESP32 box with a touchscreen. Set a target
date and it becomes a **time capsule**: the lock will not open until that moment
— and with no battery-backed way in, it genuinely can't be opened early. Set a
password or PIN instead (or as well) and it's a **normal safe**. It has no
recovery path by design.

## Features

- **Time lock** with two independent trusted-time sources (pinned HTTPS + DS3231 RTC)
- **Password or 6-digit PIN** safe, with exponential lockout on wrong attempts
- **5 occasion themes** (gift, memory, party, kids, safe), remembered across reboots
- **On-screen WiFi setup** — configure the network without a computer
- **Survives power loss** — capsule date, hash and theme persist in NVS
- Runs on a **pure, host-testable core**: 69 unit tests, no hardware needed

## How it works

**Architecture.** A pure logic core (`lib/`) sits behind interfaces; hardware
adapters (`src/hw`) and the LVGL UI (`src/ui`) plug into it. The core has zero
Arduino dependencies, so it runs under 69 native unit tests on a PC. Full
diagram: [docs/architecture.md](docs/architecture.md).

**Trusted time (anti-cheat).** The box reads time from a **pinned HTTPS**
response (seeding the system clock) and a **DS3231 RTC**. It opens if *either*
plausible source has reached the target date (OR / maximum rule), and stays
**locked** if neither is trustworthy (fail-closed).

**Security.** Passwords are stored as a **salted PBKDF2-HMAC-SHA256** hash,
never plaintext. Wrong attempts trigger an **exponential lockout** persisted to
NVS. There is no backdoor.

<!-- HARDWARE: add BOM + wiring (or link to hardware/BUILD.md) once the box is built -->

## Build & run

Requires [PlatformIO](https://platformio.org/).

```bash
# Host unit tests (no hardware needed)
pio test -e native

# Configure WiFi, then build & flash the firmware
cp src/secrets.h.example src/secrets.h   # then edit with your 2.4 GHz network
pio run -e esp32dev -t upload
```

Adapt the wiring to your board by editing [`include/board_config.h`](include/board_config.h).

## Disclaimer

This is a hobby project, not a certified security product — **treat it as a fun
time capsule, not a real safe.** Because the time lock opens when *either* clock
source reaches the date, a source that glitches forward could open it early. No
recovery path means a forgotten password means a broken box.

## License

Code under the [MIT License](LICENSE).

## Credits

Built with [LVGL](https://lvgl.io/), [LovyanGFX](https://github.com/lovyan03/LovyanGFX),
[RTClib](https://github.com/adafruit/RTClib) and [ESP32Servo](https://github.com/madhephaestus/ESP32Servo).
