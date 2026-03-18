# ESP32-C6 Touch LCD 1.47" — LVGL Animated Clock

[![GitHub](https://img.shields.io/badge/github-andreimagic%2FESP32__C6__Touch__LCD__1__47__LVGL__Animated__Clock-blue?logo=github)](https://github.com/andreimagic/ESP32_C6_Touch_LCD_1_47_LVGL_Animated_Clock)

A smart animated clock for kids built on the **Waveshare ESP32-C6 Touch LCD 1.47"** board, driven by **LVGL v9**. It displays the time in a large custom font, plays animated GIF emotions on a schedule, sounds a buzzer alarm, and controls display brightness via device tilt — all configured from a plain `config.ini` file on the SD card, no recompile needed.

---

## Features

| Feature | Details |
|---|---|
| **Big clock face** | HH:MM in a full-screen custom font (Montserrat 96px) |
| **Hello! splash** | Shown for 2.5 s on boot before the clock appears |
| **Animated GIFs** | Smile (day) and Sleep (night) emotions from SD card |
| **Scheduled animation** | GIF plays every 5 min (configurable duration), 800 ms fade back to clock |
| **Night mode** | Sleep GIF used automatically between 20:00 and 07:00 |
| **Alarm** | Configurable wake-up time, buzzer beeps, smile GIF, stops on touch |
| **Brightness schedule** | Auto-dims at 19:00 → 19:30 → 20:00, brightens at 06:00 → 07:00 |
| **Tilt brightness** | Tilt device left/right in the Status screen to adjust brightness |
| **WiFi + NTP** | Connects at boot, syncs time automatically |
| **Status screen** | Shows WiFi SSID, NTP sync status, date, and current brightness |
| **Battery monitor** | Shows ADC raw value and calculated voltage |
| **SD card config** | All settings in `/config.ini` — no recompile needed |
| **LVGL v9** | Hardware-accelerated UI, zero blocking in the main loop |

---

## Hardware

### Board

**Waveshare ESP32-C6 Touch LCD 1.47"**

Select **ESP32C6 Dev Module** in Arduino IDE. The board integrates the ST7789 display, AXS5106L touch controller, QMI8658 IMU and SD card slot on a single compact PCB.

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

### Buzzer Wiring

Connect a **passive buzzer** (not active) between **GPIO 5** and **GND**. If the buzzer is very loud, add a 100 Ω resistor in series. The firmware drives it at 2 kHz via PWM.

---

## Software Dependencies

Install all libraries through **Arduino IDE → Library Manager** unless noted otherwise.

| Library | Version tested | Purpose |
|---|---|---|
| **LVGL** | 9.5.0 | UI framework — widgets, animations, timers |
| **Arduino_GFX_Library** | latest | ST7789 display driver |
| **SD** | built-in ESP32 | SD card file access |
| **WiFi / WiFiMulti** | built-in ESP32 | WiFi connection |
| **FastIMU** | latest | QMI8658 accelerometer (tilt brightness) |
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

> `montserrat_96` (the main clock font) is a **custom generated file** — see [Custom Font](#custom-font) below.

---

## Custom Font

The large clock digits use a custom Montserrat bitmap at 96 px, generated offline to include only the characters needed (digits 0–9 and colon), keeping the file small.

1. Download **Montserrat-Regular.ttf** from [Google Fonts](https://fonts.google.com/specimen/Montserrat)
2. Go to **[https://lvgl.io/tools/fontconverter](https://lvgl.io/tools/fontconverter)**
3. Settings:
   - Font: upload `Montserrat-Regular.ttf`
   - Size: `96`
   - Range: `0x30-0x3A` (digits `0–9` + colon only)
   - Bpp: `4`
   - Name: `montserrat_96`
4. Download the generated `montserrat_96.c` file
5. Place it in the **same folder as the `.ino` sketch**

Arduino will compile it automatically as part of the project.

---

## SD Card Setup

Format the SD card as **FAT32**. Create the following structure:

```
SD root/
├── config.ini
└── cruzr_emotions/
    ├── cruzr_smile.gif     ← 160 × 86 px
    └── cruzr_sleep.gif     ← 160 × 86 px
```

### GIF Requirements

GIF files **must be resized to 160 × 86 pixels** before copying to the SD card. The LVGL GIF decoder allocates an ARGB8888 canvas (width × height × 4 bytes). At full 320 × 172 px that requires 220 KB of contiguous RAM which the ESP32-C6 cannot provide when WiFi is active. At 160 × 86 px it needs only 55 KB.

**To resize:** go to [https://ezgif.com/resize](https://ezgif.com/resize), upload your GIF, set Width=160 Height=86, download and copy to the SD card.

The firmware scales them 2× at render time to fill the 320 × 172 screen.

---

## config.ini Reference

```ini
[wifi]
ssid = myhomewifi
password = changeme

[clock]
# UTC offset in whole hours — positive east of UTC, negative west
# Examples: 1 = UTC+1 (CET), -5 = UTC-5 (EST), 5.5 not supported (use 5)
gmt_offset = 1
ntp_server = pool.ntp.org

# Comment lines start with #

[alarm]
enabled = true
time = 07:10

[animation]
# Play smile GIF (day) or sleep GIF (night) automatically every 5 minutes
# set schedule = false to disable
schedule = true
# Seconds the GIF plays before fading back to the clock (3-60)
duration = 10
```

| Section | Key | Type | Default | Description |
|---|---|---|---|---|
| `[wifi]` | `ssid` | string | `myhomewifi` | WiFi network name |
| `[wifi]` | `password` | string | `changeme` | WiFi password |
| `[clock]` | `gmt_offset` | int (-12–14) | `1` | UTC offset in hours (e.g. `1` = UTC+1, `-5` = UTC-5) |
| `[clock]` | `ntp_server` | string | `pool.ntp.org` | NTP time server |
| `[alarm]` | `enabled` | bool | `false` | Enable morning alarm |
| `[alarm]` | `time` | `HH:MM` | `07:00` | Alarm time |
| `[animation]` | `schedule` | bool | `true` | Enable periodic GIF every 5 min |
| `[animation]` | `duration` | int (3–60) | `10` | Seconds each GIF plays before fading |

> If `config.ini` is missing the firmware boots with the hardcoded defaults shown above.

---

## Touch Zones — Home Screen

The home screen has **four invisible touch zones**. Tap to open a sub-screen, **long-press anywhere** to open the Alarm editor.

```
┌─────────────────────────────────────────┐
│                   │                     │
│   Smile GIF       │    Sleep GIF        │
│   (upper-left)    │    (upper-right)    │
│                   │                     │
├───────────────────┼─────────────────────┤
│                   │                     │
│   Status          │    Battery          │
│   (lower-left)    │    (lower-right)    │
│                   │                     │
└─────────────────────────────────────────┘
         LONG-PRESS anywhere → Alarm editor
```

---

## Sub-screens

### Status (lower-left tap)
Shows WiFi connection status (SSID or disconnected), NTP sync state, today's date, and current brightness level. **Tilt the device left or right** while this screen is open to decrease or increase brightness in 10% steps.

### Battery (lower-right tap)
Shows the raw ADC reading and calculated battery voltage. Values update every second.

### GIF animations (upper taps)
Opens the corresponding GIF fullscreen. Tap anywhere to return to the clock.

### Alarm editor (long-press anywhere)
```
┌─────────────────────────────────────────┐
│  🔔  Set Alarm                           │
│  ─────────────────────────────────────  │
│        ▲          ▲          ▲          │
│      [07]  :    [10]      [ ON ]        │
│        ▼          ▼          ▼          │
│                                         │
│          hold to save & exit            │
└─────────────────────────────────────────┘
```
- Tap the **top half** of a column to increase the value
- Tap the **bottom half** to decrease
- Tap **ON / OFF** to toggle the alarm
- **Long-press** anywhere to save changes to `config.ini` and return to the clock
- A 🔔 bell icon with the alarm time appears in the bottom-right corner of the clock face when the alarm is enabled

---

## Daily Automation Schedule

All times are local time. Timezone is set via `gmt_offset` in `[clock]` in `config.ini`.

| Time | Action |
|---|---|
| 06:00 | Brightness → 10% |
| 06:30 | Brightness → 25% |
| 07:00 | Brightness → 50% |
| Alarm time | Brightness → 80%, smile GIF, buzzer starts |
| 19:00 | Brightness → 25% |
| 19:30 | Brightness → 10% |
| 20:00 | Brightness → 1% |
| 20:15 | Sleep GIF starts automatically |
| 21:00 | Sleep GIF closes automatically (if not already dismissed) |

**Buzzer:** plays a `beep-beep-beep … pause` pattern (3 × 200 ms on, 700 ms pause) until the screen is touched.

---

## Scheduled Animation

When `[animation] schedule = true`, a GIF plays automatically every 5 minutes for the configured duration, then fades back to the clock over 800 ms.

| Time of day | GIF played |
|---|---|
| Day (07:00–19:59) | `cruzr_smile.gif` |
| Night (20:00–06:59) | `cruzr_sleep.gif` |

- Skipped silently if any sub-screen is already open
- Touch the screen at any time to dismiss immediately
- The GIF fades out over 800 ms when the timer expires

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
8. Edit `lv_conf.h` as described in [lv_conf.h Settings](#lv_confh-settings)
9. Place `montserrat_96.c` in the sketch folder
10. Prepare the SD card as described in [SD Card Setup](#sd-card-setup)
11. Open `ESP32_C6_Touch_LCD_1_47_LVGL_Animated_Clock.ino`, click **Upload**
12. Open Serial Monitor at **115200 baud** to watch the boot log

### Expected Boot Log

```
========== BOOT ==========
[1] Pulling CS pins HIGH...
[2] SPI.begin(SCK=1, MISO=3, MOSI=2, CS=4)...
[3] Initialising display...
    gfx->begin() OK.
[4] Initialising touch...
[4b] Initialising IMU...
    IMU ready.
[5] Mounting SD card...
    SD mounted OK — type: SD  size: 244 MB
    GIF found — XXXXX bytes
[CFG] Loading /config.ini...
[CFG]   wifi.ssid     = myhomewifi
[CFG]   wifi.password = (hidden)
[6] Initialising LVGL...
[7] Registering LVGL SD filesystem driver...
[7b] Starting WiFi + NTP client (non-blocking)...
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
| Clock shows `--:--` permanently | NTP not synced | Check WiFi credentials in `config.ini`; check serial for connection errors |
| Touch zones unresponsive | Touch controller not detected | Check I²C wiring on pins 18/19; watch for `read: 8161` in log (normal) |
| IMU not working | Address mismatch or wiring | Confirm `IMU_ADDRESS = 0x6B`; check serial for IMU error code |
| Alarm not firing | `enabled = false` | Set `enabled = true` in `config.ini` |
| Buzzer silent | Wrong pin or active buzzer | Must be a **passive** buzzer on GPIO 5; active buzzers don't work with PWM |
| Font not found (compile error) | `montserrat_96.c` missing | Generate and place the file as described in [Custom Font](#custom-font) |

---

## Architecture Notes

- **No blocking calls in `loop()`** — `loop()` only calls `lv_timer_handler()` + `delay(5)`. All WiFi polling, clock ticks, brightness schedules, buzzer patterns and animations run as LVGL timer callbacks.
- **SD ↔ LVGL filesystem bridge** — a custom `lv_fs_drv_t` registered under drive letter `'S'` forwards all LVGL file operations to the Arduino `SD` library. This lets `lv_gif_set_src()` open files directly from the card.
- **GIF memory management** — the LVGL GIF decoder needs a contiguous block for its canvas. GIFs are pre-scaled to 160×86 px (55 KB canvas) so they fit alongside the WiFi stack. The render buffer uses 20 scan lines for good throughput without exhausting RAM.
- **config.ini** — parsed once at boot with a hand-rolled INI reader (no external library). Comments (`#`), blank lines and whitespace around `=` are all handled. The `[alarm]` section is rewritten on save while preserving all other sections and comments.
- **Scheduled animation** — `run_scheduled_animation()` fires on `minute % 5 == 0` inside `run_daily_automation()`. A one-shot LVGL timer triggers `sched_gif_close_cb()` after the configured duration, which fades opacity 100→0% over 800 ms via `lv_anim` then deletes the overlay. Touching the screen cancels the timer before it fires.

---

## Roadmap

| Version | Status | Feature |
|---|---|---|
| v1.0 | ✅ released | Clock, alarms, GIF on tap, buzzer, brightness tilt, config.ini |
| v1.1 | ✅ released | Scheduled animation (smile/sleep every 5 min, 800 ms fade) |

## Contributing

Pull requests are welcome. For major changes please open an issue first.

## License

MIT — do whatever you like with it.
