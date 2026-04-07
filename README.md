# ESP32-C6 Touch LCD 1.47" — LVGL Animated Clock

[![GitHub](https://img.shields.io/badge/github-andreimagic%2FESP32__C6__Touch__LCD__1__47__LVGL__Animated__Clock-blue?logo=github)](https://github.com/andreimagic/ESP32_C6_Touch_LCD_1_47_LVGL_Animated_Clock)

A smart animated clock for kids built on the **Waveshare ESP32-C6 Touch LCD 1.47"** board, driven by **LVGL v9**. Displays the time in a large custom font, plays animated GIF emotions on a schedule, sounds configurable buzzer alarms, runs a countdown timer, manages brightness via device tilt, hosts a full apps menu with ASCII mini-games, and supports deep-sleep power-off — all configured from a plain `config.ini` on the SD card, no recompile needed.

---

## ⚠️ Disclaimer — ASCII Art

Some coin flip ASCII art displayed in the Apps Menu was sourced from [asciiart.eu/video-games/pokemon](https://www.asciiart.eu/video-games/pokemon). All Pokémon characters and names are trademarks of **The Pokémon Company International**. This project is not affiliated with, sponsored by, or endorsed by The Pokémon Company. The art is used here solely for non-commercial, personal, educational purposes.

---

## Features

| Feature | Details |
|---|---|
| **Big clock face** | HH:MM in a full-screen custom font (Montserrat 96px) |
| **Splash screen** | "Hello!" on cold boot (2.5 s); "Salut!" on wake from sleep (1 s) |
| **Animated GIFs** | Smile (day) and Sleep (night) emotions from SD card |
| **Scheduled animation** | GIF plays on a configurable minute interval, 800 ms fade back to clock |
| **Night mode** | Sleep GIF used automatically between 20:00 and 07:00 |
| **Alarm** | Configurable wake-up time, custom buzzer pattern, `alarm_animation.gif`, fades out after beeping |
| **Countdown timer** | Set HH:MM in the carousel, live `MM:SS` on clock face, `timer_animation.gif` on completion |
| **Animation priority** | Alarm and timer always evict any running scheduled animation before playing |
| **Emotion tilt** | While the smile GIF plays (upper-left tap), tilt the device to change emotion in real-time |
| **Carousel settings** | Long-press → swipe through Clock / Timer / Alarm / WiFi settings |
| **Clock editor** | Sets HH:MM **and** DD/MON/YYYY — full date+time offline, no WiFi needed |
| **Brightness schedule** | Auto-dims at 19:00 → 19:30 → 20:00, brightens at 06:00 → 07:00 |
| **Tilt brightness** | Tilt device left/right in the Status screen to adjust brightness in 10% steps |
| **WiFi + NTP** | Connects at boot, syncs time automatically; can be disabled from the carousel |
| **DST-aware timezone** | POSIX `tz` string in `config.ini` handles daylight saving automatically forever |
| **RTC persistence** | Hourly timestamp log on SD card (`/last_seen.txt`) restores time on cold boot without WiFi |
| **Status screen** | Today's date in the title, WiFi SSID, NTP sync state, current brightness |
| **Battery monitor** | Live percentage, voltage, ADC raw — with LiPo discharge curve |
| **Battery warning** | Clock text turns orange ≤ 25%, red ≤ 10%; auto-poweroff countdown at ≤ 10% |
| **Software power-off** | Long-press battery screen → 5 s countdown → deep sleep; RESET button to wake |
| **Alarm auto-wake** | Device wakes from deep sleep 5 min before alarm to allow NTP sync; falls back to warning screen if sync fails |
| **Apps menu** | Long-press the smile GIF → math challenge gate → ASCII games carousel |
| **Math challenge** | Random arithmetic gate (+ − × ÷, result < 100) with 4 shuffled answer buttons |
| **Rock Paper Scissors** | Animated 3-2-1 countdown shake → CPU reveals its hand → GO! |
| **Rolling Dice** | Animated rolling frames → final dice face reveal |
| **Flip a Coin** | Instant flip with ASCII coin art (heads/tails) |
| **Apps sounds** | Melody on correct math answer, failure tune on wrong; beeps during animations; toggleable |
| **SD card config** | All settings in `/config.ini` — no recompile needed |
| **LVGL v9** | Hardware-accelerated UI, zero blocking in the main loop |

---

## Hardware

### Board

**Waveshare ESP32-C6 Touch LCD 1.47"**

Select **ESP32C6 Dev Module** in Arduino IDE. The board integrates the ST7789 display, AXS5106L touch controller, QMI8658 IMU, ETA6098 battery charger, and SD card slot on a single compact PCB.

### Display

| Component | Value |
|---|---|
| Controller | ST7789 |
| Resolution | 172 × 320 px |
| Interface | SPI (HWSPI) |
| Rotation | Landscape (ROTATION = 1) |

### Pin Map

| Signal | GPIO |
|---|---|
| Display DC | 15 |
| Display CS | 14 |
| Display RST | 22 |
| Display Backlight (PWM) | 23 |
| SPI SCK (shared) | 1 |
| SPI MOSI (shared) | 2 |
| SPI MISO (SD only) | 3 |
| SD Card CS | 4 |
| Touch I²C SDA | 18 |
| Touch I²C SCL | 19 |
| Touch RST | 20 |
| Touch INT | 21 |
| IMU (QMI8658) I²C | shared 18 / 19 |
| Battery ADC | 0 |
| Passive Buzzer | **5** → GND |

> The display, SD card and buzzer share the same SPI bus (SCK=1, MOSI=2). The SD card additionally needs MISO=3. Each device uses its own CS pin.

### Battery & Charging

The board includes an **ETA6098** switching-mode CC/CV charger. It handles charging automatically in hardware — pre-charge, constant current, constant voltage, and end-of-charge termination. Leaving the device plugged in permanently is safe; the IC stops charging and monitors the battery without any firmware involvement.

The firmware reads battery voltage through a ÷3 ADC voltage divider on GPIO0 and maps it to percentage using a piecewise LiPo discharge curve (4.20V = 100%, 3.00V = 0%). A small voltage offset while USB is connected is normal and not a firmware bug.

### Buzzer Wiring

Connect a **passive buzzer** (not active) between **GPIO 5** and **GND**. If the buzzer is very loud, add a 100 Ω resistor in series. The firmware drives it via PWM using `ledcChangeFrequency()` to produce distinct pitches for alarms, menu sounds, and game audio.

---

## Software Dependencies

Install all libraries through **Arduino IDE → Library Manager** unless noted otherwise.

| Library | Version tested | Purpose |
|---|---|---|
| **LVGL** | 9.5.0 | UI framework — widgets, animations, timers |
| **Arduino_GFX_Library** | latest | ST7789 display driver |
| **SD** | built-in ESP32 | SD card file access |
| **WiFi / WiFiMulti** | built-in ESP32 | WiFi connection |
| **FastIMU** | latest | QMI8658 accelerometer (tilt brightness + emotion tilt) |
| **esp_lcd_touch_axs5106l** | board-specific | Capacitive touch controller |

> `SD`, `WiFi`, `WiFiMulti`, `SPI`, and `time.h` are part of the ESP32 Arduino core — no separate install needed.

---

## lv_conf.h Settings

After installing LVGL, edit `Arduino/libraries/lvgl/src/lv_conf.h`:

```c
// Enable the file (first line of the file)
#if 1  /* was #if 0 */

// Memory — use system malloc (required for GIF decoder)
#define LV_USE_STDLIB_MALLOC    LV_STDLIB_CLIB
#define LV_USE_STDLIB_STRING    LV_STDLIB_CLIB
#define LV_USE_STDLIB_SPRINTF   LV_STDLIB_CLIB

// GIF decoder
#define LV_USE_GIF  1

// Fonts — all required
#define LV_FONT_MONTSERRAT_14  1
#define LV_FONT_MONTSERRAT_16  1
#define LV_FONT_MONTSERRAT_48  1
```

> `montserrat_96` and the `dejavu_mono_*` fonts are **custom generated files** — see [Custom Fonts](#custom-fonts) below.

---

## Custom Fonts

### Montserrat 96 (clock digits)

The large clock digits use a custom Montserrat bitmap at 96 px, generated offline to include only the characters needed (digits 0–9 and colon), keeping the file small.

1. Download **Montserrat-Regular.ttf** from [Google Fonts](https://fonts.google.com/specimen/Montserrat)
2. Go to **[https://lvgl.io/tools/fontconverter](https://lvgl.io/tools/fontconverter)**
3. Settings: Font = `Montserrat-Regular.ttf`, Size = `96`, Range = `0x30-0x3A`, Bpp = `4`, Name = `montserrat_96`
4. Download `montserrat_96.c` and place it in the sketch folder

### DejaVu Mono (apps menu / ASCII games)

The apps menu and ASCII game screens use DejaVu Sans Mono for fixed-width art rendering. Three sizes are needed: 8, 14, and 16 px.

1. Download **DejaVuSansMono.ttf** from [dejavu-fonts.github.io](https://dejavu-fonts.github.io)
2. Use the same LVGL font converter tool above
3. Generate three files with Name = `dejavu_mono_8` / `dejavu_mono_14` / `dejavu_mono_16`, same size values, full printable ASCII range (`0x20-0x7E`), Bpp = `4`
4. Place all three `.c` files in the sketch folder

Arduino will compile them automatically as part of the project.

---

## SD Card Setup

Format the SD card as **FAT32**. Create the following structure:

```
SD root/
├── config.ini
├── last_seen.txt            ← created automatically by the firmware
└── cruzr_emotions/
    ├── cruzr_smile.gif          ← 160 × 86 px  (scheduled day animation + emotion: upright)
    ├── cruzr_sleep.gif          ← 160 × 86 px  (scheduled night animation + emotion: tilt back)
    ├── cruzr_sad.gif            ← 160 × 86 px  (emotion: tilt forward)
    ├── cruzr_joy.gif            ← 160 × 86 px  (emotion: tilt left or right)
    ├── alarm_animation.gif      ← 160 × 86 px  (plays when alarm fires)
    └── timer_animation.gif      ← 160 × 86 px  (plays when countdown reaches zero)
```

### GIF Requirements

GIF files **must be resized to 160 × 86 pixels** before copying to the SD card. The LVGL GIF decoder allocates an ARGB8888 canvas (width × height × 4 bytes). At full 320 × 172 px that requires 220 KB of contiguous RAM which the ESP32-C6 cannot provide when WiFi is active. At 160 × 86 px it needs only 55 KB.

**To resize:** go to [https://ezgif.com/resize](https://ezgif.com/resize), upload your GIF, set Width=160 Height=86, download and copy to the SD card.

The firmware scales them 2× at render time to fill the 320 × 172 screen.

### RTC Persistence Log

The firmware automatically creates and maintains `/last_seen.txt` on the SD card. Every hour (and whenever the clock or config is saved) it appends a line like:

```
2026-03-23 07:30:00Z (3.92V)
```

On cold boot with no WiFi, the firmware reads the last line and restores the RTC to that timestamp. Logging stops automatically if battery voltage drops below 3.4V.

---

## config.ini Reference

```ini
[wifi]
enabled = true
ssid = myhomewifi
password = changeme

[clock]
ntp_server = pool.ntp.org
# POSIX timezone string — set once, handles DST automatically forever.
# Netherlands: CET-1CEST,M3.5.0,M10.5.0/3
# UK:          GMT0BST,M3.5.0/1,M10.5.0
# US Eastern:  EST5EDT,M3.2.0,M11.1.0
# US Pacific:  PST8PDT,M3.2.0,M11.1.0
# No DST (Japan): JST-9
tz = CET-1CEST,M3.5.0,M10.5.0/3

[alarm]
enabled = true
time = 07:10
# Number of 4-beep sequences before auto-stop. 0 = beep until screen is touched.
beep_sequences = 5

[timer]
# Last used countdown duration (HH:MM). Saved automatically.
duration = 00:05
beep_sequences = 3

[animation]
# Play smile GIF (day) or sleep GIF (night) automatically
# set schedule = false to disable
schedule = true
# Seconds the GIF plays before fading back to the clock (3-60)
duration = 10

[menu]
# Mutes/unmutes Apps Menu math sounds and game audio only
sounds = true
```

| Section | Key | Default | Description |
|---|---|---|---|
| `[wifi]` | `enabled` | `true` | Enable WiFi and NTP sync |
| `[wifi]` | `ssid` | `myhomewifi` | WiFi network name |
| `[wifi]` | `password` | `changeme` | WiFi password |
| `[clock]` | `ntp_server` | `pool.ntp.org` | NTP time server |
| `[clock]` | `tz` | `CET-1CEST,M3.5.0,M10.5.0/3` | POSIX timezone string (DST-aware) |
| `[alarm]` | `enabled` | `false` | Enable morning alarm |
| `[alarm]` | `time` | `07:00` | Alarm time (HH:MM) |
| `[alarm]` | `beep_sequences` | `5` | Alarm buzzer repeat count (0 = until touch) |
| `[timer]` | `duration` | `00:00` | Last countdown duration, saved automatically |
| `[timer]` | `beep_sequences` | `3` | Timer buzzer repeat count (0 = until touch) |
| `[animation]` | `schedule` | `true` | Enable periodic GIF animation |
| `[animation]` | `duration` | `10` | Seconds each scheduled GIF plays before fading |
| `[menu]` | `sounds` | `true` | Apps menu and game sounds on/off |

> If `config.ini` is missing the firmware boots with the hardcoded defaults shown above.

---

## Home Screen Touch Zones

The home screen has **four invisible touch zones**. Tap to open a sub-screen. **Long-press anywhere** to open the Carousel settings menu.

```
┌─────────────────────────────────────────┐
│                   │                     │
│   Smile GIF       │    Analog Clock     │
│   (upper-left)    │    (upper-right)    │
│                   │                     │
├───────────────────┼─────────────────────┤
│                   │                     │
│   Status          │    Battery          │
│   (lower-left)    │    (lower-right)    │
│                   │                     │
└─────────────────────────────────────────┘
         LONG-PRESS anywhere → Carousel menu
```

When the countdown timer is running, a small `⏹ MM:SS` label appears in the bottom-left corner. When the alarm is enabled, a 🔔 bell icon with the alarm time appears in the bottom-right corner.

---

## Carousel Settings Menu

Long-press anywhere on the clock face opens the carousel. Use the **◀ ▶** arrows on the left and right edges to cycle through the four items. The current position is shown as dots at the bottom.

```
┌─────────────────────────────────────────┐
│                                         │
│  ◀         [ ICON ]           ▶         │
│             NAME                        │
│           description                  │
│                                         │
│        tap  ·  hold to exit             │
│              ● ○ ○ ○                    │
└─────────────────────────────────────────┘
```

**Tap** the centre to enter the selected item. **Long-press** anywhere to exit back to the clock.

The four items are Clock (set date+time), Timer (countdown), Alarm (wake-up), and WiFi (on/off toggle).

### Clock editor — set date and time

Opens a two-row editor pre-loaded with the current RTC values:

```
┌─────────────────────────────────────────┐
│    ▲              ▲                     │
│  [ HH ]  :  [ MM ]                      │  montserrat_48
│    ▼              ▼                     │
│  ─────────────────────────────────────  │
│   ▲      ▲        ▲                     │
│  [22]  /[Mar]/ [2026]                   │  montserrat_16
│   ▼      ▼        ▼                     │
│        hold to save & exit              │
└─────────────────────────────────────────┘
```

Adjust all five values with ▲/▼. Day wraps correctly when the month changes (e.g. Jan 31 → Feb clips to 28 or 29). Long-press commits both date and time to the ESP32 RTC via `settimeofday()` and logs the new timestamp to `last_seen.txt`. NTP will correct the time on the next sync when WiFi is available.

### Timer — countdown

Opens the HH:MM editor with a **Ready! / Not yet** toggle. The timer always opens as "Not yet" so you must explicitly enable it before saving.

Long-press with **Ready!** selected starts the countdown immediately and returns to the clock face. The remaining time shows as `⏹ MM:SS` (or `H:MM:SS` for durations over one hour) in the bottom-left corner.

When the countdown reaches zero, any running scheduled animation is first dismissed, then `timer_animation.gif` plays fullscreen and the buzzer sounds `beep_sequences` times. The animation fades out automatically after the last beep.

To stop a running timer: open the carousel → Timer → set to **Not yet** → long-press. The label disappears from the clock face.

### Alarm — wake-up alarm

Opens the HH:MM editor with an **ON / OFF** toggle. Long-press saves to `config.ini`. When enabled, at the configured time any running scheduled animation is first dismissed, then the device raises brightness to 50%, plays `alarm_animation.gif` fullscreen, and sounds the buzzer `beep_sequences` times. The animation fades out automatically after the last beep. Touching the screen dismisses the alarm early.

### WiFi — inline toggle

Tap the centre to toggle WiFi on or off. No sub-screen. The description updates to green **ON** or red **OFF** immediately, and the change is saved to `config.ini`. When WiFi is disabled, NTP sync is suspended and reconnect attempts are skipped entirely.

---

## Sub-screens

### Analog Clock (upper-right tap)
Top-right corner shows an Analog clock, view stays opened and refreshes every minute to display the correct time. Clicking on it will return  to the regular Time view. A filled sector centered on the clock center that starts at the top of the hour (12 o’clock) and sweeps clockwise to the current minute position, visually like a pie chart showing elapsed minutes in the current hour. 

### Status (lower-left tap)
Title shows today's date (e.g. `Mon 23 Mar 2026`) when the RTC holds a valid time, falling back to `Status` on a fresh unconfigured boot. Shows WiFi connection status (SSID or disconnected), NTP sync state, and current brightness level. **Tilt the device left or right** while this screen is open to decrease or increase brightness in 10% steps.

### Battery (lower-right tap)
Shows live battery percentage (using a LiPo discharge curve), voltage to two decimal places, and raw ADC value — all updated every second.

**Long-press** on the battery screen opens a shutdown confirmation popup with a 5-second countdown and a **Cancel** button. If not cancelled, the device enters deep sleep.

Battery level is also reflected in the clock face text colour:

| Level | Clock colour |
|---|---|
| > 25% | White |
| 11–25% | Orange |
| ≤ 10% | Red + auto-poweroff countdown (60 s) |

### GIF animations (upper taps)
Opens the corresponding GIF fullscreen. Tap anywhere to return to the clock.

---

## Emotion Tilt — Interactive GIF Mode

Tapping the **upper-left** zone opens the smile GIF as usual. While this GIF is playing, if the IMU is available, tilting the device changes the emotion in real-time without touching the screen:

| Device orientation | GIF shown |
|---|---|
| Upright (flat / normal) | `cruzr_smile.gif` |
| Tilt backwards (top away from you) | `cruzr_sleep.gif` |
| Tilt forward (top toward you) | `cruzr_sad.gif` |
| Tilt left or right | `cruzr_joy.gif` |

The swap happens in-place — the GIF changes without closing the overlay or any visible flicker. The tilt is polled every 400 ms. A threshold of 0.4 g on the X axis (forward/backward) and Y axis (left/right) must be exceeded for the emotion to change, so small accidental movements are ignored.

Tapping the screen dismisses the animation and returns to the clock, as usual.

**Long-press the smile GIF** to enter the Apps Menu (math gate first).

---

## Apps Menu

### Entry flow

```
Upper-left tap → Smile GIF plays (emotion tilt active)
Long-press GIF → Math challenge gate
  ✓ Correct    → Success melody → Apps carousel
  ✗ Wrong      → Failure tune → Big white "X" (3 s) → Clock
```

### Math challenge

A random arithmetic problem (+ − × ÷, result always < 100) is shown in large font. Four shuffled answer buttons appear below. The correct button plays a 12-note success melody; a wrong tap plays a low two-note failure tune and returns to the clock after 3 seconds.

### Apps carousel

Three games plus a sounds toggle, navigated with **◀ ▶**. **Tap** to enter, **long-press** to go back.

```
┌─────────────────────────────────────────┐
│                                         │
│  ◀     Rock Paper Scissors    ▶         │
│         An interactive ASCII Game       │
│                                         │
│      tap to play  .  hold to exit       │
│              ● ○ ○ ○                    │
└─────────────────────────────────────────┘
```

#### Rock Paper Scissors

Tap to start. The device plays an animated countdown in ASCII art:

```
Ready? → UP "3" → DOWN "3" ♪ → UP "2" → DOWN "2" ♪ → UP "1" → DOWN "1" ♪ → GO! ♪♪
```

Each step is exactly 250 ms. At GO! the CPU's hand is revealed — play against it with your own hand. Tap to play again, long-press to exit.

#### Rolling Dice

Tap to start. Three animated rolling frames play at 250 ms each (one beep per frame), then the final face (1–6) is revealed with a high tone. Tap to re-roll.

#### Flip a Coin

Tap anywhere to flip. Instant result with ASCII coin art (heads / tails) and a high-tone beep. Tap to flip again.

#### Sounds toggle

The fourth carousel item. Tap to mute/unmute all apps menu and game audio. The setting is saved to `config.ini` under `[menu] sounds`. This does **not** affect alarm or timer buzzer sounds.

---

## Power Off and Wake

### Powering off
Long-press the battery screen to open the shutdown popup. After the countdown (or immediately if not cancelled) the device enters deep sleep drawing ~10 µA.

### Waking up
Press the **RESET** button on the device body. This always causes a clean reboot through the full boot sequence.
Low-battery gate code will check at startup if the battery is > 10%.

> The BOOT button on this board is wired to GPIO9, which is not a low-power GPIO on the ESP32-C6 and cannot trigger a wake-from-deep-sleep interrupt. RESET is the reliable wake method.

### Alarm auto-wake and NTP guard

If an alarm is set, the firmware calculates the sleep duration and wakes automatically:

- **Alarm > 5 min away:** wakes 5 minutes early so WiFi and NTP have time to sync before the alarm fires
- **Alarm ≤ 5 min away:** wakes 30 seconds early (no time for NTP; relies on RTC drift being minor)

At alarm time, the following logic applies:

| Condition | Behaviour |
|---|---|
| Device uptime > 5 min | Alarm fires normally (was already running, time assumed reliable) |
| Uptime ≤ 5 min, NTP synced | Alarm fires normally (accurate time confirmed) |
| Uptime ≤ 5 min, NTP not yet synced | Alarm held — checked every minute |
| NTP synced while held | Alarm fires immediately |
| 15 min elapsed, NTP never synced | Warning screen: "HELLO! / NTP not in sync / Check the time!" + buzzer |

### Boot behaviour

| Scenario | Splash | Duration | Clock |
|---|---|---|---|
| Cold boot / RESET, WiFi available | `Hello!` | 2.5 s | `--:--` until NTP syncs |
| Cold boot, no WiFi, log exists | `Hello!` | 2.5 s | Time restored from `last_seen.txt` |
| Cold boot, no WiFi, no log | `Hello!` | 2.5 s | `--:--` until manually set |
| Wake from deep sleep (alarm timer) | `Salut!` | 1.0 s | RTC time shown immediately |

---

## Daily Automation Schedule

All times are local time. Automation runs whenever the RTC holds a valid time (epoch > 2026-01-01), regardless of WiFi or NTP status.

| Time | Action |
|---|---|
| 06:00 | Brightness → 10% |
| 06:30 | Brightness → 25% |
| 07:00 | Brightness → 50% |
| Alarm time | Dismiss scheduled GIF, brightness → 50%, `alarm_animation.gif`, buzzer |
| Every N min | Scheduled GIF animation (if enabled; skipped when alarm fires same minute) |
| 19:00 | Brightness → 25% |
| 19:30 | Brightness → 10% |
| 20:00 | Brightness → 1% |
| 20:15 | Sleep GIF starts automatically |
| 21:00 | Sleep GIF closes automatically |

---

## Scheduled Animation

When `[animation] schedule = true`, a GIF plays automatically on the configured interval and fades back over 800 ms.

| Time of day | GIF played |
|---|---|
| Day (07:00–19:59) | `cruzr_smile.gif` |
| Night (20:00–06:59) | `cruzr_sleep.gif` |

Skipped silently if any screen, overlay, or carousel is already open. Alarm and timer always evict a running scheduled animation before playing their own.

---

## Buzzer Sounds

### Alarm and timer pattern

**4 × (200 ms ON + 100 ms OFF) + 1000 ms pause** = one sequence. `beep_sequences` controls how many repeat before auto-stop (0 = until touch). When finite, the animation fades out automatically after the last beep.

### Apps menu sounds (respects `[menu] sounds`)

| Event | Sound |
|---|---|
| Correct math answer | 12-note ascending melody |
| Wrong math answer | Low two-note failure tune (G4 → C4) |
| RPS down move (×3) | Short A4 beep |
| RPS GO! reveal | High C6 tone |
| Dice rolling frame (×3) | Short A4 beep |
| Dice result reveal | High C6 tone |
| Coin flip result | High C6 tone |
| Sounds toggle turned ON | High C6 tone (confirmation) |

---

## Build & Flash

1. Clone the repository:
   ```bash
   git clone https://github.com/andreimagic/ESP32_C6_Touch_LCD_1_47_LVGL_Animated_Clock.git
   ```
2. Open **Arduino IDE 2.x**
3. Install board support: **File → Preferences → Additional URLs**, add:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
   Then open **Tools → Board → Boards Manager**, search `esp32` and install **esp32 by Espressif**.

4. Select and configure the board — **all settings below are mandatory**:

   | Setting | Value |
   |---|---|
   | **Board** | `ESP32C6 Dev Module` |
   | **USB CDC On Boot** | `Enabled` |
   | **Flash Size** | `8MB (64Mb)` |
   | **Partition Scheme** | `8MB with spiffs (3MB APP/1.5MB SPIFFS)` |
   | CPU Frequency | `160MHz (WiFi)` _(recommended)_ |
   | Flash Frequency | `80MHz` |
   | Flash Mode | `QIO` |
   | Upload Speed | `921600` |
   | JTAG Adapter | `Disabled` |
   | Zigbee Mode | `Disabled` |

   > **USB CDC On Boot must be Enabled** — without it the Serial Monitor will not receive any output and the device may not be recognised on the port.
   > **Flash Size and Partition Scheme must match** — the 3MB APP partition is required to fit the firmware with LVGL v9 and all libraries.

5. Set the correct **Port** (e.g. `COM3` on Windows, `/dev/ttyUSB0` on Linux/macOS)
6. Install all libraries listed in [Software Dependencies](#software-dependencies)
7. Edit `lv_conf.h` as described in [lv_conf.h Settings](#lv_confh-settings)
8. Place all custom font `.c` files in the sketch folder (see [Custom Fonts](#custom-fonts))
9. Prepare the SD card as described in [SD Card Setup](#sd-card-setup)
10. Open `ESP32_C6_Touch_LCD_1_47_LVGL_Animated_Clock.ino`, click **Upload**
11. Open Serial Monitor at **115200 baud** to watch the boot log

### Expected Boot Log

```
========== BOOT ==========
[BOOT] Wake cause: cold boot / RESET button
[1] Pulling CS pins HIGH...
    Done.
[2] SPI.begin(SCK=1, MISO=3, MOSI=2, CS=4)...
    Done.
[3] Initialising display...
    gfx->begin() OK.
[BL] Backlight init at 50% (PWM=127)
    Display ready.
[4] Initialising touch...
read: 8161
    Touch ready.
[4b] Initialising IMU...
    IMU ready.
[5] Mounting SD card...
    CS=4  SCK=1  MISO=3  MOSI=2  speed=4MHz
    SD.begin() returned: true
    SD mounted OK — type: SD  size: 244 MB
[CFG] Loading /config.ini...
[CFG]   wifi.enabled       = true
[CFG]   wifi.ssid     = myhomewifi
[CFG]   wifi.password = (hidden)
[CFG]   alarm.enabled      = true
[CFG]   alarm.time         = 07:10
[CFG]   alarm.beep_sequences = 5
[CFG]   timer.duration      = 00:02
[CFG]   timer.beep_sequences = 3
[CFG]   menu.sounds    = true
[CFG] Done. (49 lines read)
[RTC] Restored UTC time from log: 2026-04-07 12:00:00
    Checking for GIF at: /cruzr_emotions/cruzr_smile.gif
    GIF found — 540954 bytes
[6] Initialising LVGL...
[7] Registering LVGL SD filesystem driver...
[7b] Applying WiFi state from config...
[TZ] Applied: CET-1CEST,M3.5.0,M10.5.0/3
[WiFi] Enabling...
     Done.
[BAT] Boot check: 4.11V = 91%
[8] Building UI...
========== SETUP DONE ==========
```

---

## Troubleshooting

| Symptom | Likely cause | Fix |
|---|---|---|
| Black screen after boot | Display init failed | Check SPI wiring; confirm `gfx->begin() OK` in serial log |
| `SD card mount failed` | Wrong MISO pin or card not FAT32 | Confirm GPIO 3 = MISO; reformat card as FAT32 |
| `GIF not found` | Wrong filename or path | Path is case-sensitive: `/cruzr_emotions/cruzr_smile.gif` |
| GIF shows but wrong size | GIF not resized | Resize to 160 × 86 px using ezgif.com/resize |
| `Not enough RAM for GIF` | GIF still full-size | Must be 160 × 86 px — see [GIF Requirements](#gif-requirements) |
| Clock shows `--:--` permanently | No WiFi, no log, no manual set | Set date+time via Carousel → Clock editor |
| Clock shows wrong time after RESET | Log entry is old | Set time manually or re-enable WiFi for NTP sync |
| Clock 1 hour off after DST change | Old `gmt_offset` config or missing `tz` key | Replace `gmt_offset` with `tz = CET-1CEST,M3.5.0,M10.5.0/3` in `config.ini` |
| Status shows "Status" not date | RTC not yet valid | Cold boot with no WiFi and no log; set time via Clock editor |
| Touch zones unresponsive | Touch controller not detected | Check I²C wiring on pins 18/19 |
| IMU not working | Address mismatch or wiring | Confirm `IMU_ADDRESS = 0x6B`; check serial for IMU error code |
| Alarm not firing | `enabled = false` or device was asleep | Set `enabled = true`; check boot log for wakeup cause |
| Alarm rings twice | RTC drift + NTP correction | Fixed in v1.4.1 — NTP guard prevents double-fire |
| Warning screen at alarm time | NTP did not sync within 15 min | Check WiFi; device woke 5 min early specifically to allow sync |
| Buzzer plays all tones at same pitch | Active buzzer used, or ESP32 core issue | Use a **passive** buzzer; firmware uses `ledcChangeFrequency()` for pitch control |
| Apps menu not opening | Long-pressing wrong zone | Long-press must be on the **smile GIF** (upper-left tap first, then long-press the GIF) |
| Math challenge not appearing | Smile GIF not open | Must open the smile GIF first by tapping upper-left |
| Font not found (compile error) | Custom `.c` files missing | Generate and place `montserrat_96.c`, `dejavu_mono_8.c`, `dejavu_mono_14.c`, `dejavu_mono_16.c` |
| UI stutters during games/math | WiFi reconnect stall | Fixed — `apps_cont` guard added to `wifi_poll_cb` |
| `last_seen.txt` not created | SD write error or low battery | Check SD card is writable FAT32; battery must be above 3.4V |
| BOOT button doesn't wake from sleep | Hardware limitation | GPIO9 is not an LP GPIO on ESP32-C6; press **RESET** instead |
| Battery % jumps on unplug | ADC reads elevated USB voltage | Normal — LiPo estimate is slightly elevated while USB powers the system |

---

## Architecture Notes

- **No blocking calls in `loop()`** — `loop()` only calls `lv_timer_handler()` + `delay(5)`. All WiFi polling, clock ticks, brightness schedules, buzzer patterns, countdown timer, and animations run as LVGL timer callbacks.
- **SD ↔ LVGL filesystem bridge** — a custom `lv_fs_drv_t` registered under drive letter `'S'` forwards all LVGL file operations to the Arduino `SD` library. This lets `lv_gif_set_src()` open files directly from the card with the prefix `S:/`.
- **GIF memory management** — the LVGL GIF decoder needs a contiguous block for its canvas. GIFs are pre-scaled to 160×86 px (55 KB canvas) so they fit alongside the WiFi stack. The render buffer uses 20 scan lines for good throughput without exhausting RAM.
- **DST-aware timekeeping** — `configTzTime(tz_string, ntp_server)` sets the POSIX TZ env var and starts SNTP in a single call. NTP delivers UTC; `localtime_r()` converts to correct local time including DST transitions automatically. `setenv("TZ", tz_string, 1)` is also called before WiFi starts so offline use (restore from log) is correct too.
- **RTC persistence** — `log_last_seen()` appends a timestamped voltage reading to `/last_seen.txt` every hour, on every config save, and on clock editor use. `restore_time_from_log()` reads the last entry on cold boot. With TZ set, `mktime()` converts local→UTC correctly including DST.
- **Carousel** — a full-screen LVGL modal opened by long-press. Each tap on ◀/▶ calls `lv_obj_clean()` and rebuilds the view in place. The centre zone uses `LV_EVENT_CLICKED` (not `LV_EVENT_PRESSED`) so long-press and tap are mutually exclusive — the editor never opens before the long-press exit fires.
- **Shared editor** — `open_editor()` builds the HH:MM widget for Timer and Alarm. `open_clock_editor()` builds the full two-row date+time widget. `modal_longpress_cb()` dispatches to the correct save function based on `carousel_idx`.
- **Animation priority** — `close_scheduled_gif()` forcefully tears down any scheduled overlay (cancels fade timer, deletes overlay synchronously) before alarm or timer open their GIF. Scheduled animation is also skipped entirely if the alarm fires on the same minute.
- **Apps menu** — `apps_cont` is a global LVGL object separate from `modal_cont` and `overlay_cont`. `wifi_poll_cb` skips `wifiMulti.run()` while it is open to prevent radio lock stalls during gameplay and math input.
- **ASCII games** — all art is rendered using DejaVu Mono fixed-width font via LVGL labels. RPS and Dice use LVGL timer callbacks (`rps_anim_tick_cb`, `dice_anim_tick_cb`) at 250 ms intervals for consistent animation cadence. Buzzer tones use `ledcChangeFrequency()` to switch pitch without re-attaching the PWM channel.
- **Emotion tilt** — `zone_ul_cb` sets `emotion_tilt_active = true` and starts `tilt_timer` after opening the smile GIF. `tilt_poll_cb` branches on this flag: in emotion mode it reads both `accelX` (forward/back) and `accelY` (left/right), determines the desired GIF path, and calls `lv_gif_set_src()` on the existing widget (retrieved from `overlay_cont` user data) only when the path changes. This swaps the animation in-place with no overlay rebuild. Both the flag and the timer are cleared by `overlay_close_event_cb`.
- **Buzzer state machine** — a single 9-step table drives both alarm and timer patterns. `buzzer_fade_after` is set by the caller for finite sequences; `buzzer_stop()` triggers `overlay_fade_and_close()` automatically after the last beep.
- **WiFi reconnect guard** — `wifi_poll_cb()` skips `wifiMulti.run()` when any modal or overlay is open, when WiFi is manually disabled (`cfg.wifi_enabled`), and reduces attempts to every 30 s when disconnected — preventing radio lock stalls from blocking UI interaction.
- **Automation gate** — `run_daily_automation()` fires when `now > 2026-01-01` (RTC sanity check) instead of `timeSynced`, so brightness schedules, alarms, and animations all work correctly when WiFi is disabled or the time was set manually.
- **Deep sleep & Alarm NTP guard** — `boot_millis` captured at the very start of `setup()`. Wakes 5 min before alarm when > 5 min away, 30 s when close. Holds `alarm_ntp_pending` if time unconfirmed; falls back to warning overlay after 15 min.
- **config.ini** — parsed once at boot with a hand-rolled INI reader (no external library). On save, `[wifi]`, `[alarm]`, `[timer]`, and `[menu]` sections are fully rewritten; all other sections and comments are preserved.

---

## Roadmap

| Version | Status | Feature |
|---|---|---|
| v1.0 | ✅ released | Clock, alarms, GIF on tap, buzzer, brightness tilt, config.ini |
| v1.1 | ✅ released | Scheduled animation (smile/sleep, configurable interval, 800 ms fade) |
| v1.2 | ✅ released | Carousel settings menu, countdown timer, WiFi toggle, manual time set |
| v1.3.0 | ✅ released | Full date editor, RTC persistence via SD log, animation priority fix, automation without WiFi |
| v1.4.0 | ✅ released | Emotion tilt GIF mode on upper-left tap (smile/sleep/sad/joy via IMU) |
| v1.4.1 | ✅ released | Bugfix: Fix RTC drift after long deep sleep in the event of an alarm set, allow time for NTP sync |
| v1.5.0 | ✅ released | Apps menu: math gate, Rock Paper Scissors, Rolling Dice, Flip a Coin, game sounds, DST-aware timezone |
| v2.0.0 | workinprogress| Analog clock view & low-power startup gate |

## License

MIT — do whatever you like with it.
