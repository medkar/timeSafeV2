# Architecture

TimeSafe splits cleanly into three layers. The **core is pure and host-testable**
(no Arduino, no hardware); adapters implement hardware behind interfaces; the UI
renders themed screens. This is what makes 69 unit tests run on a PC with no board.

```
        +-------------------------------------------------------------+
        |                        UI  (src/ui)                         |
        |   LVGL views, 5 occasion themes, AZERTY keyboard, PIN pad   |
        +-------------------------------------------------------------+
                              |  IUiView (interface)
        +-------------------------------------------------------------+
        |                  Core — pure & testable                     |
        |   lib/app     AppStateMachine (orchestration)               |
        |   lib/domain  TimeCore (trusted time), LockPolicy, Backoff  |
        |   lib/config  StoredConfig codec                            |
        |   lib/theme   theme registry                                |
        |   lib/hal     interfaces: IClock, ILock, IStore, IHasher …  |
        +-------------------------------------------------------------+
                              |  interfaces implemented by
        +-------------------------------------------------------------+
        |                 Adapters  (src/hw)                          |
        |   DisplayLGFX (ST7796+FT6336U)  ServoLock  RtcClock(DS3231) |
        |   HttpsTimeClient (pinned)  SystemClock  NvsStore  Hasher   |
        +-------------------------------------------------------------+
                              |  pins & params come from
                        include/board_config.h
```

## Data flow (every ~500 ms)

1. `AppStateMachine::tick()` polls one UI event (password/arm/rearm/theme).
2. It builds a `PolicyInput`: resolved time + stored config + failure state.
3. `resolveTime()` cross-checks the two clock sources (below).
4. `LockPolicy::decide()` returns a `PolicyState` (Setup / WaitingSync /
   Countdown / AskPassword / Unlock).
5. The state is rendered through `IUiView`; unlock drives `ILock`.

## Trusted time (anti-cheat)

Two independent sources: an HTTPS response from a **pinned** certificate
(seeds the system clock) and a **DS3231 RTC** that keeps time across power loss.

`resolveTime()` uses an **OR / maximum rule**: the box opens if **either**
plausible source has reached the target date. This favors "it opens on time"
over "it never opens early" — the accepted trade-off is that a source glitching
forward could open the box early. The lock is **fail-closed**: with no plausible
source, it stays locked.

## Security

- Password/PIN stored as a **salted PBKDF2-HMAC-SHA256** hash (never plaintext).
- **Exponential backoff + lockout** on wrong attempts, persisted to NVS so it
  survives a reboot. No recovery path by design.
- Config (capsule date, hash, theme, WiFi) persists in **NVS**, so a sealed,
  battery-less box keeps its state through a power cut.

## Porting

All board-specific values live in [`include/board_config.h`](../include/board_config.h).

- **Different wiring, same ESP32-WROOM-32:** edit the pin `#define`s only.
- **Different display / touch controller:** in `src/hw/DisplayLGFX.h`, swap
  `lgfx::Panel_ST7796` / `lgfx::Touch_FT5x06` for your controller's LovyanGFX
  class, then adjust `TS_LCD_INVERT` / `TS_LCD_RGB_ORDER` if colors look wrong.
- **Different ESP32 variant (S3/C3…):** add a PlatformIO env with the right
  `board =`, and pick valid GPIOs for that chip in `board_config.h`.
