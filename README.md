# ESP32-C6 Touch LCD 1.47" — LVGL Animated Clock

[![GitHub](https://img.shields.io/badge/github-andreimagic%2FESP32__C6__Touch__LCD__1__47__LVGL__Animated__Clock-blue?logo=github)](https://github.com/andreimagic/ESP32_C6_Touch_LCD_1_47_LVGL_Animated_Clock)

A smart animated clock for kids built on the **Waveshare ESP32-C6 Touch LCD 1.47"** board, driven by **LVGL v9**. It displays the time in a large custom font, plays animated GIF emotions on a schedule, sounds configurable buzzer alarms, runs a countdown timer, manages display brightness via device tilt, and supports a full software power-off — all configured from a plain `config.ini` file on the SD card, no recompile needed.

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
| **RTC persistence** | Hourly timestamp log on SD card (`/last_seen.txt`) restores time on cold boot without WiFi |
| **Status screen** | Today's date in the title, WiFi SSID, NTP sync state, current brightness |
| **Battery monitor** | Live percentage, voltage, ADC raw — with LiPo discharge curve |
| **Battery warning** | Clock text turns orange ≤ 25%, red ≤ 10%; auto-poweroff countdown at ≤ 10% |
| **Software power-off** | Long-press battery screen → 5 s countdown → deep sleep; RESET button to wake |
| **Alarm auto-wake** | If alarm is set, device wakes from deep sleep automatically 30 s before alarm time |
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
2026-03-23 07:30:00 (3.92V)
```

On cold boot with no WiFi, the firmware reads the last line of this file and restores the RTC to that timestamp. This means the clock shows a close approximation of the real time even without a network connection — typically accurate to within a few minutes of the last hourly log entry. Logging stops automatically if battery voltage drops below 3.4V to protect the SD card during low-power conditions.

---

## config.ini Reference

```ini
[wifi]
enabled = true
ssid = myhomewifi
password = changeme

[clock]
# UTC offset in whole hours — positive east of UTC, negative west
# Examples: 1 = UTC+1 (CET), -5 = UTC-5 (EST), 5.5 not supported (use 5)
gmt_offset = 1
ntp_server = pool.ntp.org

[alarm]
enabled = true
time = 07:10
# Number of 4-beep sequences before auto-stop. 0 = beep until screen is touched.
beep_sequences = 5

[timer]
# Last used countdown duration (HH:MM). Saved automatically when you set the timer.
duration = 00:05
# Number of beep sequences when timer reaches zero. 0 = until screen is touched.
beep_sequences = 3

[animation]
# Play smile GIF (day) or sleep GIF (night) automatically
# set schedule = false to disable
schedule = true
# Seconds the GIF plays before fading back to the clock (3-60)
duration = 10
```

| Section | Key | Type | Default | Description |
|---|---|---|---|---|
| `[wifi]` | `enabled` | bool | `true` | Enable WiFi and NTP sync |
| `[wifi]` | `ssid` | string | `myhomewifi` | WiFi network name |
| `[wifi]` | `password` | string | `changeme` | WiFi password |
| `[clock]` | `gmt_offset` | int (-12–14) | `1` | UTC offset in hours |
| `[clock]` | `ntp_server` | string | `pool.ntp.org` | NTP time server |
| `[alarm]` | `enabled` | bool | `false` | Enable morning alarm |
| `[alarm]` | `time` | `HH:MM` | `07:00` | Alarm time |
| `[alarm]` | `beep_sequences` | int | `5` | Repeat count for alarm buzzer (0 = until touch) |
| `[timer]` | `duration` | `HH:MM` | `00:00` | Last countdown duration (saved automatically) |
| `[timer]` | `beep_sequences` | int | `3` | Repeat count for timer buzzer (0 = until touch) |
| `[animation]` | `schedule` | bool | `true` | Enable periodic GIF animation |
| `[animation]` | `duration` | int (3–60) | `10` | Seconds each scheduled GIF plays before fading |

> If `config.ini` is missing the firmware boots with the hardcoded defaults shown above.

---

## Home Screen Touch Zones

The home screen has **four invisible touch zones**. Tap to open a sub-screen. **Long-press anywhere** to open the Carousel settings menu.

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
         LONG-PRESS anywhere → Carousel menu
```

When the countdown timer is running, a small `⏹ MM:SS` label appears in the bottom-left corner of the clock face. When the alarm is enabled, a 🔔 bell icon with the alarm time appears in the bottom-right corner.

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

**Tap** the centre area to enter the selected item. **Long-press** anywhere to exit back to the clock with no changes.

### Clock — set date and time

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

Opens the HH:MM editor with an **ON / OFF** toggle. Long-press saves to `config.ini`. When enabled, at the configured time any running scheduled animation is first dismissed, then the device raises brightness to 80%, plays `alarm_animation.gif` fullscreen, and sounds the buzzer `beep_sequences` times. The animation fades out automatically after the last beep. Touching the screen dismisses the alarm early.

### WiFi — inline toggle

Tap the centre to toggle WiFi on or off. No sub-screen. The description updates to green **ON** or red **OFF** immediately, and the change is saved to `config.ini`. When WiFi is disabled, NTP sync is suspended and reconnect attempts are skipped entirely.

---

## Sub-screens

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

> The upper-right zone always opens `cruzr_sleep.gif` directly with no tilt interaction — tilt emotion mode is exclusive to the upper-left zone.

---

## Power Off and Wake

### Powering off
Long-press the battery screen to open the shutdown popup. After the countdown (or immediately if not cancelled) the device enters deep sleep drawing ~10 µA.

### Waking up
Press the **RESET** button on the device body. This always causes a clean reboot through the full boot sequence.

> The BOOT button on this board is wired to GPIO9, which is not a low-power GPIO on the ESP32-C6 and cannot trigger a wake-from-deep-sleep interrupt. RESET is the reliable wake method.

### Alarm auto-wake
If an alarm is configured and enabled, the firmware sets a timer wakeup before entering deep sleep. The device wakes automatically 30 seconds before the alarm time, completing the boot sequence so the alarm fires at the correct moment. If no alarm is set, the device sleeps indefinitely until RESET is pressed.

The RTC keeps running during deep sleep from the battery. On wake, the correct time is shown immediately without waiting for NTP.

### Boot behaviour

| Scenario | Splash | Duration | Clock display |
|---|---|---|---|
| Cold boot / RESET, WiFi available | `Hello!` (large) | 2.5 s | `--:--` until NTP syncs |
| Cold boot / RESET, no WiFi, log exists | `Hello!` (large) | 2.5 s | Time restored from `last_seen.txt` |
| Cold boot / RESET, no WiFi, no log | `Hello!` (large) | 2.5 s | `--:--` until manually set |
| Wake from deep sleep (alarm timer) | `⌂ Salut!` (small) | 1.0 s | RTC time shown immediately |

---

## Daily Automation Schedule

All times are local time, set via `gmt_offset` in `[clock]`. Automation runs whenever the RTC holds a valid time (epoch > 2026-01-01), regardless of WiFi or NTP status.

| Time | Action |
|---|---|
| 06:00 | Brightness → 10% |
| 06:30 | Brightness → 25% |
| 07:00 | Brightness → 50% |
| Alarm time | Dismiss any scheduled GIF, brightness → 80%, `alarm_animation.gif`, buzzer |
| Every N min | Scheduled GIF animation (if enabled, skipped when alarm fires same minute) |
| 19:00 | Brightness → 25% |
| 19:30 | Brightness → 10% |
| 20:00 | Brightness → 1% |
| 20:15 | Sleep GIF starts automatically |
| 21:00 | Sleep GIF closes automatically (if not already dismissed) |

---

## Scheduled Animation

When `[animation] schedule = true`, a GIF plays automatically on the configured minute interval for the configured duration, then fades back to the clock over 800 ms.

| Time of day | GIF played |
|---|---|
| Day (07:00–19:59) | `cruzr_smile.gif` |
| Night (20:00–06:59) | `cruzr_sleep.gif` |

Skipped silently if any sub-screen, overlay, or carousel is already open. When the alarm or timer fires at the same time as a scheduled animation, the scheduled animation is immediately dismissed and the alarm/timer animation takes over. Touch the screen at any time to dismiss immediately.

---

## Buzzer Pattern

Both alarm and timer use the same pattern: **4 × (200 ms ON + 100 ms OFF) + 1000 ms pause** = one sequence. The number of sequences before auto-stop is configured independently for alarm and timer in `config.ini`. Setting `beep_sequences = 0` means the buzzer repeats until the screen is touched. When the sequence count is finite, the animation fades out automatically after the last beep.

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
8. Place `montserrat_96.c` in the sketch folder
9. Prepare the SD card as described in [SD Card Setup](#sd-card-setup)
10. Open `ESP32_C6_Touch_LCD_1_47_LVGL_Animated_Clock.ino`, click **Upload**
11. Open Serial Monitor at **115200 baud** to watch the boot log

### Expected Boot Log

```
========== BOOT ==========
[BOOT] Wake cause: cold boot / RESET button
[1] Pulling CS pins HIGH...
[2] SPI.begin(SCK=1, MISO=3, MOSI=2, CS=4)...
[3] Initialising display...
    gfx->begin() OK.
[4] Initialising touch...
[4b] Initialising IMU...
    IMU ready.
[5] Mounting SD card...
    SD mounted OK — type: SD  size: 244 MB
[CFG] Loading /config.ini...
[CFG]   wifi.enabled       = true
[CFG]   wifi.ssid          = myhomewifi
[CFG]   wifi.password      = (hidden)
[RTC] Restored time from log: 2026-03-23 07:29:00
[6] Initialising LVGL...
[7] Registering LVGL SD filesystem driver...
[7b] Applying WiFi state from config...
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
| Status shows "Status" not date | RTC not yet valid | Cold boot with no WiFi and no log; set time via Clock editor |
| Touch zones unresponsive | Touch controller not detected | Check I²C wiring on pins 18/19; watch for `read: 8161` in log (normal) |
| IMU not working | Address mismatch or wiring | Confirm `IMU_ADDRESS = 0x6B`; check serial for IMU error code |
| Alarm not firing | `enabled = false` or device was asleep | Set `enabled = true`; check deep sleep timer wakeup fired in boot log |
| Alarm GIF replaced by scheduled GIF | Timing conflict (old firmware) | Fixed in v1.3.0 — `close_scheduled_gif()` evicts scheduled animation first |
| Buzzer silent | Wrong pin or active buzzer | Must be a **passive** buzzer on GPIO 5; active buzzers don't work with PWM |
| BOOT button doesn't wake from sleep | Hardware limitation | GPIO9 is not an LP GPIO on ESP32-C6; press **RESET** instead |
| UI stutters briefly | WiFi reconnect stall | Suppressed while carousel/editors open; reduced to every 30 s when disconnected |
| Battery % jumps on unplug | ADC reads elevated USB voltage | Normal — LiPo voltage estimate is slightly elevated while USB powers the system |
| `last_seen.txt` not created | SD write error or low battery | Check SD card is writable FAT32; battery must be above 3.4V for logging |
| Font not found (compile error) | `montserrat_96.c` missing | Generate and place the file as described in [Custom Font](#custom-font) |

---

## Architecture Notes

- **No blocking calls in `loop()`** — `loop()` only calls `lv_timer_handler()` + `delay(5)`. All WiFi polling, clock ticks, brightness schedules, buzzer patterns, countdown timer, and animations run as LVGL timer callbacks.
- **SD ↔ LVGL filesystem bridge** — a custom `lv_fs_drv_t` registered under drive letter `'S'` forwards all LVGL file operations to the Arduino `SD` library. This lets `lv_gif_set_src()` open files directly from the card with the prefix `S:/`.
- **GIF memory management** — the LVGL GIF decoder needs a contiguous block for its canvas. GIFs are pre-scaled to 160×86 px (55 KB canvas) so they fit alongside the WiFi stack. The render buffer uses 20 scan lines for good throughput without exhausting RAM.
- **RTC persistence** — `log_last_seen()` appends a timestamped voltage reading to `/last_seen.txt` every hour, on every config save, and when the clock editor is used. On cold boot, `restore_time_from_log()` reads the last entry and sets the RTC, providing approximate time without WiFi. Waking from deep sleep skips the log restore since the RTC kept running from battery.
- **Carousel** — a full-screen LVGL modal opened by long-press. Each tap on ◀/▶ calls `lv_obj_clean()` and rebuilds the view in place. The centre zone uses `LV_EVENT_CLICKED` (not `LV_EVENT_PRESSED`) so long-press and tap are mutually exclusive — the editor never opens before the long-press exit fires.
- **Shared editor** — `open_editor()` builds the HH:MM widget for Timer and Alarm. `open_clock_editor()` builds the full two-row date+time widget. `modal_longpress_cb()` dispatches to the correct save function based on `carousel_idx`.
- **Animation priority** — `close_scheduled_gif()` forcefully tears down any scheduled overlay (cancels fade timer, deletes overlay synchronously) before alarm or timer open their GIF. Scheduled animation is also skipped entirely if the alarm fires on the same minute.
- **Emotion tilt** — `zone_ul_cb` sets `emotion_tilt_active = true` and starts `tilt_timer` after opening the smile GIF. `tilt_poll_cb` branches on this flag: in emotion mode it reads both `accelX` (forward/back) and `accelY` (left/right), determines the desired GIF path, and calls `lv_gif_set_src()` on the existing widget (retrieved from `overlay_cont` user data) only when the path changes. This swaps the animation in-place with no overlay rebuild. Both the flag and the timer are cleared by `overlay_close_event_cb`.
- **Buzzer state machine** — a single 9-step table drives both alarm and timer patterns. `buzzer_fade_after` is set by the caller for finite sequences; `buzzer_stop()` triggers `overlay_fade_and_close()` automatically after the last beep.
- **WiFi reconnect guard** — `wifi_poll_cb()` skips `wifiMulti.run()` when any modal or overlay is open, when WiFi is manually disabled (`cfg.wifi_enabled`), and reduces attempts to every 30 s when disconnected — preventing radio lock stalls from blocking UI interaction.
- **Automation gate** — `run_daily_automation()` fires when `now > 2026-01-01` (RTC sanity check) instead of `timeSynced`, so brightness schedules, alarms, and animations all work correctly when WiFi is disabled or the time was set manually.
- **Deep sleep** — `esp_deep_sleep_start()` with `esp_sleep_enable_timer_wakeup()` set 30 s before the next alarm. If no alarm is configured, the device sleeps indefinitely until RESET.
- **config.ini** — parsed once at boot with a hand-rolled INI reader (no external library). On save, `[wifi]`, `[alarm]`, and `[timer]` sections are fully rewritten; all other sections and comments are preserved verbatim.

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

## License

MIT — do whatever you like with it.
