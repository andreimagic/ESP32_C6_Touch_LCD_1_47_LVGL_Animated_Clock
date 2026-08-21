## 🌐 View Demo

👉 https://andreimagic.github.io/ESP32_C6_Touch_LCD_1_47_LVGL_Animated_Clock/

[![Watch Demo](https://img.youtube.com/vi/FQkz1KrQX3I/0.jpg)](https://youtu.be/FQkz1KrQX3I)

# ESP32-C6 / ESP32-S3 Touch LCD 1.47" — LVGL Animated Clock

[![GitHub](https://img.shields.io/badge/github-andreimagic%2FESP32__C6__Touch__LCD__1__47__LVGL__Animated__Clock-blue?logo=github)](https://github.com/andreimagic/ESP32_C6_Touch_LCD_1_47_LVGL_Animated_Clock)
[![Build](https://github.com/andreimagic/ESP32_C6_Touch_LCD_1_47_LVGL_Animated_Clock/actions/workflows/build.yml/badge.svg?branch=main)](https://github.com/andreimagic/ESP32_C6_Touch_LCD_1_47_LVGL_Animated_Clock/actions/workflows/build.yml)
[![Release](https://img.shields.io/github/v/release/andreimagic/ESP32_C6_Touch_LCD_1_47_LVGL_Animated_Clock)](https://github.com/andreimagic/ESP32_C6_Touch_LCD_1_47_LVGL_Animated_Clock/releases/latest)

A smart animated clock for kids that runs on **two Waveshare 1.47" touch boards — the ESP32-C6 and the ESP32-S3** — driven by **LVGL v9**. Displays the time in a large custom font, plays animated GIF emotions on a schedule, sounds configurable buzzer alarms, runs a countdown timer, adjusts brightness by swipe (or tilt, where an IMU is fitted), hosts a full apps menu with ASCII mini-games, and supports deep-sleep power-off — all configured from a plain `config.ini`, no recompile needed.

One sketch builds for both boards. The target is detected at compile time and every pin, bus and capability difference is resolved in [`board_config.h`](board_config.h) — see [Board Abstraction](#board-abstraction-board_configh). Storage is chosen at boot: an SD card if one is mounted, otherwise the S3's internal flash.

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
| **Emotion tilt** | While the smile GIF plays (upper-left tap), tilt the device to change emotion in real-time _(C6 only — needs an IMU)_ |
| **Dual-board support** | One sketch builds for both the ESP32-C6 and the ESP32-S3; pins, buses and capabilities resolved in `board_config.h` |
| **Carousel settings** | Long-press → swipe through Clock / Timer / Alarm / WiFi settings |
| **Clock editor** | Sets HH:MM **and** DD/MON/YYYY — full date+time offline, no WiFi needed |
| **Brightness schedule** | Auto-dims at 19:00 → 19:30 → 20:00, brightens at 06:00 → 07:00 |
| **Brightness adjust** | Swipe left/right in the Status screen to adjust brightness in 10% steps — or tilt the device, where an IMU is fitted (C6) |
| **WiFi modes** | Three-way radio policy set from the carousel or `config.ini`: **WiFi** (join network, NTP syncs), **AP** (own hotspot only, no internet), **Off** (airplane mode — radio fully down) |
| **Resilient WiFi connect** | Non-blocking STA association with a disconnect-reason state machine: a rejected password auto-falls-back to the AP hotspot after 2 tries (so the web UI stays reachable to fix it); an unreachable network backs off the radio 30 s → 60 s → 2/4/8 min → 10 min cap instead of scanning forever |
| **Web configuration** | PIN-protected browser UI served by the device — edit `config.ini`, set date/time, reboot; accessible in both WiFi and AP mode |
| **AP hotspot** | Own **open** hotspot (no WPA2 passphrase) named `ESP32-Clock-XXXXXX` (unique per device, suffixed from its MAC) when in AP mode or rescuing a rejected password; the boot-generated PIN only authorises the web UI's mutating actions, not joining the hotspot |
| **DST-aware timezone** | POSIX `tz` string in `config.ini` handles daylight saving automatically forever |
| **RTC persistence** | Hourly timestamp log on SD card (`/last_seen.txt`) restores time on cold boot without WiFi |
| **Status screen** | Today's date in the title, WiFi SSID/mode, NTP sync state, current brightness |
| **Battery monitor** | Live percentage, voltage, ADC raw — with LiPo discharge curve |
| **Battery warning** | Clock text turns orange ≤ 25%, red ≤ 10%; auto-poweroff countdown at ≤ 10% |
| **Software power-off** | Long-press battery screen → 5 s countdown → deep sleep; RESET button to wake |
| **Alarm auto-wake** | In WiFi mode, device wakes from deep sleep 5 min before alarm to allow NTP sync; in AP/Off mode it wakes right at alarm time instead (no sync possible, no point burning battery early) |
| **Alarm drift warning** | If the alarm fires with no reliable time source — AP/Off mode, or WiFi mode where NTP never landed — it still always sounds, but swaps the GIF for a plain text warning telling you to check another clock |
| **Apps menu** | Long-press the smile GIF → math challenge gate → ASCII games carousel |
| **Math challenge** | Random arithmetic gate (+ − × ÷, result < 100) with 4 shuffled answer buttons |
| **Rock Paper Scissors** | Animated 3-2-1 countdown shake → CPU reveals its hand → GO! Shake or hard-tilt the device to restart |
| **Rolling Dice** | Animated rolling frames → final dice face reveal. Shake or hard-tilt to re-roll |
| **Flip a Coin** | Instant flip with ASCII coin art (heads/tails) |
| **Tennis Letters** | Breakout-style ASCII game. Catch cycling letters (a-z) with a tilt-controlled paddle. Score points and complete alphabets |
| **Letters Rain** | Falling-letters ASCII game. Letters and modifiers rain in waves; catch the target letter (A→Z) with a gyro-controlled paddle. Wrong catches shrink the paddle; `+` / `-` modify its size; `*` restores the default. Miss the target letter and the game ends. Last score saved to `config.ini` |
| **Snake Letters** | Classic snake ASCII game. Steer using tilt to eat the alphabet (a-z). Avoid distraction letters and manage length with modifiers. High scores persisted to `config.ini` |
| **Bingo!** | On-device 1–90 number caller. Tap or tilt to draw a ball with a cycling reveal animation and buzzer; long-press the circle for a history popup of every number called. No repeats — powered by a fresh Fisher-Yates shuffle each game |
| **Printable Bingo tickets** | Web Configuration page links to `/bingo` — a self-contained, in-browser generator for UK/housie-style 3×9 ticket sheets (1–90) to pair with the on-device caller. Fresh tickets every load, no PIN needed |
| **Gyro shake/tilt trigger** | While playing RPS or Dice, physically shaking the device (ΔaccelZ > 1.8 g) or tilting it hard sideways (accelY > 1.0 g) restarts the game — no tap needed |
| **Metronome** | Full-screen BPM metronome (60–240 BPM) with hardware-timer accuracy, visual beat dots, time-signature selector (2/4, 3/4, 4/4), and distinct hi/lo tones for downbeat vs weak beats |
| **Apps sounds** | Melody on correct math answer, failure tune on wrong; beeps during animations; toggleable. Metronome always sounds regardless of this toggle |
| **Birthday Easter egg** | If today matches any date in the `[birthdays]` list, both the Alarm and Timer play the **Happy Birthday** melody and show `happybirthday.gif` instead of their normal GIF — fully transparent to the user |
| **SD card config** | All settings in `/config.ini` — no recompile needed |
| **LVGL v9** | Hardware-accelerated UI, zero blocking in the main loop |

---

## Hardware

### Board

Two boards are supported, both from Waveshare and both built around the same
1.47-inch panel and AXS5106L touch controller:

| | **[ESP32-C6 Touch LCD 1.47"](https://www.waveshare.com/wiki/ESP32-C6-Touch-LCD-1.47)** | **[ESP32-S3 Touch LCD 1.47"](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.47)** |
|---|---|---|
| Arduino IDE board | `ESP32C6 Dev Module` | `ESP32S3 Dev Module` |
| Flash | 8 MB | 16 MB |
| PSRAM | none | 8 MB OPI |
| IMU | QMI8658 | **none** |
| SD card slot | shares the LCD SPI bus | own dedicated SPI pins |
| Internal filesystem | — | 9.9 MB FFat partition |
| Panel | 172 × 320, JD9853 (ST7789 command set) | identical |

The C6 is the original target and the board this project was developed on. It
features a QMI8658 IMU for motion sensing, an ETA6098 battery charger, and an SD
card slot. This all-in-one design simplifies wiring and allows for a compact
form factor.

You can purchase the C6 board from [Waveshare](https://www.waveshare.com/esp32-c6-touch-lcd-1.47.htm?&aff_id=150729). It’s an affiliate link, so if you use it, you’re basically buying me a coffee (and I really appreciate it)! ☕

#### Feature differences

The S3 board has no IMU, so every tilt-driven interaction is unavailable there.
The firmware detects this **at runtime** (not at compile time), so a C6 whose
QMI8658 fails to answer at boot degrades exactly the same way.

| Feature | C6 | S3 | Notes |
|---|---|---|---|
| Brightness adjust | tilt **or** swipe | swipe | Swipe left/right on the Status screen works on both |
| Tennis Letters, Letters Rain, Snake Letters | ✅ | **hidden** | Steer only by tilt, so they are removed from the carousel rather than shipped unplayable |
| Emotion GIF cycling | ✅ | **smile only** | The overlay still opens on upper-left tap; `sleep`/`sad`/`joy` are unreachable without tilt |
| Rock Paper Scissors, Dice | ✅ | ✅ | Tap-driven; shake is a bonus, not the only input |
| Bingo | ✅ | ✅ | Tap and the circle both draw |
| RTC restore from log | ✅ | SD card only | The log is deliberately card-only — see [RTC Persistence Log](#rtc-persistence-log) |

> **Not the ESP32-S3-LCD-1.47B.** That is a different SKU which *does* carry a
> QMI8658. This firmware targets the **ESP32-S3-Touch-LCD-1.47**, whose complete
> component list contains no IMU — the I²C bus reaches only the LCD connector and
> the expansion header.

### Display

Key specifications of the display include a resolution of 172 × 320 pixels in landscape mode (ROTATION = 1) and an SPI interface for communication. The touch controller uses I²C for input handling. The board's integrated components make it ideal for building interactive projects like this animated clock.

| Component | Value |
|---|---|
| Controller | JD9853, driven with the ST7789 command set |
| Resolution | 172 × 320 px |
| Interface | C6: `Arduino_HWSPI` on the global `SPI` · S3: `Arduino_ESP32SPI` on **HSPI (SPI3)** |
| Rotation | Landscape (ROTATION = 1) |

The panel, its register init sequence and the whole LVGL layout are identical on
both boards. Only the bus differs: on the C6 the display shares the global `SPI`
object with the SD card, while on the S3 the display is pinned to HSPI explicitly
so it cannot collide with the global `SPI` instance the card uses (the default
would land on FSPI, which does collide).

### Pin Map

Both columns are the values in [`board_config.h`](board_config.h); you never need
to edit them by hand.

| Signal | C6 GPIO | S3 GPIO |
|---|---|---|
| Display DC | 15 | 45 |
| Display CS | 14 | 21 |
| Display RST | 22 | 40 |
| Display Backlight (PWM) | 23 | 46 |
| Display SCK | 1 _(shared)_ | 38 |
| Display MOSI | 2 _(shared)_ | 39 |
| SD Card CS | 4 | 14 |
| SD Card SCK | 1 _(shared)_ | 16 |
| SD Card MOSI | 2 _(shared)_ | 15 |
| SD Card MISO | 3 | 17 |
| Touch I²C SDA | 18 | 42 |
| Touch I²C SCL | 19 | 41 |
| Touch RST | 20 | 47 |
| Touch INT | 21 | 48 |
| IMU (QMI8658) I²C | shared 18 / 19 | — _(no IMU)_ |
| Battery ADC | 0 | 12 |
| Passive Buzzer | **5** → GND | **5** → GND |

> **C6:** the display, SD card and buzzer share one SPI bus (SCK=1, MOSI=2); the
> SD card additionally needs MISO=3. Each device uses its own CS pin.
>
> **S3:** the display and the TF card are on **completely separate pins and
> separate SPI hosts** — the card keeps the global `SPI` (FSPI) while the panel
> gets HSPI. Both boards read the battery through an identical 200K/100K divider,
> so the same ÷3 conversion applies.

> **S3 Display RST is 40, not 47.** Waveshare's own Arduino demo passes 47, which
> is the *touch* reset line — their example is wrong. It appears to work only
> because the JD9853 self-resets at power-on. 40 is confirmed working on hardware.

> **The S3 TF slot works over SPI** despite Waveshare's demo using `SD_MMC`. The
> slot is wired for both modes (nets labelled `SD_D0..D3`/`SD_CLK`/`SD_CMD` *and*
> `SD_MISO`/`SD_MOSI`/`SD_SCLK`/`SD_CS`, with 10K pull-ups throughout), so SPI
> mode is used and the existing `SD.h` code needs no changes.

### Board Abstraction (`board_config.h`)

All hardware differences live in one header. It selects on the
`CONFIG_IDF_TARGET_*` macros that the ESP32 core always defines — deliberately
**not** on `ARDUINO_<BOARD>` variant macros, which depend on which board entry
you happen to pick in the IDE. An unsupported target is a compile error by
design rather than a board that builds and then misbehaves.

Besides the pins above it exposes the capability flags the firmware branches on:

| Macro | C6 | S3 | Meaning |
|---|---|---|---|
| `BOARD_HAS_IMU` | 1 | 0 | A QMI8658 is fitted (tilt features compile in) |
| `BOARD_HAS_INTERNAL_FS` | 0 | 1 | An FFat partition exists to fall back to |
| `BOARD_SD_SHARES_LCD_BUS` | 1 | 0 | The card and panel share one SPI bus |
| `BOARD_NEW_LCD_BUS()` | `Arduino_HWSPI` | `Arduino_ESP32SPI` on HSPI | Expands to the right bus constructor |
| `BOARD_NAME` | `"ESP32-C6-Touch-LCD-1.47"` | `"ESP32-S3-Touch-LCD-1.47"` | Printed in the boot log |

Adding a third board means adding one `#elif` block here — not touching the
sketch.

### Battery & Charging

The board includes an **ETA6098** switching-mode CC/CV charger. It handles charging automatically in hardware — pre-charge, constant current, constant voltage, and end-of-charge termination. Leaving the device plugged in permanently is safe; the IC stops charging and monitors the battery without any firmware involvement.

The firmware reads battery voltage through a ÷3 ADC voltage divider on GPIO0 and maps it to percentage using a piecewise LiPo discharge curve (4.20V = 100%, 3.00V = 0%). A small voltage offset while USB is connected is normal and not a firmware bug.

### Buzzer Wiring

Connect a **passive buzzer** (not active) between **GPIO 5** and **GND**. If the buzzer is very loud, add a 100 Ω resistor in series. The firmware drives it via PWM using `ledcChangeFrequency()` to produce distinct pitches for alarms, menu sounds, and game audio.

---

## Software Dependencies

Install board through **Arduino IDE → Tools → Board → Boards Manager**, search `esp32` and install **esp32 by Espressif** (this will provide SD, WiFi & WifiMulti libraries).

Install all libraries through **Arduino IDE → Library Manager** unless noted otherwise.

| Library | Version tested | Purpose |
|---|---|---|
| **LVGL** | 9.5.0 | UI framework — widgets, animations, timers |
| **Arduino_GFX_Library** | 1.6.7 | Display driver (JD9853 via the ST7789 command set) |
| **FastIMU** | 1.3.0 | QMI8658 accelerometer (tilt brightness + emotion tilt) |
| **esp_lcd_touch_axs5106l** | board-specific (included in repo) | Capacitive touch controller |
| **SD** | built-in ESP32 (pre-install with esp32 Board) | SD card file access |
| **FFat** | built-in ESP32 (pre-install with esp32 Board) | Internal-flash fallback storage (S3) |
| **WiFi / WiFiMulti** | built-in ESP32 (pre-install with esp32 Board) | WiFi connection |

> `SD`, `FFat`, `WiFi`, `WiFiMulti`, `SPI`, and `time.h` are part of the ESP32 Arduino core — no separate install needed.

> **FastIMU is required even for the S3**, which has no IMU: the header is
> included unconditionally, and the tilt code paths are disabled at runtime rather
> than compiled out. The versions above are the exact ones CI pins, so a local
> build reproduces CI byte-for-byte.

---

## lv_conf.h Settings

**Nothing to edit — `lv_conf.h` ships in this repository, in the sketch folder.**
LVGL finds it there automatically (it checks the including file's own directory
before the library's), so a fresh clone builds as-is.

> Do **not** create a second copy inside `Arduino/libraries/lvgl/src/`. LVGL uses
> the first one it finds, so a second copy silently overrides the repo's config
> and is lost on the next library update. If you previously created one, delete it.

CI compiles against this exact file, so the board and the build agree by
construction. For reference, these are the settings the project depends on:

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
├── scripts/                 ← created automatically; macroPad reads it
│   ├── ASCII_house.art          ← bundled sample
│   ├── ASCII_hut.art            ← bundled sample
│   └── ASCII_penguin.art        ← bundled sample
└── cruzr_emotions/
    ├── cruzr_smile.gif          ← 160 × 86 px  (scheduled day animation + emotion: upright)
    ├── cruzr_sleep.gif          ← 160 × 86 px  (scheduled night animation + emotion: tilt back)
    ├── cruzr_sad.gif            ← 160 × 86 px  (emotion: tilt forward)
    ├── cruzr_joy.gif            ← 160 × 86 px  (emotion: tilt left or right)
    ├── alarm_animation.gif      ← 160 × 86 px  (plays when alarm fires)
    ├── timer_animation.gif      ← 160 × 86 px  (plays when countdown reaches zero)
    └── happybirthday.gif        ← 160 × 86 px  (Easter egg — replaces alarm/timer GIF on birthdays)
```

### Storage backends

The firmware picks **one** filesystem at boot and uses it for everything:

1. An **SD card**, if one mounts. A card always wins.
2. Otherwise the **internal FFat partition**, on boards where one exists (S3 only).

Everything — `config.ini`, the GIFs, the web config editor — routes through that
single choice, so the layout above is identical whichever backend is active. The
Status screen's config editor shows a badge telling you which one is in use.

On a board with no `config.ini` on the active storage, the firmware writes a
complete default one on first boot, including the `[clock]`, `[animation]` and
`[birthdays]` sections. **The default WiFi mode is `ap`**, because a device
without a valid config has no valid credentials either, and on internal flash the
web UI is the only way to set them.

> **A cardless S3 needs its GIFs put on flash once.** Boot the board with a card
> in the slot and the animations are mirrored onto the FFat partition
> automatically at step `[7a]`, after which the card can come out for good. On a
> board that has never seen a card, upload them through the
> [file manager](#file-manager) at `/files` instead. Until either has happened it
> shows `GIF not found on Internal flash (FFat)` where an animation would be.
>
> **Provisioning covers GIFs and `config.ini` only — not `/scripts`.** macroPad
> art is never copied from card to flash, so a board that mirrors its animations
> and then has the card removed comes back with an empty script list. See
> [macroPad](#macropad--ascii-art-over-usb-s3-only) for how to put art on flash.

### GIF Requirements

GIF files **must be resized to 160 × 86 pixels** before copying to storage. The LVGL GIF decoder allocates an ARGB8888 canvas (width × height × 4 bytes) plus decoder state. At 160 × 86 px the whole decode costs 84 KB, measured on hardware. At full 320 × 172 px the canvas alone is 220 KB, and with decoder state that is ~249 KB against the ~221 KB largest contiguous block the ESP32-C6 can offer — it does not fit whether or not WiFi is up.

The S3 could in principle render them at native resolution, since its PSRAM gives ~8 MB of contiguous space, but the assets are shared between both boards so they stay at 160 × 86.

**To resize:** go to [https://ezgif.com/resize](https://ezgif.com/resize), upload your GIF, set Width=160 Height=86, download and copy to the SD card.

The firmware scales them 2× at render time to fill the 320 × 172 screen.

### RTC Persistence Log

The firmware automatically creates and maintains `/last_seen.txt` on the SD card. Every hour (and whenever the clock or config is saved) it appends a line like:

```
2026-03-23 07:30:00Z (3.92V)
```

On cold boot with no WiFi, the firmware reads the last line and restores the RTC to that timestamp. Logging stops automatically if battery voltage drops below 3.4V.

> **The log is SD-card-only by design**, because it grows without bound and the
> internal flash partition is not the place for that. The consequence is that a
> **cardless S3 cannot restore its clock** from a log — with no WiFi it starts
> with an unset RTC and shows `Status` instead of the date until the time is set
> from the web UI or the clock editor.

---

## config.ini Reference

```ini
[wifi]
# wifi = join the network below, NTP runs, web UI reachable on the LAN
# ap   = own open hotspot only, no internet/NTP, web UI reachable on the hotspot
# off  = airplane mode — radio fully down, no NTP, no web UI
mode = wifi
ssid = myhomewifi
password = changeme
# Reached at http://<hostname>.local in wifi mode. Must be unique on your
# network — give a second clock its own name, e.g. esp32clock2.
hostname = esp32clock

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

[usb]
# S3 only — ignored on C6 (no USB-OTG peripheral). See "USB Mode — Mouse
# Jiggler" below. Changing this outside the on-device editor still requires a
# reboot to take effect, same as changing it from the touchscreen.
# hid        = mouse only, no serial port
# hid_serial = mouse + Serial debug output (default)
# serial     = Serial debug only, no mouse
mode = hid_serial

[birthdays]
# Comma-separated list of birthdays in DD-MM-YYYY format.
# Only day and month are compared — the year is stored as reference only.
# On a matching day, both the Alarm and Timer replace their normal GIF with
# happybirthday.gif and play the Happy Birthday melody instead of the usual beeps.
# Remove this section (or leave it empty) to disable the Easter egg entirely.
dates = 01-01-1970,06-08-2017

[tennis]
high_score = 0
paddle_size = 6
ball_speed_ms = 500
ball_speed_min_ms = 200
ball_speed_change_ms = 10
paddle_speed_ms = 250
paddle_speed_min_ms = 100
paddle_speed_change_ms = 5

[letter_rain]
last_score = 0
paddle_size = 6
max_entities = 5
fall_speed_ms = 850
fall_speed_min_ms = 200
fall_speed_change_ms = 10
paddle_speed_ms = 200
paddle_speed_min_ms = 50
paddle_speed_change_ms = 5

[snake]
high_score = 0
snake_size = 3
snake_speed_ms = 600
snake_speed_min_ms = 200
snake_speed_change_ms = 5
vertical_walls = true
horizontal_walls = false
distractions = 3
next_level_score = 10

[tonequest]
high_score = 0
start_moves = 4
flash_ms = 420
gap_ms = 220
tilt_percent = 55
```

| Section | Key | Default | Description |
|---|---|---|---|
| `[wifi]` | `mode` | `wifi` | `wifi` (join network, NTP on) / `ap` (own hotspot, no NTP) / `off` (airplane mode). Legacy `enabled = true/false` files are still read automatically (`true`→`wifi`, `false`→`ap`) as long as no `mode` key is present, then rewritten to `mode` on the next save. |
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
| `[birthdays]` | `dates` | _(empty)_ | Comma-separated birthdays `DD-MM-YYYY`. On a matching day, alarm and timer use `happybirthday.gif` and the Happy Birthday melody. Up to 8 entries. Section may be omitted to disable the Easter egg. |

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
│    ◀         [ ICON ]           ▶      │
│                NAME                     │
│             description                 │
│                                         │
│         tap in or hold to exit          │
│                ● ○ ○ ○                  │
└─────────────────────────────────────────┘
```

**Tap** the centre to enter the selected item. **Long-press** anywhere to exit back to the clock.

The four items are Clock (set date+time), Timer (countdown), Alarm (wake-up), and WiFi (on/off toggle).

### Clock editor — set date and time

Opens a two-row editor pre-loaded with the current RTC values:

```
┌─────────────────────────────────────────┐
│          ▲              ▲               │
│        [ HH ]  :  [ MM ]                │  montserrat_48
│          ▼              ▼               │
│  ─────────────────────────────────────  │
│         ▲      ▲        ▲               │
│        [22]  /[Mar]/ [2026]             │  montserrat_16
│         ▼      ▼        ▼               │
│           hold to save & exit           │
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

### WiFi — mode selector

Tap the centre to open a dedicated sub-screen — a three-way selector, not an inline toggle:

```
┌─────────────────────────────────────────┐
│              WiFi mode                  │
│   ◀            WiFi             ▶       │
│           join your network             │
│                 ● ○ ○                   │
│           hold to save & exit           │
└─────────────────────────────────────────┘
```

Use **◀ ▶** to step through **WiFi** (green — joins the configured network, NTP syncs) → **AP** (cyan — own hotspot only, no internet/NTP) → **OFF** (red — airplane mode, radio fully down). Nothing is applied while browsing; **long-press** commits the selection to `config.ini` and applies it immediately — the old mode is always torn down first (radio, NTP client, web server) before the new one starts, so no listener or netif survives the switch. The carousel's WiFi row shows "Currently: WiFi / AP / OFF" colour-coded the same way.

---

## Web Configuration

The device serves a browser-based configuration UI on port 80 whenever the radio is up — reachable in **both WiFi and AP mode** (not in **Off**/airplane mode, since the radio is powered down entirely).

### Connecting

**WiFi mode** (joined to your home network): navigate to `http://esp32clock.local` or the IP shown in the Status screen WiFi popup.

> **Running two clocks on one network?** `esp32clock` is an mDNS hostname, and it
> has to be unique on the LAN — two devices answering to the same `.local` name
> collide, and you reach whichever one replies first. Give the second device its
> own name with `hostname = esp32clock2` under `[wifi]` in its `config.ini`
> (editable from the web UI, then reboot). The AP hotspot name never collides;
> it already carries a per-device MAC suffix.
>
> The value is a DNS label, so only letters, digits and hyphens survive: spaces,
> dots and underscores are converted, uppercase is folded, and an empty or
> unusable value falls back to `esp32clock`. The boot log prints what was
> actually registered, and says so explicitly if mDNS could not claim the name.

**AP mode** (chosen explicitly, or entered automatically as a credential-rescue when the WiFi password keeps getting rejected — see [Resilient WiFi connect](#resilient-wifi-connect) below): the device creates its own hotspot, named `ESP32-Clock-XXXXXX` where `XXXXXX` is a 6-hex-digit suffix derived from the device's own MAC address, so multiple units stay distinguishable. Long-press anywhere → Carousel → WiFi to open the detail popup, which shows:

```
┌───────────────────────────────────┐
│  📶  ESP32-Clock-8AF8A5           │
│  IP  192.168.4.1                  │
│  🔑  Web PIN: 483921              │
│  http://192.168.4.1               │
│                      tap to close │
└───────────────────────────────────┘
```

The hotspot is **open — no WPA2 passphrase**. Joining it needs nothing more than picking it from a WiFi list; the boot-generated PIN is not a WiFi key, it only unlocks the web UI's mutating actions (`POST /config`, `/settime`, `/reboot`). A new PIN is generated on every power cycle — reading it requires physical access to the device screen, so no fixed credential is ever embedded in the firmware.

In **Off** mode, or while a WiFi-mode connection attempt is between retries (radio intentionally powered down — see below), the detail popup shows what the radio is doing instead of a PIN/URL (e.g. "Radio down — clock runs offline" or "Radio off — retry in 47s"), since there is no web UI to reach.

### Resilient WiFi connect

Joining a network no longer uses a blocking scan-and-connect loop. `WiFi.begin()` is fired once (returns in milliseconds) and a WiFi event handler records *why* a disconnect happened, so the retry logic can tell two very different failures apart:

| Failure | Behaviour |
|---|---|
| **Wrong password** (AP actively rejects the credentials) | After 2 rejected attempts, the device stops trying that network and automatically brings up the **AP hotspot** instead, so `config.ini` can be fixed from a phone without a cable. |
| **Network unreachable** (SSID out of range, router off/rebooting) | Association simply times out (15 s). The radio is then powered fully down and the next attempt is scheduled with exponential backoff: 30 s → 60 s → 2 min → 4 min → 8 min, capped at 10 min. A router reboot is usually caught by the first 30 s retry; a device left permanently out of range costs one 15 s radio-on attempt every 10 minutes instead of scanning forever. |

Radio is powered off completely between retries (not just idle) to save battery — an associated-but-idle radio still draws roughly 20 mA, a scanning one far more.

### Web UI

The dark-themed page has several panels:

| Panel | Action |
|---|---|
| **PIN field** | Enter the 6-digit PIN shown on the device. All save/apply/reboot actions require a valid PIN. |
| **config.ini editor** | Textarea pre-filled with the current config. The WiFi password is masked as `••••••••` — leave it unchanged to keep the current password, or type a new value to update it. Tap **Save & Reload** to write the file and apply settings that don't need a reboot (alarm, timer, animation, timezone). WiFi/NTP changes take effect after a reboot. |
| **Bingo Cards** | Opens `/bingo` in a new tab — a printable sheet of tickets for the on-device [Bingo!](#bingo) caller. No PIN needed (read-only, nothing is saved to SD). |
| **Set date & time** | A `datetime-local` picker pre-filled with the current device time. Tap **Apply Time** to set the RTC immediately via `settimeofday()` — no reboot needed. |
| **Reboot** | Reboots the device remotely after PIN confirmation. |
| **Manage Files** | Opens `/files` — browse, download, delete and upload anything on the active storage. See [File manager](#file-manager). |
| **Download Log** | Downloads `/last_seen.txt` — no PIN needed (read-only). |

### File manager

`/files` lists **everything on whichever backend mounted at boot**, so it manages
an SD card and internal flash identically. The listing is flat and recursive —
full paths rather than browsable folders, since the tree is only `config.ini`
plus one GIF directory.

| Element | Behaviour |
|---|---|
| Storage meter | Active backend, file count, used and free KB, plus a usage bar |
| ⬇ per file | Downloads it. **PIN required** |
| ✖ per file | Deletes it, after a browser confirm. **PIN required** |
| Upload | Pick an existing folder and a file. Overwrites same-name files. **PIN required** |

**Free space is guarded three times:** the browser refuses a file bigger than
the reported free space; the server rejects the request up front if
`Content-Length` exceeds free space minus a 16 KB margin; and a running check
aborts mid-stream, closing and **deleting the partial file**. The third layer
exists because `Content-Length` describes the whole multipart body, so it is an
upper bound on the file rather than its size.

Three deliberate restrictions:

- **The listing reaches the root plus two folder levels.** That covers
  `/config.ini` and `/cruzr_emotions/*.gif` with a level to spare. The limit is a
  file-handle budget, not an arbitrary number: the walk holds one directory
  handle open per level, plus one for the file being read and one more if a GIF
  is playing on the device, against the SD library's five. Raising that limit
  would cost about 4 KB of heap per extra slot — each carries its own sector
  cache — which the C6 cannot spare. Anything nested deeper is reported in the
  page rather than silently omitted.
- **Folders are never created**, and directories cannot be deleted. Upload
  targets are chosen from a dropdown of folders that already exist, because
  FFat does not create parent directories on write.
- **Deleting and uploading are refused while a screen is open on the device**
  (HTTP 409). The GIF decoder may still hold that file open, and unlinking it
  underneath FAT risks corrupting the filesystem. Tap back to the clock first.

Nothing is protected by name — with the PIN you can delete `config.ini`. That is
recoverable: `bootstrap_config()` writes a fresh default on the next boot, though
on a cardless S3 you would have to re-enter WiFi credentials over the AP.

> **Uploading to internal flash stalls the CPU.** Writing FFat means writing the
> same SPI flash the firmware executes from, which briefly disables the
> instruction cache. Expect the display to stutter during an S3 upload. Uploads
> to an SD card are unaffected.

### Printable Bingo Tickets

`GET /bingo` serves a self-contained, printable ticket sheet to pair with the on-device [Bingo!](#bingo) caller — no PIN required, and nothing is written to the SD card. Layout and ticket generation run entirely client-side in the browser; the ESP32 just streams one static page out of flash.

- Tickets are **UK/housie style**, matching the 1–90 caller: 3 rows × 9 columns, 15 numbers per ticket, exactly 5 per row, each column restricted to its own decade (column 1 = 1–9 … column 9 = 80–90).
- A fresh random set is generated every time the page loads. A **New cards** button re-rolls without touching the device at all.
- Choose 4, 6, or 8 tickets per page, then use your browser's Print (Share → Print on mobile).
- The page is light-themed on screen so what you see is what you print, and each ticket avoids being split across a page break.

The exact same generator (same ticket algorithm, ported to `docs/bingo.js`) is also mirrored on the [GitHub Pages site](https://andreimagic.github.io/ESP32_C6_Touch_LCD_1_47_LVGL_Animated_Clock/bingo.html), so anyone can print a set without owning the hardware — themed to match the rest of the docs site, tickets always rendered paper-white for a faithful print preview. The two copies are independent; mirror any change to the ticket-generation rules in both `BINGO_PAGE` (the `.ino`) and `docs/bingo.js`.

### Security model

- The AP hotspot is **deliberately open** (no WPA2 passphrase) — joining it is meant to be frictionless, since it exists purely to reach the web UI, not to protect a network
- The PIN's job is narrower and different: it authorises the web UI's *mutating* actions only, not joining the hotspot
- The PIN is not persisted — it is regenerated at every boot and never written to storage
- **The PIN is not a secret in transit.** The web UI is plain HTTP, so it travels in cleartext; it is also printed to serial at boot (`[AP] PIN generated:`) and shown on the device's status screen by design. It gates casual tampering by someone within WiFi range, and nothing stronger
- Downloads and uploads carry the PIN in the **query string**, so it also lands in browser history. This is forced rather than chosen: a download is a plain link, and for uploads the multipart body is not parsed into `arg()` until after the whole file has streamed — a form-field PIN could only be checked once the file was already written to flash
- The WiFi password is never transmitted to the browser (masked on GET, re-injected server-side on POST if unchanged). **Downloading `config.ini` from the file manager bypasses that masking**, which is exactly why downloads require the PIN
- All mutating routes (`POST /config`, `POST /settime`, `POST /reboot`, `POST /files/delete`, `POST /files/upload`) and `GET /files/get` return HTTP 403 if the PIN is wrong or absent
- Every web-supplied path is validated before touching the filesystem — anything containing `..`, a backslash, `//`, a quote or a control character is rejected, and paths must be rooted at `/`. This is the file manager's only security boundary, so it deliberately errs strict
- A fresh PIN on every boot means stealing a previous PIN is useless
- Anyone on the open hotspot can still read the config page unauthenticated (as before, over the LAN, and only reachable by being physically close enough to see the AP), but cannot change anything without the PIN shown on the device screen

---

## Sub-screens

### Analog Clock (upper-right tap)
Top-right corner shows an Analog clock, view stays opened and refreshes every minute to display the correct time. Clicking on it will return  to the regular Time view. A filled sector centered on the clock center that starts at the top of the hour (12 o’clock) and sweeps clockwise to the current minute position, visually like a pie chart showing elapsed minutes in the current hour.

**Long-press** on the open analog clock (S3 only) opens the [USB carousel](#usb-mode--mouse-jiggler-s3-only), which holds **USB Mode** and **[macroPad](#macropad--ascii-art-over-usb-s3-only)**.

### USB Mode — Mouse Jiggler (S3 only)
Reached by a long-press on the open analog clock (above). Not present on the C6 build at all — the C6 has no USB-OTG peripheral, only USB-Serial-JTAG, which cannot present USB HID, so `BOARD_HAS_USB_HID` compiles the whole feature out rather than merely hiding it.

A two-item carousel, styled like the [Carousel Settings Menu](#carousel-settings-menu) — left/right to page between **USB Mode** and **macroPad**, tap to enter, hold to exit. USB Mode opens a 3-way picker (left/right to cycle, hold to save & exit — same interaction as the WiFi mode editor):

| Mode | Host sees | Serial debug output |
|---|---|---|
| **HID** | mouse **and keyboard**, no serial port | none |
| **HID + Serial** *(default)* | mouse **and keyboard**, plus a serial port | yes |
| **Serial** | serial port only, no mouse or keyboard | yes |

Both HID interfaces register on one shared descriptor, so the host still enumerates a single composite device on a single port. The keyboard is what [macroPad](#macropad--ascii-art-over-usb-s3-only) types with; it is idle unless a script is running.

Changing the mode reboots the device — USB can't swap what it's presenting to the host without a full re-enumeration, so a change is saved and applied on the next boot rather than live. Persisted as `[usb] mode` in `config.ini` (`hid` / `hid_serial` / `serial`).

**Mouse jiggler.** In any HID-enabled mode, opening the smile GIF (top-left tap, see [below](#gif-animations-upper-taps)) arms a timer that, after a 5-second delay, starts sending small randomised relative mouse movements — enough to keep a PC from going idle/locking, without visibly disrupting anything. Each nudge is sent out and then replayed backwards along the same step magnitudes, so the raw movement sums to zero and host pointer acceleration cancels between the two legs. (With the pointer already against a screen edge the outbound leg is clamped by the host and the return leg is not, so a cursor parked in a corner can still shift.) It stops the moment the GIF overlay closes: tapping back to the clock, or long-pressing into the [math gateway](#math-challenge)/apps carousel.

**Recovery hatch.** If you pick `HID` and want Serial back for reflashing, hold the **BOOT** button through power-up — the device boots Serial-only for that boot only, without touching the saved mode. (Assumes BOOT is wired to GPIO0, standard for ESP32-S3 dev boards; the C6 variant of this board wires BOOT to GPIO9 instead, so this hasn't been taken for granted — see `board_config.h`.)

### macroPad — ASCII art over USB (S3 only)
The second item on the USB carousel. The clock introduces itself to the computer as a USB keyboard and *types* — one keystroke at a time, at human speed — a text file stored on the device into whatever window has focus.

The point is that you can watch it happen. Open a text editor, tap a picture on the clock, and something the size of a matchbox starts drawing a penguin into your window by pressing keys. It is the same trick every USB keyboard performs, taken apart and made visible, and it is a good first answer to *"how does the computer know what I typed?"* — no toolchain, no IDE, nothing to install. Write a new `.art` file, drop it in `/scripts`, and it shows up in the menu.

Three samples ship in `sd_card_root/scripts/`: **`ASCII_house.art`**, **`ASCII_hut.art`** and **`ASCII_penguin.art`**. Copy the folder to the card, or upload the files through the [file manager](#file-manager) on a cardless board.

**Where scripts live.** `/scripts` on whichever backend is active (SD card, or internal flash on a cardless S3). The directory is created automatically at boot, so the [file manager](#file-manager) always has somewhere to upload to.

> **Scripts do not migrate from card to internal flash.** Boot provisioning mirrors GIFs and `config.ini` only, so pulling the card out takes the art with it. To put art on flash: **remove the card**, boot from flash, then upload the files at `/files`. Uploading while a card is inserted writes to the *card*, since the file manager follows whichever backend is live — so with the card still in, you would be filling the storage you are about to remove. There is **no extension filter** — every file in the directory is listed, so `.txt`, `.art` and extensionless files all work. Subdirectories are ignored; the listing is one level deep and caps at 24 entries, showing `4/24+` when more are present.

**Running one.** Tap a script to arm a 3-second countdown, then click into the target window on the host — the device cannot know what has focus, which is the whole reason for the delay. Tapping anywhere during the countdown cancels it. Once typing starts, a progress bar and a `N written · M left` readout track position in the file, and **tapping the screen stops it part-way**.

**Line endings and indentation.** Only the trailing `\r`/`\n` is stripped from each line; leading whitespace is preserved exactly. That matters for ASCII art, where the indentation *is* the picture.

**Choose the target window carefully.** Editors that auto-indent or reformat as you type — VS Code and most IDEs — will mangle careful ASCII spacing on their own, no matter what the device sends. Use a plain text target for anything where the alignment matters: Notepad, `nano`, a browser textarea, or **vim** (verified — in insert mode it does not reflow what arrives; `:set paste` disables `autoindent` outright if your config needs it).

**No keyboard in Serial mode.** A board booted in the `Serial` persona has no keyboard interface at all, so macroPad says so rather than counting down to nothing. Switch to `HID` or `HID + Serial` and reboot.

### Status (lower-left tap)
Title shows today's date (e.g. `Mon 23 Mar 2026`) when the RTC holds a valid time, falling back to `Status` on a fresh unconfigured boot.

**Brightness is adjusted from this screen in 10% steps, two ways:**

- **Swipe left or right** across the screen — decrease / increase. Works on every board.
- **Tilt the device left or right** — same steps, same clamping. Requires an IMU, so C6 only.

The row itself tells you which are available: `(tilt or swipe)` where an IMU
answered at boot, `(swipe to adjust)` where it did not. A swipe is one 10% step —
swipe again for the next. Swiping does **not** close the screen; a tap does.

The WiFi row reflects the live radio state: connected SSID (green), `Connecting...` (amber), `Off (airplane)` (grey) in Off mode, `Retry in Ns` (counting down) while backed off between STA attempts, or `Failed (check password)` after repeated credential rejections. The NTP row is context-aware too — `NTP: synced` / `NTP: not synced` in WiFi mode, `NTP: n/a (AP mode)` in AP mode, and `NTP: off` in Off mode, so an unsynced clock never looks like a fault when the mode itself rules NTP out.

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

> **C6 only.** With no IMU the overlay still opens on upper-left tap and plays
> `cruzr_smile.gif`, but it never cycles, so `cruzr_sleep`, `cruzr_sad` and
> `cruzr_joy` are unreachable on the S3. Tap-to-cycle is on the [Roadmap](#roadmap).

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

Nine games plus a sounds toggle, navigated with **◀ ▶**. **Tap** to enter, **long-press** to go back.

> **On a board with no IMU, four of the nine are hidden.** Tennis Letters,
> Letters Rain, Snake Letters and ToneQuest steer *only* by tilt, so rather than
> ship them unplayable the carousel skips them in both directions, leaving six
> reachable entries. A persisted carousel position pointing at a hidden app is
> normalised when the carousel is built, so it can never strand you on an
> unreachable entry.
>
> The position dots below the title are drawn one per *reachable* entry and the
> row is re-centred on its own width, so on an S3 you get six dots rather than
> ten with four that can never light up.

```
┌─────────────────────────────────────────┐
│                                         │
│    ◀     Rock Paper Scissors    ▶      │
│        An interactive ASCII Game        │
│                                         │
│      tap to play  .  hold to exit       │
│              ● ○ ○ ○ ○                  │
└─────────────────────────────────────────┘
```

#### Rock Paper Scissors

Tap to start. The device plays an animated countdown in ASCII art:

```
Ready? → UP "3" → DOWN "3" ♪ → UP "2" → DOWN "2" ♪ → UP "1" → DOWN "1" ♪ → GO! ♪♪
```

Each step is exactly 250 ms. At GO! the CPU's hand is revealed — play against it with your own hand. Tap, **shake**, or **hard-tilt** the device to play again, long-press to exit.

#### Rolling Dice

Tap to start. Three animated rolling frames play at 250 ms each (one beep per frame), then the final face (1–6) is revealed with a high tone. Tap, **shake**, or **hard-tilt** to re-roll.

#### Gyro shake/tilt trigger (RPS & Dice)

While a game is active, the IMU is polled every 150 ms. Two gestures restart the game:

| Gesture | Axis | Threshold |
|---|---|---|
| **Shake** (any direction) | ΔaccelZ between consecutive samples | > 1.8 g |
| **Hard side tilt** | accelY absolute | > 1.0 g |

Both trigger the same `app_screen_start()` as tapping the screen. The watcher starts automatically when RPS or Dice begins and stops when leaving the game.

#### Flip a Coin

Tap anywhere to flip. Instant result with ASCII coin art (heads / tails) and a high-tone beep. Tap to flip again.

#### Metronome

A full-screen BPM metronome driven by **hardware ESP32 timer** for sample-accurate timing — not affected by WiFi polling or LVGL task load.

```
┌─────────────────────────────────────────┐
│  ──────────●──────────  120  BPM        │  ← slider + value
│  [ -1 ]   [ +1 ]   [    START    ]      │  ← buttons
│     ●  ○  ○  ○                          │  ← beat dots
│    [ 2/4 ]  [ 3/4 ]  [ 4/4 ]            │  ← time signature
└─────────────────────────────────────────┘
```

| Control | Action |
|---|---|
| **Slider** | Drag to set BPM (60–240). Live update — changes tempo while running. |
| **−1 / +1 buttons** | Fine-tune BPM one step at a time |
| **START / STOP** | Toggle the metronome on/off |
| **2/4, 3/4, 4/4 tabs** | Change time signature; rebuilds the beat-dot row immediately |
| **Beat dots** | Light up in sync with each beat: 🔴 red for the downbeat (beat 1), 🟢 green for weak beats |

**Tones:** downbeat accent at 1800 Hz, weak beats at 900 Hz, each 25 ms long. The metronome always sounds regardless of the `[menu] sounds` toggle — it is a musical tool, not a game effect.

**Implementation:** `metro_hw_beat_cb()` runs in the ESP32 hardware timer task — it writes directly to `ledcChangeFrequency()` and sets a volatile flag. An LVGL 20 ms poll timer (`metro_dot_poll_cb`) reads the flag and updates the dot colours, keeping all UI work on the LVGL thread.

#### Tennis Letters

A Breakout-style ASCII game where the ball is a cycling letter (a→z).

- **Gameplay:** Use the device's **Y-axis tilt** to move the paddle (`___`) and catch the ball.
- **Scoring:** Each catch increments the score and advances the letter. Completing a full alphabet (26 letters) plays a success tune.
- **Game Over:** Losing the ball ends the game. High scores are persisted to `config.ini`.
- **Controls:** Tilt left/right to move; tap the game-over popup to restart, long-press to exit.
- **Configuration:** The `config.ini` file includes various configurable variables to increase the challenge.

#### Letters Rain

An ASCII falling-letters game. Letters and modifiers descend in separate waves — a letter wave (target + decoys) enters first, followed by a modifier wave ( `+` / `-` / `*` ) 3–5 rows behind.

- Use the device's **Y-axis tilt** to move the paddle (`___`) and catch the ball.
- Catch the **target letter** (shown capitalised in the status bar, A→Z) to score. The remaining decoys clear and a fresh letter wave spawns immediately.
- Catching a **wrong letter** shrinks the paddle by 1.
- Catching `+` enlarges the paddle (max 10); `-` shrinks it (min 3).
- Catching `*` (rare — approx every 20 waves) instantly restores the paddle to its configured default size.
- If the **target letter falls** through without being caught, the game ends and the last score is saved.
- Catch all 26 letters to win. The status bar shows ✓ on completion.

Each successive target spawns within 5–15 columns of the previous one, keeping the action in a natural zone. Speed increases with every correct catch. Last score is persisted to `config.ini`.

#### Snake Letters
Classic snake ASCII game. Steer using tilt to eat the alphabet (a-z) in order. Avoid "distraction" letters that end the game instantly. Look out for modifiers: `-` shrinks the snake, and `/` halves its length. High scores are persisted to `config.ini`.

#### Bingo!

An on-device number caller for playing classic 1–90 bingo with printed tickets (see [Printable Bingo Tickets](#printable-bingo-tickets) below).

- A large circle in the centre of the screen shows the current ball; a status bar below shows `Previous: X` (left) and `Numbers left: Y` (right).
- **Tap** anywhere, or **tilt left/right**, to draw the next ball.
- Each draw plays a short cycle-and-reveal animation — three amber "peek" frames from the remaining pool followed by the real number in white — with a `Bip-Bip-Bip-Bop` buzzer pattern (silent if `[menu] sounds = false`).
- **Long-press inside the circle** opens a history popup listing every number called so far, in call order. Tap the popup to return to the game; long-press it to exit to the carousel.
- Numbers are drawn from a fresh Fisher-Yates shuffle of 1–90 at the start of each game, so there are never any repeats. Calling all 90 numbers ("full house") plays a success tune.
- **Long-press anywhere outside the circle** exits to the carousel.

#### ToneQuest

A Simon-says tone-memory game, ported from the [Arduino original](https://github.com/andreimagic/ToneQuest_Game) where a joystick picked the directions and four LEDs echoed them. Here the joystick is the IMU and the LEDs are four "sunset" domes rising from the screen edges — but the direction→tone table is the original one, note for note.

```
┌──────────── ▄▄▄▄▄ ─────────────┐   ← UP dome (amber)
│ ▌                            ▐ │
│ ▌LEFT          ○      RIGHT  ▐ │   ← target ring + ball
│ ▌(green)              (blue) ▐ │
├──────────── ▀▀▀▀▀ ─────────────┤   ← DOWN dome (rose)
│ Level: 3                Best: 7│   ← status bar
└────────────────────────────────┘
```

- **Level the device to begin.** The ball is a bubble level: tilt moves it, and the sub-note reads `level the device` until you hold it inside the centre ring for 700 ms. Every new game starts here.
- **Watch.** The sequence plays back — each step lights its edge as a semicircle that fades out like a sunset, and sounds that edge's tone. Level 1 is four moves.
- **Repeat it.** Roll the ball into each wall in the order just shown. The ball has to come back near the centre between moves (the original joystick's spring return), which is also what stops one long sweep from registering two edges.
- **Advance.** A clean sequence plays the success melody, adds one move and starts the next level. The pattern is drawn once per game and only ever revealed a prefix at a time, so level N is always level N−1 plus one new move.
- **Or don't.** A wrong wall plays the failure tune and opens a popup with the level reached and the best ever. Tap it to play again — back to levelling the device — or long-press to exit.
- **The game never ends.** There is no win state; the score *is* the level you reach. Best is persisted to `config.ini` under `[tonequest] high_score` and shown on the carousel card once it is above zero.

| Direction | Tone | Dome |
|---|---|---|
| **UP** | D4 (294 Hz) | Amber |
| **DOWN** | C4 (262 Hz) | Rose |
| **LEFT** | E4 (330 Hz) | Green |
| **RIGHT** | F4 (349 Hz) | Blue |

**Tuning** — `[tonequest]` in `config.ini`:

| Key | Default | Meaning |
|---|---|---|
| `high_score` | `0` | Highest level completed. Written by the game. |
| `start_moves` | `4` | Sequence length at level 1 (1–16). |
| `flash_ms` | `420` | How long each edge stays lit, and its tone sounds, during playback (80–2000). |
| `gap_ms` | `220` | Silence between playback steps (20–2000). |
| `tilt_percent` | `55` | Percent of 1 g of tilt that pins the ball against a wall (30–100). Lower is twitchier and needs less wrist; higher demands a firmer tilt. |

**Implementation note:** the domes are full circles inside wrapper objects sized to exactly the half that should be visible — LVGL clips children to their parent, so the other half simply never draws. The gradient runs the whole circle, which puts the wall on the 50 % white/hue mix and the crown on the pure hue: white-hot at the horizon, saturated at the top, like a setting sun.

#### Sounds toggle

The last carousel item — tenth on a board with an IMU, sixth on one without, since the three tilt-steered games are dropped from the carousel at runtime where no accelerometer answers. Tap to mute/unmute all apps menu and game audio. The setting is saved to `config.ini` under `[menu] sounds`. This does **not** affect alarm, timer, or metronome sounds.

---

## Power Off and Wake

### Powering off
Long-press the battery screen to open the shutdown popup. After the countdown (or immediately if not cancelled) the device enters deep sleep drawing ~10 µA.

### Waking up
Press the **RESET** button on the device body. This always causes a clean reboot through the full boot sequence.
Low-battery gate code will check at startup if the battery is > 10%.

> The BOOT button on this board is wired to GPIO9, which is not a low-power GPIO on the ESP32-C6 and cannot trigger a wake-from-deep-sleep interrupt. RESET is the reliable wake method.

### Alarm auto-wake and NTP guard

If an alarm is set **and `[wifi] mode = wifi`**, the firmware calculates the sleep duration and wakes automatically:

- **Alarm > 5 min away:** wakes 5 minutes early so WiFi and NTP have time to sync before the alarm fires
- **Alarm ≤ 5 min away:** wakes 30 seconds early (no time for NTP; relies on RTC drift being minor)

In **AP** or **Off** mode there is no uplink to sync with, so the early-wake margin is skipped entirely — the device always wakes just 30 seconds before the alarm, saving the battery that would otherwise be spent sitting idle for up to 4.5 minutes waiting on a sync that can never happen.

At alarm time, the following logic applies:

| Condition | Behaviour |
|---|---|
| `[wifi] mode = ap` or `off`, fresh wake (uptime ≤ 5 min), never synced | Alarm fires immediately with a **drift warning** instead of the GIF (see below) — there is no correction source, so the RTC's drift since last sync/set is unbounded |
| `[wifi] mode = ap` or `off`, uptime > 5 min (device already running a while) | Alarm fires normally — the one-time drift warning only applies to a fresh deep-sleep wake, not every subsequent firing |
| Device uptime > 5 min (WiFi mode) | Alarm fires normally (was already running, time assumed reliable) |
| Uptime ≤ 5 min, NTP synced | Alarm fires normally (accurate time confirmed) |
| Uptime ≤ 5 min, NTP still possible (WiFi mode) but not yet synced | Alarm held — checked every minute |
| NTP synced while held | Alarm fires immediately |
| NTP ruled out while held (mode switched away from WiFi mid-hold) | Alarm fires immediately with the drift warning, rather than continuing to wait |
| 15 min elapsed, NTP never synced (WiFi mode) | Drift warning: "HELLO! / NTP not in sync / Check the time!" + buzzer |

The drift-warning overlay always sounds the buzzer exactly like a normal alarm — it is never silently skipped, only the GIF is swapped for text. The message differs by cause: `"NTP disabled - time may have drifted. Check a clock nearby!"` when the mode itself rules NTP out (AP/Off), versus the original `"NTP not in sync / Check the time!"` when WiFi mode expected a sync that never arrived.

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

### Birthday override

When today matches a `[birthdays]` entry, the standard alarm/timer beep pattern is **replaced** by a single non-blocking play of the **Happy Birthday to You** melody (~11 seconds, C major, ~90 BPM). The melody is driven by an LVGL timer (no blocking delays) and fades the overlay automatically when it finishes — exactly like a finite beep sequence.

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
| ToneQuest edge (playback and input) | D4 / C4 / E4 / F4 by direction, `flash_ms` long |
| ToneQuest level cleared | 12-note ascending melody |
| ToneQuest wrong edge | Low two-note failure tune (G4 → C4) |
| Sounds toggle turned ON | High C6 tone (confirmation) |

### Metronome tones (always active, ignores `[menu] sounds`)

| Beat type | Frequency | Duration |
|---|---|---|
| Downbeat (beat 1) | 1800 Hz | 25 ms |
| Weak beat | 900 Hz | 25 ms |

---

## Birthday Easter Egg 🎂

Add a `[birthdays]` section to `config.ini` to unlock a hidden birthday greeting mode.

```ini
[birthdays]
dates = 06-08-2017,20-08-1989,07-09-2017,21-03-1989
```

**How it works:**
- Dates are written in `DD-MM-YYYY` format, comma-separated, up to 8 entries.
- Only the **day and month** are compared at runtime. The year is kept in the file purely as a human-readable reference (e.g. to remember who was born in that year).
- At boot and at every alarm/timer trigger, the firmware calls `is_birthday_today()`, which reads today's local date from the RTC and checks it against every entry.
- If there is a match, **both the Alarm Clock and the Timer Alarm** automatically switch to:
  - 🎂 `happybirthday.gif` — shown fullscreen instead of `alarm_animation.gif` / `timer_animation.gif`
  - 🎵 **Happy Birthday to You** melody played on the buzzer (~11 s, C major, ~90 BPM, fully non-blocking via LVGL timer) instead of the standard 4-beep pattern
- The melody fades the overlay automatically when it finishes, just like a finite beep sequence.
- If `[birthdays]` is absent or `dates` is empty, the feature is completely inactive — no performance overhead.

**SD card:** place `happybirthday.gif` (160 × 86 px, same rules as all other GIFs) at:
```
/cruzr_emotions/happybirthday.gif
```

---

## Build & Flash

There are two routes. Flashing a prebuilt release takes a couple of minutes and
needs no toolchain; building from source is only necessary if you want to change
the firmware.

### Option A — Flash a prebuilt release (no toolchain)

Every release ships a complete image per board — bootloader, partition table and
application in one file:

| Board | Full image |
|---|---|
| ESP32-C6 Touch LCD 1.47 | `firmware-<version>-esp32c6-full.bin` |
| ESP32-S3 Touch LCD 1.47 | `firmware-<version>-esp32s3-full.bin` |

> **The images are chip-specific and not interchangeable.** Flashing the C6 image
> to an S3 (or the reverse) produces a board that does not boot. Check the `esp32c6`
> / `esp32s3` in the filename before you flash.

1. Download the `-full.bin` **for your board** from
   [Releases](https://github.com/andreimagic/ESP32_C6_Touch_LCD_1_47_LVGL_Animated_Clock/releases).
2. Open **[Espressif's ESP Launchpad](https://espressif.github.io/esp-launchpad/)**
   in **Chrome or Edge**. It flashes over WebSerial, which Firefox and Safari do
   not support.
3. Open the **DIY** tab, connect the board over USB, click **Connect** and pick
   its serial port.
4. Select the `.bin` file and set the flash address to **`0x0`**.
5. Click **Program**. The board reboots into the new firmware when it finishes.

> **The address is `0x0`, not `0x1000`.** Both the ESP32-C6 and the ESP32-S3
> place their bootloader at zero, and using `0x1000` produces an image that will
> not boot. `0x1000` belongs to the original ESP32 and the ESP32-S2 only — this is
> **not** an Xtensa-versus-RISC-V distinction, as the S3 is Xtensa and still uses
> `0x0`.

You do not need to erase the flash first — the image replaces the bootloader,
partition table and app in a single write.

Your settings survive it. On a card they live in `/config.ini`, outside flash
entirely. On a cardless S3 they sit on the internal FFat partition at
`0x610000`, and the release image deliberately **ends just after the app**
(around `0x1ba000`), so nothing above that offset is touched.

> **What does still erase FFat**, taking a cardless S3's config and GIFs with it:
> **Erase All Flash Before Sketch Upload** (keep it *Disabled*), **Burn
> Bootloader**, and any image that spans the whole chip.
>
> That last one is a live trap for anyone building locally: the ESP32 core pads
> its own `merged.bin` out to the full flash size with `--pad-to-size`, filling
> everything above the app with `0xFF`. Flashing that file at `0x0` blanks the
> FFat partition. The release pipeline trims it at the end of the app before
> publishing, which is why the published `-full.bin` is safe and a raw local
> `merged.bin` is not.

If you prefer a command line, the same images work with
[esptool](https://github.com/espressif/esptool). Only the `--chip` argument and
the filename change; the address is `0x0` for both:

```bash
esptool --chip esp32c6 write-flash 0x0 firmware-<version>-esp32c6-full.bin
```

```bash
esptool --chip esp32s3 write-flash 0x0 firmware-<version>-esp32s3-full.bin
```

> Releases also contain `firmware-<version>-<chip>-app.bin`, the application
> partition on its own, for reflashing over an existing install at `0x10000`.
> **Use esptool for that one, not a browser flasher** — esptool writes exactly the
> offset given and erases only the sectors it touches, whereas a browser tool
> makes it easy to erase the chip or write to the wrong address, either of which
> removes the bootloader and leaves the board unable to boot. It is only valid on
> a board already running that same partition scheme. Recover by flashing the
> matching `-full.bin` at `0x0` again.

### Option B — Build from source

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

4. Select and configure the board — **all settings below are mandatory**. Use
   the column for the board you have:

   | Setting | ESP32-C6 | ESP32-S3 |
   |---|---|---|
   | **Board** | `ESP32C6 Dev Module` | `ESP32S3 Dev Module` |
   | **USB CDC On Boot** | `Enabled` | **`Disabled`** |
   | **USB Mode** | — | **`USB-OTG (TinyUSB)`** |
   | **PSRAM** | — | **`OPI PSRAM`** |
   | **Flash Size** | `8MB (64Mb)` | `16MB (128Mb)` |
   | **Partition Scheme** | `8MB with spiffs (3MB APP/1.5MB SPIFFS)` | `16M Flash (3MB APP/9.9MB FATFS)` |
   | CPU Frequency | `160MHz (WiFi)` _(recommended)_ | `240MHz (WiFi)` |
   | Flash Frequency | `80MHz` | _(no such menu — see below)_ |
   | Flash Mode | `QIO` | `QIO 80MHz` |
   | Upload Speed | `921600` | `921600` |
   | JTAG Adapter | `Disabled` | `Disabled` |
   | Zigbee Mode | `Disabled` | `Disabled` |
   | Core Debug Level | `None` | `None` |
   | **Erase All Flash Before Upload** | `Disabled` | `Disabled` |

   > **USB CDC On Boot must be Enabled on the C6** — without it the Serial Monitor will not receive any output and the device may not be recognised on the port.
   > **Flash Size and Partition Scheme must match** — the 3MB APP partition is required to fit the firmware with LVGL v9 and all libraries.

   Four S3-specific traps, all of which change the produced binary:

   > **USB CDC On Boot must be `Disabled` on the S3 — the opposite of the C6.**
   > The USB-persona feature (see [USB mode / mouse jiggler](#usb-mode--mouse-jiggler-s3-only))
   > brings its own CDC/HID interfaces up at runtime from the touchscreen menu's
   > choice; if this were Enabled, the core would auto-start a CDC interface
   > before `setup()` even runs, so a "HID only" choice could never truly hide
   > the serial port from the host. Serial output still works normally in the
   > default HID+Serial persona — it just starts a moment later, from inside
   > the sketch instead of before it.
   >
   > **USB Mode must be `USB-OTG (TinyUSB)`, not `Hardware CDC and JTAG`.**
   > Waveshare's setup page pictures Hardware CDC, but the working configuration
   > is USB-OTG. This setting feeds `build.usb_mode`, so the two genuinely
   > produce different firmware — it is not a cosmetic serial-port preference.
   >
   > **PSRAM must be `OPI PSRAM`.** The GIF decoder allocates from
   > `MALLOC_CAP_8BIT`, which only includes the 8 MB PSRAM when it is enabled.
   > Leave it Disabled and the S3 ends up with *less* usable headroom than the C6.
   >
   > **The S3 has no Flash Frequency menu.** It is folded into the Flash Mode
   > label, so `QIO 80MHz` is one choice rather than two settings.

   > **Keep Erase All Flash Before Sketch Upload Disabled** on the S3. A normal
   > upload writes only the bootloader, partition table and app (up to
   > ~`0x1a8fff`), leaving the FFat partition at `0x610000` intact. Erasing wipes
   > the config and GIFs stored there, which on a cardless board is everything.

5. Set the correct **Port** (e.g. `COM3` on Windows, `/dev/ttyUSB0` on Linux/macOS)
6. Install all libraries listed in [Software Dependencies](#software-dependencies)
7. No `lv_conf.h` edit is needed — see [lv_conf.h Settings](#lv_confh-settings)
8. Place all custom font `.c` files in the sketch folder (see [Custom Fonts](#custom-fonts))
9. Prepare the SD card as described in [SD Card Setup](#sd-card-setup)
10. Open `ESP32_C6_Touch_LCD_1_47_LVGL_Animated_Clock.ino`, click **Upload**
11. Open Serial Monitor at **115200 baud** to watch the boot log

### Continuous integration and releases

Three workflows, all driven from one shared target list:

| File | Trigger | Does |
|---|---|---|
| [`board-targets.json`](.github/board-targets.json) | — | **The single source of truth.** One object per board: FQBN, chip, and pinned library versions |
| `build.yml` | PR / push to `development`, `main` | Compiles every target and reports flash + RAM usage per board |
| `version-check.yml` | PR | Fails the PR unless `FW_VERSION` is bumped above the base branch |
| `release.yml` | push to `main` | If `FW_VERSION` isn't tagged yet: builds every target, tags, and publishes one release carrying a binary set per board |

Both `build.yml` and `release.yml` read their matrix from `board-targets.json`
via `fromJSON`, so a board cannot be validated by CI with one set of settings and
then released with another. **Adding a board is one JSON object** — no workflow
edits, matching the one `#elif` block it takes in `board_config.h`.

`release.yml` creates the tag only after *every* board has compiled
(`fail-fast: true`, and the tag lives in a job that `needs` all of them), so a
failure on one board can never leave a tag with no release attached. A final
guard counts the collected `-full.bin` images against the number of entries in
`board-targets.json` before publishing.

### Expected Boot Log

```
========== BOOT ==========
[BOOT] ESP32-C6-Touch-LCD-1.47  fw v3.3.0
[BOOT] PSRAM no  internal FS no
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
[5] Mounting storage...
    SD: CS=4  SCK=1  MISO=3  MOSI=2  speed=4MHz
    SD.begin() returned: true
    SD mounted OK — type: SD  size: 244 MB
    Active storage: SD card
[CFG] Loading /config.ini...
[CFG]   wifi.mode          = wifi
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
[WiFi] Mode WiFi — joining configured network
[WiFi] STA attempt 1 -> SSID 'myhomewifi'
[BAT] Boot check: 4.11V = 91%
[8] Building UI...
========== SETUP DONE ==========
```

That is a C6 with a card inserted. An S3 differs in four places:

```
[BOOT] ESP32-S3-Touch-LCD-1.47  fw v3.3.0
[BOOT] PSRAM yes  internal FS yes (FFat)
...
[4b] No IMU on this board — tilt control disabled.
```

and, when no card is present, step [5] falls through to internal flash instead of
failing (sizes as reported by `FFat`; the partition is 10,354,688 bytes):

```
[5] Mounting storage...
    SD: CS=14  SCK=16  MISO=17  MOSI=15  speed=4MHz
    SD.begin() returned: false
    Retrying at 1 MHz...
    Retry returned: false
    No SD card (not inserted, unformatted, or wiring).
    Falling back to internal flash (FFat)...
    FFat mounted OK — 10112 KB total, <free> KB free
    Active storage: Internal flash (FFat)
```

On a virgin board the `FFat.begin(true)` in that path formats the partition once,
which takes a few seconds — expect a one-off pause on the very first cardless boot.

---

## Troubleshooting

| Symptom | Likely cause | Fix |
|---|---|---|
| Black screen after boot | Display init failed | Check SPI wiring; confirm `gfx->begin() OK` in serial log |
| **S3:** compile error `unsupported target` | Wrong board selected in the IDE | `board_config.h` only knows ESP32-C6 and ESP32-S3; pick `ESP32S3 Dev Module` |
| **S3:** no serial output at all | `USB Mode` set to `Hardware CDC and JTAG` | Set **USB Mode = `USB-OTG (TinyUSB)`** and re-upload — see [Option B](#option-b--build-from-source) |
| **S3:** `PSRAM not enabled` on screen, or `[GIF] ... PSRAM is not enabled` in the serial log | `PSRAM` left `Disabled` | Set **PSRAM = `OPI PSRAM`** and re-upload; the boot log must then say `PSRAM yes`. Do **not** resize the GIF — the asset is fine, the build was not |
| **S3:** config and GIFs vanished after upload | **Erase All Flash Before Sketch Upload** was Enabled | Keep it `Disabled`; it wipes the FFat partition holding both |
| **S3:** three games missing from the carousel | Working as intended — no IMU | Tennis Letters, Letters Rain and Snake Letters steer only by tilt |
| **S3:** linker warning `missing .note.GNU-stack section implies executable stack` | Comes from the Xtensa toolchain's own `libgcc` (`_floatdidf.o`), not this sketch | Harmless — ignore it. It appears on every S3 build with the pinned core and does not affect the firmware |
| `SD card mount failed` | Wrong MISO pin or card not FAT32 | Confirm MISO (GPIO 3 on C6, GPIO 17 on S3); reformat card as FAT32 |
| `GIF not found` | Wrong filename or path | Path is case-sensitive: `/cruzr_emotions/cruzr_smile.gif` |
| Birthday GIF not showing | `happybirthday.gif` absent or wrong date format | Place the file at `/cruzr_emotions/happybirthday.gif` (160×86 px); verify `dates` entries are `DD-MM-YYYY` |
| GIF shows but wrong size | GIF not resized | Resize to 160 × 86 px using ezgif.com/resize |
| `Not enough RAM — resize GIF to 160x86` on screen, or `[GIF] need ~84KB, only N available` in the serial log | GIF still full-size | Must be 160 × 86 px — see [GIF Requirements](#gif-requirements). **On the S3, check the `PSRAM` row above first** — the firmware only names the GIF once it has confirmed PSRAM is up |
| Clock shows `--:--` permanently | No WiFi, no log, no manual set | Set date+time via Carousel → Clock editor |
| Clock shows wrong time after RESET | Log entry is old | Set time manually or re-enable WiFi for NTP sync |
| Clock 1 hour off after DST change | Old `gmt_offset` config or missing `tz` key | Replace `gmt_offset` with `tz = CET-1CEST,M3.5.0,M10.5.0/3` in `config.ini` |
| Status shows "Status" not date | RTC not yet valid | Cold boot with no WiFi and no log; set time via Clock editor |
| Touch zones unresponsive | Touch controller not detected | Check I²C wiring on pins 18/19 |
| IMU not working | Address mismatch or wiring | Confirm `IMU_ADDRESS = 0x6B`; check serial for IMU error code |
| Alarm not firing | `enabled = false` or device was asleep | Set `enabled = true`; check boot log for wakeup cause |
| Web UI returns 403 | Wrong or missing PIN | Read the current PIN from the device screen (long-press → Carousel → WiFi) |
| Web UI not reachable in AP mode | Not connected to the device's hotspot | The AP is open (no password needed) — join `ESP32-Clock-XXXXXX` from your WiFi list (name shown on the device's WiFi detail popup), then navigate to `http://192.168.4.1` |
| Web UI not reachable in WiFi mode | mDNS not resolving | Use the IP address shown in the Status WiFi popup instead of `esp32clock.local` |
| `esp32clock.local` opens the **wrong** clock | Two devices sharing one mDNS hostname | Set a distinct `[wifi] hostname` on one of them — see [Connecting](#connecting) |
| Boot log says mDNS could not claim the hostname | The name is already taken on this network | Change `[wifi] hostname`, or reach the device by IP |
| Web UI unreachable, popup shows "Radio off — retry in Ns" | Backed off after a failed connection attempt | Normal — wait out the countdown, or power-cycle to force an immediate retry |
| Config saved but WiFi not reconnecting | WiFi/NTP changes need a reboot | Use the Reboot button in the web UI after saving |
| Date/time set via web not sticking | RTC drift before next NTP sync | Normal — NTP will correct it at next sync; set `[wifi] mode = ap` or `off` if you want the manual time to persist |
| Alarm rings twice | RTC drift + NTP correction | Fixed in v1.4.1 — NTP guard prevents double-fire |
| Drift warning at alarm time (text instead of GIF) | `[wifi] mode = ap`/`off` (no sync source), or WiFi mode where NTP did not sync within 15 min | Expected in AP/Off mode — there's nothing to sync with. In WiFi mode, check the network; the device woke 5 min early specifically to allow sync |
| WiFi keeps falling back to the AP hotspot | Password in `config.ini` is being rejected (2 failed attempts triggers the rescue) | Join the open `ESP32-Clock-XXXXXX` hotspot, open the web UI, correct the WiFi password, save, then set `[wifi] mode = wifi` again |
| Buzzer plays all tones at same pitch | Active buzzer used, or ESP32 core issue | Use a **passive** buzzer; firmware uses `ledcChangeFrequency()` for pitch control |
| Apps menu not opening | Long-pressing wrong zone | Long-press must be on the **smile GIF** (upper-left tap first, then long-press the GIF) |
| Math challenge not appearing | Smile GIF not open | Must open the smile GIF first by tapping upper-left |
| Font not found (compile error) | Custom `.c` files missing | Generate and place `montserrat_96.c`, `dejavu_mono_8.c`, `dejavu_mono_14.c`, `dejavu_mono_16.c` |
| UI stutters during games/math while reconnecting | Historical issue, pre-v2.7.1 | Fixed in v2.7.1 — `WiFi.begin()` replaced the blocking `WiFiMulti::run()` scan, so reconnect attempts no longer stall LVGL regardless of what's open |
| `last_seen.txt` not created | SD write error or low battery | Check SD card is writable FAT32; battery must be above 3.4V |
| BOOT button doesn't wake from sleep | Hardware limitation | GPIO9 is not an LP GPIO on ESP32-C6; press **RESET** instead |
| Battery % jumps on unplug | ADC reads elevated USB voltage | Normal — LiPo estimate is slightly elevated while USB powers the system |

---

## Architecture Notes

- **No blocking calls in `loop()`** — `loop()` only calls `lv_timer_handler()` + `delay(5)`. All WiFi polling, clock ticks, brightness schedules, buzzer patterns, countdown timer, and animations run as LVGL timer callbacks.
- **Board abstraction** — `board_config.h` selects on `CONFIG_IDF_TARGET_*` (never on `ARDUINO_<BOARD>` variant macros, which depend on the IDE board entry the user picks) and an unsupported target is a deliberate compile error. Beyond pins it exports capability flags — `BOARD_HAS_IMU`, `BOARD_HAS_INTERNAL_FS`, `BOARD_SD_SHARES_LCD_BUS` — and a `BOARD_NEW_LCD_BUS()` macro expanding to the correct `Arduino_DataBus` constructor, so the sketch contains no `#ifdef` per board. Adding a target is one `#elif` block.
- **Storage abstraction** — a single `fs::FS *STORAGE` pointer is resolved once at boot: the SD card if one mounts, otherwise `FFat` where `BOARD_HAS_INTERNAL_FS`. Exactly one backend is ever active and a card always wins. Every one of the 18 call sites goes through the pointer, including `lvgl_sd_open()`, which is the single chokepoint for all `GIF_*_PATH` opens — so the `S:` drive letter behaves identically whichever backend is live. `sdCardAvailable` means literally "a card is mounted"; `storageAvailable` means "some filesystem is mounted"; the two are deliberately not interchangeable.
- **Storage ↔ LVGL filesystem bridge** — a custom `lv_fs_drv_t` registered under drive letter `'S'` forwards all LVGL file operations to whichever backend `STORAGE` points at. This lets `lv_gif_set_src()` open files with the prefix `S:/` from either an SD card or internal flash. Paths are probed with `STORAGE->exists()` before being handed to LVGL, because `lv_gif_set_src()` only logs a warning on a missing file and renders nothing — indistinguishable from a hung black screen.
- **GIF memory management** — the LVGL GIF decoder needs a contiguous block for its canvas. GIFs are pre-scaled to 160×86 px, which costs **84 KB measured** end to end (55 KB canvas + ~29 KB decoder state). The C6's largest free block is ~221 KB with the radio and web server up, so that leaves ~137 KB of headroom; full 320×172 would need ~249 KB and genuinely does not fit. On the S3 the same allocation lands in PSRAM, since that is inside `MALLOC_CAP_8BIT`. The render buffer uses 20 scan lines for good throughput without exhausting RAM.
- **Bootstrapped config** — `bootstrap_config()` writes a complete default `config.ini` from PROGMEM when the active storage has none. Necessary because `save_config()` only rewrites the sections it manages, so `[clock]`, `[animation]` and `[birthdays]` would never appear and the web editor would show a blank textarea on a virgin device. The default WiFi mode is `ap` in all three places it is defined (struct, template, parser fallback), because a device with no valid config has no valid credentials either — and on internal flash the web UI is the only way to enter them.
- **DST-aware timekeeping** — `configTzTime(tz_string, ntp_server)` sets the POSIX TZ env var and starts SNTP in a single call. NTP delivers UTC; `localtime_r()` converts to correct local time including DST transitions automatically. `setenv("TZ", tz_string, 1)` is also called before WiFi starts so offline use (restore from log) is correct too.
- **RTC persistence** — `log_last_seen()` appends a timestamped voltage reading to `/last_seen.txt` every hour, on every config save, and on clock editor use. `restore_time_from_log()` reads the last entry on cold boot. With TZ set, `mktime()` converts local→UTC correctly including DST.
- **Carousel** — a full-screen LVGL modal opened by long-press. Each tap on ◀/▶ calls `lv_obj_clean()` and rebuilds the view in place. The centre zone uses `LV_EVENT_CLICKED` (not `LV_EVENT_PRESSED`) so long-press and tap are mutually exclusive — the editor never opens before the long-press exit fires.
- **Shared editor** — `open_editor()` builds the HH:MM widget for Timer and Alarm. `open_clock_editor()` builds the full two-row date+time widget. `modal_longpress_cb()` dispatches to the correct save function based on `carousel_idx`.
- **Animation priority** — `close_scheduled_gif()` forcefully tears down any scheduled overlay (cancels fade timer, deletes overlay synchronously) before alarm or timer open their GIF. Scheduled animation is also skipped entirely if the alarm fires on the same minute.
- **Apps menu** — `apps_cont` is a global LVGL object separate from `modal_cont` and `overlay_cont`.
- **ASCII games** — all art is rendered using DejaVu Mono fixed-width font via LVGL labels. RPS and Dice use LVGL timer callbacks (`rps_anim_tick_cb`, `dice_anim_tick_cb`) at 250 ms intervals for consistent animation cadence. Buzzer tones use `ledcChangeFrequency()` to switch pitch without re-attaching the PWM channel.
- **Gyro shake/tilt trigger** — `app_gyro_poll_cb()` runs as a 150 ms LVGL timer while RPS or Dice is active. It reads `accelZ` and `accelY` from the QMI8658; a Z-axis spike (|ΔaccelZ| > 1.8 g between samples) detects a physical shake, and a hard side tilt (|accelY| > 1.0 g) detects a roll gesture. Both call `app_screen_start()` identically to a screen tap. The timer is created when the game starts and deleted when leaving.
- **Metronome** — beat timing uses two `esp_timer` hardware timers (`metro_hw_beat_cb`, `metro_hw_off_cb`) running outside the LVGL loop for µs-accurate periods. The beat callback writes directly to `ledcChangeFrequency()` (safe from timer task context) and sets a volatile flag. A 20 ms LVGL poll timer (`metro_dot_poll_cb`) reads the flag and updates dot colours on the LVGL thread. Changing BPM while running stops and restarts the periodic timer with the new `60,000,000 / bpm` µs period. Changing time signature calls `metro_build_ui()` which rebuilds only the dot row, then restarts if it was running.
- **Bingo!** — `bn_shuffle()` runs a single Fisher-Yates pass over 1–90 into `bn_order[]` at game start, so every draw is just `bn_order[bn_count++]` with no "already called?" lookup ever needed. `bn_anim_tick_cb()` (250 ms LVGL timer) drives three cosmetic "peek" frames sampled from the remaining pool before revealing the real, already-fixed next number. Tap, the circle, and tilt (own 150 ms gyro timer, `bn_gyro_tick_cb`, hysteresis-latched on `accelY`) all funnel through the same `bn_start_reveal()`. The `/bingo` web route serves a static `PROGMEM` page — ticket generation and printing happen entirely client-side, so the request costs the ESP32 nothing beyond `send_P()`.
- **Emotion tilt** — `zone_ul_cb` sets `emotion_tilt_active = true` and starts `tilt_timer` after opening the smile GIF. `tilt_poll_cb` branches on this flag: in emotion mode it reads both `accelX` (forward/back) and `accelY` (left/right), determines the desired GIF path, and calls `lv_gif_set_src()` on the existing widget (retrieved from `overlay_cont` user data) only when the path changes. This swaps the animation in-place with no overlay rebuild. Both the flag and the timer are cleared by `overlay_close_event_cb`.
- **Buzzer state machine** — a single 9-step table drives both alarm and timer patterns. `buzzer_fade_after` is set by the caller for finite sequences; `buzzer_stop()` triggers `overlay_fade_and_close()` automatically after the last beep.
- **WiFi state machine** — `wifiMode` (`WM_IDLE` / `CONNECTING` / `STA` / `AP` / `RETRY` / `FAILED` / `OFF`) tracks what the radio is *actually* doing, kept deliberately separate from `cfg.wifi_mode` (what the user asked for) so a config.ini edit over the web can never desync live state. `WiFiMulti` was removed — its `run()` performed a blocking full channel scan (~1.5-3 s) on every poll; plain `WiFi.begin()` is non-blocking and the IDF drives association in its own task, so `loop()` and LVGL are never stalled by a reconnect attempt regardless of what UI is open. A `WiFi.onEvent()` handler records the disconnect reason (`wifi_last_reason`), which `wifi_reason_is_auth()` classifies as a credential rejection or not: repeated auth failures (`WIFI_AUTHFAIL_LIMIT = 2`) trigger the AP rescue, anything else backs off exponentially (`WIFI_RETRY_BASE_MS` 30 s, doubling to a `WIFI_RETRY_MAX_MS` 10 min ceiling) with the radio fully powered down between attempts via `wifi_radio_down()`. `wifi_poll_cb()` is paused outright (`lv_timer_pause`) in the AP/Off/idle steady states rather than polling a fixed interval.
- **Automation gate** — `run_daily_automation()` fires when `now > 2026-01-01` (RTC sanity check) instead of `timeSynced`, so brightness schedules, alarms, and animations all work correctly when WiFi is in AP/Off mode or the time was set manually.
- **Deep sleep & Alarm NTP guard** — `boot_millis` captured at the very start of `setup()`. In WiFi mode, wakes 5 min before alarm when > 5 min away, 30 s when close; AP/Off mode always wakes 30 s early since there is no sync to wait for (`wifi_ntp_possible()` gates this). Holds `alarm_ntp_pending` while a sync is still possible; `show_alarm_warning()` fires the buzzer with a text overlay instead of the GIF if NTP is ruled out by the mode (AP/Off) or times out after 15 min in WiFi mode — the alarm itself is never skipped or delayed, only its visual changes.
- **config.ini** — parsed once at boot with a hand-rolled INI reader (no external library). On save, `[wifi]`, `[alarm]`, `[timer]`, and `[menu]` sections are fully rewritten (using the new `mode = wifi|ap|off` key); all other sections and comments are preserved. A legacy `[wifi] enabled` key (no `mode` key present) is still read and mapped (`true`→`wifi`, `false`→`ap`) for backward compatibility with files from before v2.7.1.
- **Web configuration server** — `WebServer` on port 80, whose routes are registered exactly once (`webRoutesRegistered`) since `WebServer::on()` appends to a linked list and would leak a duplicate handler set on every AP/WiFi mode switch otherwise; `start_web_server()`/`stop_web_server()` just start/stop the listener (and `MDNS.end()`) on top of that fixed route table as the radio comes and goes. The boot-generated PIN (`ap_pin[7]`) is seeded from `esp_timer_get_time()` and authorises the web UI's mutating routes only — the AP hotspot itself (`ap_ssid`, `ESP32-Clock-XXXXXX`) is deliberately open, no WPA2 passphrase. The MAC in that SSID is read straight from eFuse via `esp_read_mac(..., ESP_MAC_WIFI_SOFTAP)` rather than `WiFi.softAPmacAddress()`, which was being called before `softAP()` had created the netif and therefore returned zeros — every device came up as `ESP32-Clock-000000`. This was a latent bug on the C6 too; it simply kept winning the race. `GET /` sends the config textarea with the WiFi password masked; `POST /config` re-injects the real password from RAM if the placeholder is unchanged, then writes to storage and calls `load_config()`. Both that route and `save_config()` write `/config.tmp` and rename it over the original — the previous `remove()`-then-`write()` left a window in which a power cut lost the config outright, which is unrecoverable on a cardless board. The editor also shows a badge naming the active backend (`SD card` or `Internal flash (FFat)`), and `/log` returns an explanatory message rather than a bare 404 when there is no card to read a log from. `POST /settime` parses `YYYY-MM-DDTHH:MM` and calls `settimeofday()` directly. `POST /reboot` calls `ESP.restart()` after flushing the HTTP response. All three POST routes return HTTP 403 on PIN mismatch.

---

## ⚠️ Disclaimer — ASCII Art

Some coin flip ASCII art displayed in the Apps Menu was sourced from [asciiart.eu/video-games/pokemon](https://www.asciiart.eu/video-games/pokemon). All Pokémon characters and names are trademarks of **The Pokémon Company International**. This project is not affiliated with, sponsored by, or endorsed by The Pokémon Company. The art is used here solely for non-commercial, personal, educational purposes.

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
| v2.0.0 | ✅ released | Analog clock view & low-power startup gate |
| v2.1.0 | ✅ released | Metronome app (60–240 BPM, hardware-timer accuracy, 2/4 3/4 4/4, beat dots); gyro shake/tilt trigger for RPS & Dice |
| v2.2.0 | ✅ released | Birthday Easter egg: `[birthdays]` in config.ini, Happy Birthday melody, `happybirthday.gif` for alarm & timer. Fixed Read/Write config.ini |
| v2.3.0 | ✅ released | PIN-protected web configuration UI — edit config, set RTC, reboot; WPA2 AP with boot-generated PIN |
| v2.4.0 | ✅ released | Tennis Letters game: Breakout-style ASCII game with tilt-controlled paddle and alphabet cycling |
| v2.5.0 | ✅ released | Letters Rain game: letters and modifiers fall in waves; catch the target letter (A→Z) with a gyro paddle while dodging wrong letters |
| v2.6.0 | ✅ released | Snake Letters game: classic snake with alphabet targets, distraction letters, and length modifiers |
| v2.6.1 | ✅ released | Bugfix: Clock editor touch zones re-derived from drawn geometry, fixing dead strips and drift between the HH/mm/date fields; Metronome BPM label right-aligned so digits grow without overlapping the "BPM" unit or the slider |
| v2.7.0 | ✅ released | Bingo! — on-device 1–90 number caller (tap/tilt to draw, cycle-and-reveal animation, call-history popup); web-served printable UK/housie ticket sheets at `/bingo`, linked from the Web Configuration page; same generator mirrored on the docs site at `/bingo.html` |
| v2.7.1 | ✅ released | WiFi configuration redesign — three-way `[wifi] mode = wifi\|ap\|off` (replaces the old on/off toggle) with a carousel sub-screen selector; non-blocking connect state machine with credential-rejection → automatic AP rescue and exponential backoff for unreachable networks (`WiFiMulti` removed); AP hotspot is now **open** and named `ESP32-Clock-XXXXXX` per device — the PIN authorises only the web UI's mutating actions; alarm always fires with a drift-warning overlay instead of the GIF when no time-sync source is available (AP/Off mode, or NTP timeout), and the 5-minute early-wake margin is skipped entirely outside WiFi mode |
| v2.7.2 | ✅ released | Three-stage CI/CD pipeline — compile validation against a pinned toolchain, an `FW_VERSION` guard that fails any PR reusing a released version, and automatic tag-and-release on `main` — plus a weekly canary that rebuilds against the latest upstream core and libraries. One shared tune engine now backs the apps menu and every game, fixing two tune bugs |
| v3.0.0 | ✅ released | **ESP32-S3 support** — one sketch, two boards, all hardware differences in `board_config.h`; storage abstraction that falls back from SD card to the S3's internal FFat partition (with a generated default `config.ini` on a virgin device); **internal-flash provisioning** — GIFs on the card are mirrored onto FFat at boot, so a board provisioned once from a card keeps its animations with the card removed; tilt-only games and emotion cycling hidden at runtime where no IMU answers; **swipe left/right to adjust brightness** on every board; atomic config writes (temp file + rename); AP SSID MAC read from eFuse instead of the not-yet-created softAP netif; missing-GIF paths reported instead of rendering a blank screen; **web file manager** at `/files` — browse, download, delete and upload on either storage backend, with a three-layer free-space guard; **dual-target CI and releases** — every release ships a binary set per board, built from one shared target definition |
| v3.1.0 | ✅ released | **USB Mode / mouse jiggler (S3 only)** — the board can present itself as a USB HID mouse and nudge the cursor at randomised intervals to keep a host awake; three-way persona (`HID` / `HID+Serial` / `Serial`) chosen from a carousel reached by long-pressing the analog clock, persisted as `[usb] mode` and applied on the next boot, since a composite USB descriptor cannot be swapped live; jiggling runs only while the smile GIF is open, starting 5 s after it opens; **hold BOOT through power-up** to force Serial-only for one boot, so a HID-only choice can never lock out reflashing. Requires **USB CDC On Boot = `Disabled`** on the S3 — the opposite of the C6 — because the sketch now brings its own CDC/HID interfaces up itself. Compiled out entirely on the C6, which has no USB-OTG peripheral. Also: a GIF that will not fit now names a disabled **PSRAM** build setting directly instead of blaming the asset size |
| v3.2.0 | ✅ released | **macroPad — ASCII art over USB (S3 only)** — the clock enumerates as a USB keyboard and types a stored text file into whatever window has focus, one keystroke at a time, so the mechanism is visible rather than magic. Ships with three `.art` samples (`ASCII_house`, `ASCII_hut`, `ASCII_penguin`) in `sd_card_root/scripts/`; drop a new file in `/scripts` and it appears in the menu, no toolchain involved. Also: the **mouse jiggler**'s return path is now a mirror of its outbound one — the same step magnitudes replayed backwards rather than a freshly randomised split — so host pointer acceleration applies equally to both legs and the cursor stops creeping over long sessions. Second item on the USB carousel; scripts live in `/scripts` on whichever storage backend is active, listed with no extension filter and created automatically at boot so the [file manager](#file-manager) always has an upload target. Tap a script for a **3-second countdown** — time to click into the target window, since the device cannot know what has focus — then a progress bar tracks position in the file and **tapping the screen stops it part-way**. Only the trailing line ending is stripped, so leading indentation survives intact for ASCII art. The keyboard registers on the same composite descriptor as the jiggler's mouse, so the HID personas now enumerate **mouse and keyboard** on one port; a `Serial`-only boot has no keyboard and says so instead of counting down to nothing. Compiled out entirely on the C6 |
| v3.3.0 | 🚀 new | **ToneQuest** — a Simon-says tone-memory game, ported from the [Arduino original](https://github.com/andreimagic/ToneQuest_Game) where a joystick picked the directions and four LEDs echoed them. The joystick is now the IMU and the LEDs are four "sunset" domes rising from the screen edges, but the direction→tone table is the original one note for note (UP D4, DOWN C4, LEFT E4, RIGHT F4). Every game opens on a **bubble level**: hold the ball inside the centre ring for 700 ms and the round begins. Then watch the sequence play back — each step lights its edge as a semicircle that fades out like a setting sun — and roll the ball into the same walls in the same order, coming back near the centre between moves the way the original joystick sprang back. Level 1 is four moves and every level adds one, revealed as a prefix of one pattern drawn per game, so level N is always level N−1 plus one new move. There is no win state: the score *is* the level you reach, persisted as `[tonequest] high_score` and tunable via `start_moves` / `flash_ms` / `gap_ms` / `tilt_percent`. Sits between Bingo! and the sounds toggle; tilt-only, so it is hidden on a board with no IMU |

## License

MIT — do whatever you like with it.