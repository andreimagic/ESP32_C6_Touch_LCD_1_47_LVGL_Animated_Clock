/*
 * ESP32-C6 Touch LCD 1.47" — LVGL Animated Clock
 * https://github.com/andreimagic/ESP32_C6_Touch_LCD_1_47_LVGL_Animated_Clock
 *
 * A smart animated clock for kids built on the ESP32-C6, driven by LVGL v9.
 * Displays the time in large digits, plays GIF animations on a schedule,
 * sounds a buzzer alarm, and controls brightness via tilt.
 * All user settings live in /config.ini on the SD card — no recompile needed.
 *
 * Board  : ESP32-C6 Dev Module
 * Display: ST7789 172×320 (landscape) via Arduino_GFX
 * Touch  : AXS5106L (I²C)
 * IMU    : QMI8658 (I²C, shared bus with touch)
 *
 * lv_conf.h requirements:
 *   LV_USE_GIF             = 1
 *   LV_USE_STDLIB_MALLOC   = LV_STDLIB_CLIB
 *   LV_USE_STDLIB_STRING   = LV_STDLIB_CLIB
 *   LV_USE_STDLIB_SPRINTF  = LV_STDLIB_CLIB
 *   LV_FONT_MONTSERRAT_14/16/48 = 1
 *   (montserrat_96.c is a custom generated font — see README)
 *
 * SD card filesystem bridge:
 *   A custom lv_fs_drv_t registered under drive letter "S" forwards every
 *   LVGL file operation (open/read/seek/close) to the Arduino SD library.
 *   Any LVGL widget that accepts a path can reference SD files with the
 *   prefix "S:/" — e.g. "S:/cruzr_emotions/cruzr_smile.gif"
 *
 * GIF files must be pre-scaled to 160×86 px (see README for why).
 * The firmware scales them 2× at render time to fill the 320×172 screen.
 */

#include <lvgl.h>
#include "esp_lcd_touch_axs5106l.h"
#include <Arduino_GFX_Library.h>
#include <SD.h>
#include <SPI.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <time.h>
#include <FastIMU.h>



// ─── Runtime configuration ───────────────────────────────────────────────────
// Loaded from /config.ini on the SD card at boot.
// These hardcoded values are the fallback when the card or file is absent.

struct AppConfig {
  char wifi_ssid[64]          = "myhomewifi";     // [wifi] ssid
  char wifi_password[64]      = "changeme";       // [wifi] password
  char ntp_server[64]         = "pool.ntp.org";   // [clock] ntp_server
  char tz_string[48]          = "CET-1CEST,M3.5.0,M10.5.0/3"; // [clock] tz (POSIX — set once, handles DST forever)
  bool wifi_enabled           = true;        // [wifi] enabled
  bool alarm_enabled          = false;       // [alarm] enabled
  int  alarm_hour             = 7;           // [alarm] time HH
  int  alarm_minute           = 0;           // [alarm] time MM
  int  alarm_beep_sequences   = 5;           // [alarm] beep_sequences (0=until touch)
  int  timer_hours            = 0;           // [timer] hours
  int  timer_minutes          = 0;           // [timer] minutes
  int  timer_beep_sequences   = 3;           // [timer] beep_sequences
  bool anim_schedule_enabled  = true;        // [animation] schedule
  int  anim_duration_sec      = 10;          // [animation] duration
  bool menu_sounds            = true;        // [menu] sounds
} cfg;

// ─── Pin definitions ─────────────────────────────────────────────────────────
#define ROTATION        1
#define GFX_BL          23
#define BUZZER_PIN      5    // Passive buzzer — connect between GPIO5 and GND
#define SD_CS           4
#define SD_SCK          1
#define SD_MOSI         2
#define SD_MISO         3

#define Touch_I2C_SDA   18
#define Touch_I2C_SCL   19
#define Touch_RST       20
#define Touch_INT       21

#define BAT_PIN         0

// ─── GIF paths — SD card root is mapped to LVGL drive letter "S" ──────────────
#define GIF_SMILE_PATH  "S:/cruzr_emotions/cruzr_smile.gif"
#define GIF_SLEEP_PATH  "S:/cruzr_emotions/cruzr_sleep.gif"
#define GIF_SAD_PATH    "S:/cruzr_emotions/cruzr_sad.gif"
#define GIF_JOY_PATH    "S:/cruzr_emotions/cruzr_joy.gif"
#define GIF_ALARM_PATH  "S:/cruzr_emotions/alarm_animation.gif"
#define GIF_TIMER_PATH  "S:/cruzr_emotions/timer_animation.gif"

// ─── Forward declarations ─────────────────────────────────────────────────────
static void home_screen_init(void);
static void clock_face_show(lv_timer_t *t);
static void clock_tick_cb(lv_timer_t *t);
static void wifi_poll_cb(lv_timer_t *t);
static void show_gif_fullscreen(const char *path);
static void show_battery_screen(void);
static void show_status_screen(void);
static void overlay_close_event_cb(lv_event_t *e);
static void lvgl_sd_fs_init(void);
static void show_carousel(void);
static void save_config(void);
static void apply_wifi_state(void);


// ─── Display ─────────────────────────────────────────────────────────────────
Arduino_DataBus *bus = new Arduino_HWSPI(15 /* DC */, 14 /* CS */, 1 /* SCK */, 2 /* MOSI */);
Arduino_GFX    *gfx = new Arduino_ST7789(
  bus, 22 /* RST */, 0 /* rotation */, false /* IPS */,
  172 /* width */, 320 /* height */,
  34, 0, 34, 0);

// ─── LVGL display dimensions ─────────────────────────────────────────────────
// Set after gfx->begin(); used by the display driver and GIF size checks.
uint32_t  screenWidth;
uint32_t  screenHeight;

// ─── WiFi / NTP state ────────────────────────────────────────────────────────
WiFiMulti wifiMulti;
bool wifiConnected = false;
bool timeSynced    = false;

// ─── UI handles ──────────────────────────────────────────────────────────────
lv_obj_t   *overlay_cont   = nullptr;
lv_obj_t   *home_hello_lbl = nullptr;  // "Hello!" splash label
lv_obj_t   *home_time_lbl  = nullptr;  // HH:mm clock label on home screen
lv_obj_t   *label_adc_raw  = nullptr;
lv_obj_t   *label_voltage  = nullptr;
lv_timer_t *battery_timer  = nullptr;
lv_timer_t *clock_timer    = nullptr;
lv_timer_t *wifi_timer     = nullptr;
lv_obj_t   *home_bell_lbl  = nullptr;  // bell icon shown on home when alarm ON
lv_obj_t   *alarm_cont     = nullptr;  // alarm editor screen

bool sdCardAvailable  = false;
bool boot_from_sleep      = false;  // true when waking from deep sleep via BOOT btn
uint32_t boot_millis       = 0;     // millis() captured at start of setup()
bool     alarm_ntp_pending = false; // alarm held waiting for NTP sync after wake

lv_obj_t   *label_percent        = nullptr;  // battery % in title bar
lv_obj_t   *shutdown_popup       = nullptr;  // countdown confirmation card
lv_obj_t   *shutdown_cntdown_lbl = nullptr;  // "Shutting down in N..." label
lv_timer_t *shutdown_timer       = nullptr;  // 1-second tick
int         shutdown_count       = 5;

LV_FONT_DECLARE(montserrat_96);
LV_FONT_DECLARE(dejavu_mono_8);
LV_FONT_DECLARE(dejavu_mono_14);
LV_FONT_DECLARE(dejavu_mono_16);

// ─── Backlight PWM ───────────────────────────────────────────────────────────
#define BL_PWM_FREQ       5000
#define BL_PWM_RESOLUTION 8      // 0-255
#define IMU_ADDRESS       0x6B   // QMI8658

int  brightnessPercent = 50;     // boot brightness

// ─── IMU (QMI8658) — tilt-to-brightness ─────────────────────────────────────
QMI8658   imu;
calData   imuCalib  = {0};
AccelData accelData;
bool      imuReady  = false;

// ─── Brightness + tilt timer handles — valid only while Status screen is open ─
lv_obj_t   *label_brightness = nullptr;
lv_timer_t *tilt_timer         = nullptr;
bool        emotion_tilt_active = false;   // true while UL smile GIF is open
const char *emotion_current_gif = nullptr; // tracks which GIF is showing
lv_timer_t *buzzer_timer      = nullptr;  // alarm beep pattern timer
bool        buzzer_active     = false;
lv_timer_t *sched_close_timer = nullptr;  // auto-close timer for scheduled GIF

// ─── Carousel / modal settings ────────────────────────────────────────────────
lv_obj_t   *modal_cont   = nullptr;  // carousel / editor full-screen modal
lv_obj_t   *apps_cont    = nullptr;  // apps menu container
static int  carousel_idx = 0;        // 0=Clock 1=Timer 2=Alarm 3=WiFi

// ─── Countdown timer ─────────────────────────────────────────────────────────
lv_timer_t *countdown_timer = nullptr;
int         countdown_sec   = 0;
bool        timer_running   = false;
lv_obj_t   *home_timer_lbl  = nullptr;  // small label bottom-left when running



// ══════════════════════════════════════════════════════════════════════════════
//  SD ↔ LVGL FILESYSTEM BRIDGE  (drive letter "S")
//  Registers 6 callbacks (open/close/read/write/seek/tell) so that LVGL
//  widgets (lv_gif, lv_img, lv_font_load) can open SD files transparently.
// ══════════════════════════════════════════════════════════════════════════════
struct LvSdFile { File f; };

static void *lvgl_sd_open(lv_fs_drv_t * /*drv*/, const char *path, lv_fs_mode_t mode)
{
  const char *sdMode = (mode == LV_FS_MODE_WR) ? FILE_WRITE : FILE_READ;
  LvSdFile *fp = new LvSdFile();
  fp->f = SD.open(path, sdMode);
  if (!fp->f) { delete fp; return nullptr; }
  return (void *)fp;
}

static lv_fs_res_t lvgl_sd_close(lv_fs_drv_t * /*drv*/, void *file_p)
{
  LvSdFile *fp = (LvSdFile *)file_p;
  fp->f.close(); delete fp;
  return LV_FS_RES_OK;
}

static lv_fs_res_t lvgl_sd_read(lv_fs_drv_t * /*drv*/, void *file_p,
                                  void *buf, uint32_t btr, uint32_t *br)
{
  *br = ((LvSdFile *)file_p)->f.read((uint8_t *)buf, btr);
  return LV_FS_RES_OK;
}

static lv_fs_res_t lvgl_sd_write(lv_fs_drv_t * /*drv*/, void *file_p,
                                   const void *buf, uint32_t btw, uint32_t *bw)
{
  *bw = ((LvSdFile *)file_p)->f.write((const uint8_t *)buf, btw);
  return LV_FS_RES_OK;
}

static lv_fs_res_t lvgl_sd_seek(lv_fs_drv_t * /*drv*/, void *file_p,
                                  uint32_t pos, lv_fs_whence_t whence)
{
  LvSdFile *fp = (LvSdFile *)file_p;
  uint32_t target;
  switch (whence) {
    case LV_FS_SEEK_CUR: target = fp->f.position() + pos; break;
    case LV_FS_SEEK_END: target = fp->f.size()     + pos; break;
    default:             target = pos;                     break;
  }
  fp->f.seek(target);
  return LV_FS_RES_OK;
}

static lv_fs_res_t lvgl_sd_tell(lv_fs_drv_t * /*drv*/, void *file_p, uint32_t *pos)
{
  *pos = ((LvSdFile *)file_p)->f.position();
  return LV_FS_RES_OK;
}

static void lvgl_sd_fs_init(void)
{
  static lv_fs_drv_t drv;
  lv_fs_drv_init(&drv);
  drv.letter   = 'S';
  drv.open_cb  = lvgl_sd_open;
  drv.close_cb = lvgl_sd_close;
  drv.read_cb  = lvgl_sd_read;
  drv.write_cb = lvgl_sd_write;
  drv.seek_cb  = lvgl_sd_seek;
  drv.tell_cb  = lvgl_sd_tell;
  lv_fs_drv_register(&drv);
}


// ══════════════════════════════════════════════════════════════════════════════
//  BUZZER  —  passive buzzer on BUZZER_PIN via PWM
//  Plays a repeating "beep-beep-beep … pause" pattern using an LVGL timer.
//  Stopped by touching the screen (overlay_close_event_cb) or explicitly.
// ══════════════════════════════════════════════════════════════════════════════

// ══════════════════════════════════════════════════════════════════════════════
//  BUZZER — unified pattern for alarm and timer
//  One sequence = 4 × (200ms ON + 100ms OFF) + 1000ms pause
//  cfg.alarm_beep_sequences / cfg.timer_beep_sequences controls auto-stop.
//  0 = repeat until buzzer_stop() is called (touch to dismiss).
// ══════════════════════════════════════════════════════════════════════════════

static int  buzzer_seq_total  = 0;   // 0 = infinite
static int  buzzer_seq_done   = 0;
static int  buzzer_step       = 0;
static bool buzzer_fade_after = false; // fade overlay when beeping finishes

// 9 steps: 4 × (on200 + off100) + pause1000
static const struct { bool on; uint32_t ms; } BUZZ_STEPS[9] = {
  {true,200},{false,100},{true,200},{false,100},
  {true,200},{false,100},{true,200},{false,100},
  {false,1000}
};

static void overlay_fade_and_close()
{
  if (!overlay_cont) return;
  lv_anim_t a;
  lv_anim_init(&a);
  lv_anim_set_var(&a, overlay_cont);
  lv_anim_set_exec_cb(&a, [](void *obj, int32_t v) {
    if (obj) lv_obj_set_style_opa((lv_obj_t *)obj, (lv_opa_t)v, 0);
  });
  lv_anim_set_values(&a, LV_OPA_COVER, LV_OPA_TRANSP);
  lv_anim_set_duration(&a, 800);
  lv_anim_set_ready_cb(&a, [](lv_anim_t * /*a*/) {
    if (overlay_cont) {
      if (tilt_timer) { lv_timer_del(tilt_timer); tilt_timer = nullptr; }
      label_brightness = nullptr;
      lv_obj_del(overlay_cont);
      overlay_cont = nullptr;
      Serial.println("[ANIM] Alarm GIF closed (fade after beeping)");
    }
  });
  lv_anim_start(&a);
}

static void buzzer_stop()
{
  if (!buzzer_active) return;
  if (buzzer_timer) { lv_timer_del(buzzer_timer); buzzer_timer = nullptr; }
  ledcWrite(BUZZER_PIN, 0);
  buzzer_active   = false;
  buzzer_step     = 0;
  buzzer_seq_done = 0;
  Serial.println("[BUZZ] Stopped");
  if (buzzer_fade_after) {
    buzzer_fade_after = false;
    overlay_fade_and_close();  // fade out alarm animation
  }
}

static void buzzer_tick_cb(lv_timer_t *t)
{
  ledcWrite(BUZZER_PIN, BUZZ_STEPS[buzzer_step].on ? 128 : 0);
  lv_timer_set_period(t, BUZZ_STEPS[buzzer_step].ms);
  buzzer_step++;
  if (buzzer_step >= 9) {
    buzzer_step = 0;
    buzzer_seq_done++;
    if (buzzer_seq_total > 0 && buzzer_seq_done >= buzzer_seq_total)
      buzzer_stop();
  }
}

static void buzzer_start(int sequences)
{
  if (buzzer_active) buzzer_stop();
  buzzer_seq_total = sequences;
  buzzer_seq_done  = 0;
  buzzer_step      = 0;
  ledcChangeFrequency(BUZZER_PIN, 2000, 8);  // ensure 2kHz alarm freq
  buzzer_active = true;
  buzzer_timer  = lv_timer_create(buzzer_tick_cb, 200, nullptr);
  Serial.printf("[BUZZ] Started (%d sequences)\n", sequences);
}

static void buzzer_start_alarm()
{
  buzzer_fade_after = (cfg.alarm_beep_sequences > 0); // only when finite
  buzzer_start(cfg.alarm_beep_sequences);
}

// This will keep the animation on screen after the alarm is done
// static void buzzer_start_timer() { buzzer_start(cfg.timer_beep_sequences); }

// This fades the animation after the alarm is done
static void buzzer_start_timer()
{
  buzzer_fade_after = (cfg.timer_beep_sequences > 0);
  buzzer_start(cfg.timer_beep_sequences);
}

// ══════════════════════════════════════════════════════════════════════════════
//  BACKLIGHT PWM
// ══════════════════════════════════════════════════════════════════════════════
void backlight_init()
{
  ledcAttach(GFX_BL, BL_PWM_FREQ, BL_PWM_RESOLUTION);
  // Apply boot brightness (50%)
  uint8_t pwm = (255 * brightnessPercent) / 100;
  ledcWrite(GFX_BL, pwm);
  Serial.printf("[BL] Backlight init at %d%% (PWM=%d)\n", brightnessPercent, pwm);
}

void set_brightness(int percent)
{
  if (percent < 1)   percent = 1;
  if (percent > 100) percent = 100;
  brightnessPercent = percent;
  uint8_t pwm = (255 * percent) / 100;
  ledcWrite(GFX_BL, pwm);
  Serial.printf("[BL] Brightness %d%% (PWM=%d)\n", percent, pwm);
  // Update label if status screen is open
  if (label_brightness) {
    lv_label_set_text_fmt(label_brightness, LV_SYMBOL_IMAGE "  Brightness: %d%%", percent);
  }
}

// ══════════════════════════════════════════════════════════════════════════════
//  CONFIG.INI PARSER
//  Reads /config.ini from SD card. Syntax:
//    [section]
//    key = value   (whitespace around = is stripped)
//    # comment     (lines starting with # or ; are ignored)
// ══════════════════════════════════════════════════════════════════════════════

// Trim leading and trailing whitespace in-place, return pointer to first char
static char *ini_trim(char *s)
{
  while (*s == ' ' || *s == '\t') s++;
  char *end = s + strlen(s) - 1;
  while (end > s && (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n'))
    *end-- = '\0';
  return s;
}

static void load_config()
{
  Serial.println("[CFG] Loading /config.ini...");

  File f = SD.open("/config.ini", FILE_READ);
  if (!f) {
    Serial.println("[CFG] config.ini not found — using defaults.");
    return;
  }

  char   line[128];
  char   section[32] = "";
  int    lineNum = 0;

  while (f.available()) {
    // Read one line
    int len = 0;
    while (f.available() && len < (int)sizeof(line) - 1) {
      char ch = f.read();
      if (ch == '\n') break;
      line[len++] = ch;
    }
    line[len] = '\0';
    lineNum++;

    char *p = ini_trim(line);

    // Skip blank lines and comments
    if (*p == '\0' || *p == '#' || *p == ';') continue;

    // Section header  [section]
    if (*p == '[') {
      char *end = strchr(p, ']');
      if (end) {
        *end = '\0';
        strncpy(section, p + 1, sizeof(section) - 1);
        section[sizeof(section) - 1] = '\0';
        ini_trim(section);
      }
      continue;
    }

    // key = value
    char *eq = strchr(p, '=');
    if (!eq) continue;
    *eq = '\0';
    char *key = ini_trim(p);
    char *val = ini_trim(eq + 1);

    // ── [wifi] ──────────────────────────────────────────────────────────────
    if (strcmp(section, "wifi") == 0) {
      if (strcmp(key, "ssid") == 0) {
        strncpy(cfg.wifi_ssid, val, sizeof(cfg.wifi_ssid) - 1);
        Serial.printf("[CFG]   wifi.ssid     = %s\n", cfg.wifi_ssid);
      }
      else if (strcmp(key, "password") == 0) {
        strncpy(cfg.wifi_password, val, sizeof(cfg.wifi_password) - 1);
        Serial.println("[CFG]   wifi.password = (hidden)");
      }
    }

    // ── [clock] — NTP server + POSIX timezone string ─────────────────────
    else if (strcmp(section, "clock") == 0) {
      if (strcmp(key, "ntp_server") == 0) {
        strncpy(cfg.ntp_server, val, sizeof(cfg.ntp_server) - 1);
        Serial.printf("[CFG]   clock.ntp_server  = %s\n", cfg.ntp_server);
      }
      else if (strcmp(key, "tz") == 0) {
        strncpy(cfg.tz_string, val, sizeof(cfg.tz_string) - 1);
        cfg.tz_string[sizeof(cfg.tz_string) - 1] = '\0';
        Serial.printf("[CFG]   clock.tz           = %s\n", cfg.tz_string);
      }
    }

    // ── [wifi] extra keys ────────────────────────────────────────────────────
    if (strcmp(section, "wifi") == 0 && strcmp(key, "enabled") == 0) {
      cfg.wifi_enabled = (strcmp(val,"true")==0||strcmp(val,"1")==0);
      Serial.printf("[CFG]   wifi.enabled       = %s\n", cfg.wifi_enabled?"true":"false");
    }

    // ── [alarm] ─────────────────────────────────────────────────────────────
    else if (strcmp(section, "alarm") == 0) {
      if (strcmp(key, "enabled") == 0) {
        cfg.alarm_enabled = (strcmp(val,"true")==0||strcmp(val,"1")==0);
        Serial.printf("[CFG]   alarm.enabled      = %s\n", cfg.alarm_enabled?"true":"false");
      }
      else if (strcmp(key, "time") == 0) {
        int h=0,m=0;
        if (sscanf(val,"%d:%d",&h,&m)==2) {
          cfg.alarm_hour=h; cfg.alarm_minute=m;
          Serial.printf("[CFG]   alarm.time         = %02d:%02d\n",h,m);
        }
      }
      else if (strcmp(key,"beep_sequences")==0) {
        cfg.alarm_beep_sequences=atoi(val);
        Serial.printf("[CFG]   alarm.beep_sequences = %d\n",cfg.alarm_beep_sequences);
      }
    }

    // ── [timer] ─────────────────────────────────────────────────────────────
    else if (strcmp(section,"timer")==0) {
      if (strcmp(key,"duration")==0) {
        int h=0,m=0;
        if (sscanf(val,"%d:%d",&h,&m)==2) {
          cfg.timer_hours=h; cfg.timer_minutes=m;
          Serial.printf("[CFG]   timer.duration      = %02d:%02d\n",h,m);
        }
      }
      else if (strcmp(key,"beep_sequences")==0) {
        cfg.timer_beep_sequences=atoi(val);
        Serial.printf("[CFG]   timer.beep_sequences = %d\n",cfg.timer_beep_sequences);
      }
    }

    // ── [animation] ─────────────────────────────────────────────────────
    else if (strcmp(section, "animation") == 0) {
      if (strcmp(key, "schedule") == 0) {
        cfg.anim_schedule_enabled = (strcmp(val, "true") == 0 || strcmp(val, "1") == 0);
        Serial.printf("[CFG]   anim.schedule  = %s\n",
                      cfg.anim_schedule_enabled ? "true" : "false");
      }
      else if (strcmp(key, "duration") == 0) {
        cfg.anim_duration_sec = atoi(val);
        if (cfg.anim_duration_sec < 3)  cfg.anim_duration_sec = 3;
        if (cfg.anim_duration_sec > 60) cfg.anim_duration_sec = 60;
        Serial.printf("[CFG]   anim.duration  = %ds\n", cfg.anim_duration_sec);
      }
    }

    // ── [menu] ─────────────────────────────────────────────────────────────
    else if (strcmp(section, "menu") == 0) {
      if (strcmp(key, "sounds") == 0) {
        cfg.menu_sounds = (strcmp(val,"true")==0||strcmp(val,"1")==0);
        Serial.printf("[CFG]   menu.sounds    = %s\n", cfg.menu_sounds?"true":"false");
      }
    }
  }

  f.close();
  Serial.printf("[CFG] Done. (%d lines read)\n", lineNum);
}

static void restore_time_from_log() {
  if (!sdCardAvailable) return;

  File f = SD.open("/last_seen.txt", FILE_READ);
  if (!f) {
    Serial.println("[RTC] No last_seen.txt found. Skipping restore.");
    return;
  }

  // The log format is: "YYYY-MM-DD HH:MM:SS (X.XXV)\n" (~30 chars)
  // We'll read the last 128 bytes to ensure we capture the full last line.
  size_t size = f.size();
  if (size < 19) { // Too small to contain a timestamp
    f.close();
    return;
  }

  size_t seekPos = (size > 128) ? size - 128 : 0;
  f.seek(seekPos);

  char buffer[129];
  size_t bytesRead = f.readBytes(buffer, 128);
  buffer[bytesRead] = '\0';
  f.close();

  // Find the last occurrence of a year-like string (starts with "20")
  char *lastLine = nullptr;
  char *ptr = strstr(buffer, "20"); 
  while (ptr != nullptr) {
    lastLine = ptr;
    ptr = strstr(ptr + 1, "20");
  }

  if (lastLine) {
    int yr, mn, dy, hr, min, sec;
    // Parse: 2026-03-23 15:30:45
    if (sscanf(lastLine, "%d-%02d-%02d %02d:%02d:%02d", &yr, &mn, &dy, &hr, &min, &sec) == 6) {
      struct tm tm_new;
      tm_new.tm_year = yr - 1900;
      tm_new.tm_mon  = mn - 1;
      tm_new.tm_mday = dy;
      tm_new.tm_hour = hr;
      tm_new.tm_min  = min;
      tm_new.tm_sec  = sec;
      tm_new.tm_isdst = -1;

      // TZ env var is set before this runs; mktime converts local→UTC correctly
      time_t t = mktime(&tm_new);
      if (t > 0) {
        struct timeval now_tv = { .tv_sec = t, .tv_usec = 0 };
        settimeofday(&now_tv, NULL);
        Serial.printf("[RTC] Restored UTC time from log: %04d-%02d-%02d %02d:%02d:%02d\n", 
                      yr, mn, dy, hr, min, sec);
        return;
      }
    }
  }
  Serial.println("[RTC] Could not parse last timestamp from log.");
}

static void log_last_seen() {
  // 1. Voltage Check: Measure battery voltage
  uint16_t mv = analogReadMilliVolts(BAT_PIN);
  float voltage = mv * 3.0f / 1000.0f;
  
  // Stop writing if voltage is below 3.4V
  if (voltage < 3.4f) {
    Serial.printf("[LOG] Battery low (%.2fV). Logging skipped.\n", voltage);
    return;
  }

  // 2. Get current time
  time_t now = time(nullptr);
  struct tm t;
  gmtime_r(&now, &t);

  // Only log if time is synced (sane year > 2026)
  if (now < 1735689600UL) return; 

  // 3. Format timestamp: YYYY-MM-DD HH:MM:SS
  char log_buf[32];
  snprintf(log_buf, sizeof(log_buf), "%04d-%02d-%02d %02d:%02d:%02dZ (%.2fV)\n",
           t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
           t.tm_hour, t.tm_min, t.tm_sec, voltage);

  // 4. Append Mode: Write to last_seen.txt
  // FILE_WRITE on ESP32 SD library defaults to appending/creating if it exists.
  File logFile = SD.open("/last_seen.txt", FILE_APPEND);
  if (logFile) {
    logFile.print(log_buf);
    logFile.close();
    Serial.print("[LOG] Appended: ");
    Serial.print(log_buf);
  } else {
    Serial.println("[LOG] Failed to open last_seen.txt");
  }
}

static void save_config()
{
  // Re-read existing config line by line, rewrite with updated [alarm] section.
  // We load all non-alarm lines into a buffer then append the updated alarm block.
  // This preserves [wifi] and any comments in the original file.
  const char *path = "/config.ini";
  char lines[32][128];
  int  lineCount = 0;
  bool inAlarm   = false;  // also covers [wifi] and [timer]

  File fr = SD.open(path, FILE_READ);
  if (fr) {
    while (fr.available() && lineCount < 30) {
      int len = 0;
      while (fr.available() && len < 126) {
        char ch = fr.read();
        if (ch == '\n') break;
        lines[lineCount][len++] = ch;
      }
      lines[lineCount][len] = '\0';
      // Detect [alarm] section and skip its lines — we'll rewrite them fresh
      char *trimmed = lines[lineCount];
      while (*trimmed == ' ' || *trimmed == '\t') trimmed++;
      if (strncmp(trimmed,"[wifi]", 6)==0||
          strncmp(trimmed,"[alarm]",7)==0||
          strncmp(trimmed,"[timer]",7)==0||
          strncmp(trimmed,"[menu]", 6)==0) { inAlarm=true; continue; }
      if (*trimmed=='[') inAlarm=false;
      if (!inAlarm) lineCount++;
    }
    fr.close();
  }

  File fw = SD.open(path, FILE_WRITE);
  if (!fw) { Serial.println("[CFG] save_config: cannot open for write"); return; }

  // Write preserved lines
  for (int i = 0; i < lineCount; i++) {
    fw.println(lines[i]);
  }
  fw.println();
  fw.println("[wifi]");
  fw.printf("enabled = %s\n",  cfg.wifi_enabled ?"true":"false");
  fw.printf("ssid = %s\n",     cfg.wifi_ssid);
  fw.printf("password = %s\n", cfg.wifi_password);

  fw.println();
  fw.println("[alarm]");
  fw.printf("enabled = %s\n",        cfg.alarm_enabled?"true":"false");
  fw.printf("time = %02d:%02d\n",    cfg.alarm_hour, cfg.alarm_minute);
  fw.printf("beep_sequences = %d\n", cfg.alarm_beep_sequences);

  fw.println();
  fw.println("[timer]");
  fw.printf("duration = %02d:%02d\n",  cfg.timer_hours, cfg.timer_minutes);
  fw.printf("beep_sequences = %d\n",   cfg.timer_beep_sequences);

  fw.println();
  fw.println("[menu]");
  fw.printf("sounds = %s\n", cfg.menu_sounds?"true":"false");
  fw.close();

  Serial.println("[CFG] Saved wifi/alarm/timer/menu.");
  
  // Save the current timestamp to the log file as well
  log_last_seen();
}

// ── WiFi runtime toggle ───────────────────────────────────────────────────────
static void apply_wifi_state()
{
  // Apply POSIX TZ immediately — makes localtime_r correct even offline
  setenv("TZ", cfg.tz_string, 1);
  tzset();
  Serial.printf("[TZ] Applied: %s\n", cfg.tz_string);

  if (cfg.wifi_enabled) {
    Serial.println("[WiFi] Enabling...");
    WiFi.mode(WIFI_STA);
    wifiMulti.addAP(cfg.wifi_ssid, cfg.wifi_password);
    // configTzTime sets TZ env var AND starts SNTP in one call.
    // NTP delivers UTC; localtime_r converts to local using tz_string.
    configTzTime(cfg.tz_string, cfg.ntp_server);
  } else {
    Serial.println("[WiFi] Disabled by user.");
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    wifiConnected = false;
    timeSynced    = false;
  }
}

// ── Countdown timer ───────────────────────────────────────────────────────────
static void countdown_tick_cb(lv_timer_t * /*t*/)
{
  if (!timer_running || countdown_sec <= 0) return;
  countdown_sec--;
  if (home_timer_lbl) {
    int h=countdown_sec/3600, m=(countdown_sec%3600)/60, s=countdown_sec%60;
    if (h > 0)
      lv_label_set_text_fmt(home_timer_lbl, LV_SYMBOL_STOP " %d:%02d:%02d",h,m,s);
    else
      lv_label_set_text_fmt(home_timer_lbl, LV_SYMBOL_STOP " %02d:%02d",m,s);
  }
  if (countdown_sec == 0) {
    timer_running = false;
    if (countdown_timer) { lv_timer_del(countdown_timer); countdown_timer=nullptr; }
    Serial.println("[TIMER] Done.");
    // Hide the 00:00 label immediately — GIF animation takes over
    if (home_timer_lbl) lv_obj_add_flag(home_timer_lbl, LV_OBJ_FLAG_HIDDEN);
    close_scheduled_gif();  // evict any running scheduled animation
    show_gif_fullscreen(GIF_TIMER_PATH);
    buzzer_start_timer();
  }
}

static void timer_start_countdown()
{
  countdown_sec = cfg.timer_hours*3600 + cfg.timer_minutes*60;
  if (countdown_sec <= 0) return;
  if (countdown_timer) { lv_timer_del(countdown_timer); countdown_timer=nullptr; }
  timer_running   = true;
  countdown_timer = lv_timer_create(countdown_tick_cb, 1000, nullptr);
  Serial.printf("[TIMER] Started %02d:%02d (%ds)\n",
                cfg.timer_hours, cfg.timer_minutes, countdown_sec);
  if (home_timer_lbl) {
    int m=countdown_sec/60, s=countdown_sec%60;
    lv_label_set_text_fmt(home_timer_lbl, LV_SYMBOL_STOP " %02d:%02d",m,s);
    lv_obj_clear_flag(home_timer_lbl, LV_OBJ_FLAG_HIDDEN);
  }
}

static void timer_stop()
{
  if (countdown_timer) { lv_timer_del(countdown_timer); countdown_timer=nullptr; }
  timer_running = false;
  if (home_timer_lbl) lv_obj_add_flag(home_timer_lbl, LV_OBJ_FLAG_HIDDEN);
}

// ══════════════════════════════════════════════════════════════════════════════
//  LCD REGISTER INIT  (unchanged from original)
// ══════════════════════════════════════════════════════════════════════════════
void lcd_reg_init(void)
{
  static const uint8_t init_operations[] = {
    BEGIN_WRITE,
    WRITE_COMMAND_8, 0x11,
    END_WRITE,
    DELAY, 120,
    BEGIN_WRITE,
    WRITE_C8_D16, 0xDF, 0x98, 0x53,
    WRITE_C8_D8,  0xB2, 0x23,
    WRITE_COMMAND_8, 0xB7,
    WRITE_BYTES, 4, 0x00, 0x47, 0x00, 0x6F,
    WRITE_COMMAND_8, 0xBB,
    WRITE_BYTES, 6, 0x1C, 0x1A, 0x55, 0x73, 0x63, 0xF0,
    WRITE_C8_D16, 0xC0, 0x44, 0xA4,
    WRITE_C8_D8,  0xC1, 0x16,
    WRITE_COMMAND_8, 0xC3,
    WRITE_BYTES, 8, 0x7D, 0x07, 0x14, 0x06, 0xCF, 0x71, 0x72, 0x77,
    WRITE_COMMAND_8, 0xC4,
    WRITE_BYTES, 12, 0x00, 0x00, 0xA0, 0x79, 0x0B, 0x0A,
                     0x16, 0x79, 0x0B, 0x0A, 0x16, 0x82,
    WRITE_COMMAND_8, 0xC8,
    WRITE_BYTES, 32,
    0x3F,0x32,0x29,0x29,0x27,0x2B,0x27,0x28,
    0x28,0x26,0x25,0x17,0x12,0x0D,0x04,0x00,
    0x3F,0x32,0x29,0x29,0x27,0x2B,0x27,0x28,
    0x28,0x26,0x25,0x17,0x12,0x0D,0x04,0x00,
    WRITE_COMMAND_8, 0xD0,
    WRITE_BYTES, 5, 0x04, 0x06, 0x6B, 0x0F, 0x00,
    WRITE_C8_D16, 0xD7, 0x00, 0x30,
    WRITE_C8_D8,  0xE6, 0x14,
    WRITE_C8_D8,  0xDE, 0x01,
    WRITE_COMMAND_8, 0xB7,
    WRITE_BYTES, 5, 0x03, 0x13, 0xEF, 0x35, 0x35,
    WRITE_COMMAND_8, 0xC1,
    WRITE_BYTES, 3, 0x14, 0x15, 0xC0,
    WRITE_C8_D16, 0xC2, 0x06, 0x3A,
    WRITE_C8_D16, 0xC4, 0x72, 0x12,
    WRITE_C8_D8,  0xBE, 0x00,
    WRITE_C8_D8,  0xDE, 0x02,
    WRITE_COMMAND_8, 0xE5,
    WRITE_BYTES, 3, 0x00, 0x02, 0x00,
    WRITE_COMMAND_8, 0xE5,
    WRITE_BYTES, 3, 0x01, 0x02, 0x00,
    WRITE_C8_D8,  0xDE, 0x00,
    WRITE_C8_D8,  0x35, 0x00,
    WRITE_C8_D8,  0x3A, 0x05,
    WRITE_COMMAND_8, 0x2A,
    WRITE_BYTES, 4, 0x00, 0x22, 0x00, 0xCD,
    WRITE_COMMAND_8, 0x2B,
    WRITE_BYTES, 4, 0x00, 0x00, 0x01, 0x3F,
    WRITE_C8_D8,  0xDE, 0x02,
    WRITE_COMMAND_8, 0xE5,
    WRITE_BYTES, 3, 0x00, 0x02, 0x00,
    WRITE_C8_D8,  0xDE, 0x00,
    WRITE_C8_D8,  0x36, 0x00,
    WRITE_COMMAND_8, 0x21,
    END_WRITE,
    DELAY, 10,
    BEGIN_WRITE,
    WRITE_COMMAND_8, 0x29,
    END_WRITE
  };
  bus->batchOperation(init_operations, sizeof(init_operations));
}

// ══════════════════════════════════════════════════════════════════════════════
//  LVGL DISPLAY + INPUT CALLBACKS
// ══════════════════════════════════════════════════════════════════════════════
#if LV_USE_LOG != 0
void my_print(lv_log_level_t level, const char *buf) { Serial.print(buf); Serial.flush(); }
#endif

void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
  uint32_t w = area->x2 - area->x1 + 1;
  uint32_t h = area->y2 - area->y1 + 1;
  // px_map is raw pixel bytes; cast to uint16_t* for RGB565
  gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)px_map, w, h);
  lv_display_flush_ready(disp);  // v9: lv_display_flush_ready (not lv_disp_flush_ready)
}

void touchpad_read_cb(lv_indev_t * /*indev*/, lv_indev_data_t *data)
{
  touch_data_t touch_data;
  bsp_touch_read();
  bool pressed = bsp_touch_get_coordinates(&touch_data);
  if (pressed) {
    data->point.x = touch_data.coords[0].x;
    data->point.y = touch_data.coords[0].y;
    data->state   = LV_INDEV_STATE_PRESSED;
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

// ══════════════════════════════════════════════════════════════════════════════
// ══════════════════════════════════════════════════════════════════════════════
//  BATTERY HELPERS
// ══════════════════════════════════════════════════════════════════════════════

// LiPo discharge curve: 4.20V=100% → 3.00V=0%
// ADC reads through a ÷3 divider: actual_V = analogReadMilliVolts × 3 / 1000
static int battery_voltage_to_percent(float v)
{
  if (v >= 4.20f) return 100;
  if (v >= 4.00f) return 80  + (int)((v - 4.00f) / 0.20f * 20.f);
  if (v >= 3.80f) return 50  + (int)((v - 3.80f) / 0.20f * 30.f);
  if (v >= 3.60f) return 20  + (int)((v - 3.60f) / 0.20f * 30.f);
  if (v >= 3.00f) return      (int)((v - 3.00f) / 0.60f * 20.f);
  return 0;
}

static const char *battery_icon(int pct)
{
  if (pct >= 75) return LV_SYMBOL_BATTERY_FULL;
  if (pct >= 50) return LV_SYMBOL_BATTERY_3;
  if (pct >= 25) return LV_SYMBOL_BATTERY_2;
  if (pct >= 10) return LV_SYMBOL_BATTERY_1;
  return LV_SYMBOL_BATTERY_EMPTY;
}

// Read current battery percentage (shared by timer + low-bat check)
static int battery_read_percent()
{
  uint16_t mv = analogReadMilliVolts(BAT_PIN);
  float v     = mv * 3.0f / 1000.0f;
  return battery_voltage_to_percent(v);
}

// ── Shutdown ──────────────────────────────────────────────────────────────────
static void shutdown_cancel()
{
  if (shutdown_timer) { lv_timer_del(shutdown_timer); shutdown_timer = nullptr; }
  if (shutdown_popup) { lv_obj_del(shutdown_popup); shutdown_popup = nullptr; }
  shutdown_cntdown_lbl = nullptr;
  shutdown_count = 5;
}

static void shutdown_cancel_cb(lv_event_t *e)
{
  if (lv_event_get_code(e) == LV_EVENT_CLICKED) shutdown_cancel();
}

static void shutdown_execute()
{
  // ── Wake strategy ────────────────────────────────────────────────────────
  // RESET button (EN pin) always causes a hard reboot — use it any time.
  // Timer wakeup: if an alarm is configured, the device wakes automatically
  // 30 s before alarm time so the boot sequence completes before it fires.
  // If no alarm is set, the device sleeps indefinitely until RESET is pressed.
  //
  Serial.println("[PWR] Entering deep sleep.");
  Serial.println("[PWR] Press RESET button to wake the device.");
  if (cfg.alarm_enabled) {
    time_t now = time(nullptr);
    struct tm t; localtime_r(&now, &t);
    int now_sec  = t.tm_hour * 3600 + t.tm_min * 60 + t.tm_sec;
    int alm_sec  = cfg.alarm_hour * 3600 + cfg.alarm_minute * 60;
    int diff_sec = alm_sec - now_sec;
    if (diff_sec <= 0) diff_sec += 86400;   // alarm is tomorrow
    // If alarm is more than 5 min away: wake 5 min early so WiFi+NTP
    // have time to sync before the alarm fires.
    // If alarm is 5 min or less away: wake 30s early — no time for NTP,
    // the fallback warning alarm will cover any drift.
    const int EARLY_NTP  = 5 * 60;   // 300s = 5 min
    const int EARLY_BOOT =      30;  // 30s  — just enough to boot
    int early = (diff_sec > EARLY_NTP) ? EARLY_NTP : EARLY_BOOT;
    diff_sec = max(diff_sec - early, 10);
    esp_sleep_enable_timer_wakeup((uint64_t)diff_sec * 1000000ULL);
    Serial.printf("[PWR] Timer wakeup set for %ds (%d min early, alarm at %02d:%02d)\n",
                  diff_sec, early / 60, cfg.alarm_hour, cfg.alarm_minute);
  }
  Serial.flush();
  ledcWrite(GFX_BL, 0);  // blank display
  delay(200);
  esp_deep_sleep_start();
}

static void shutdown_tick_cb(lv_timer_t * /*t*/)
{
  shutdown_count--;
  if (shutdown_cntdown_lbl)
    lv_label_set_text_fmt(shutdown_cntdown_lbl,
                          "Shutting down in %d...", shutdown_count);
  if (shutdown_count <= 0) {
    shutdown_cancel();
    shutdown_execute();
  }
}

static void show_shutdown_popup()
{
  if (shutdown_popup) return;
  shutdown_count = 5;

  // Semi-transparent confirmation card centred over the battery screen
  shutdown_popup = lv_obj_create(overlay_cont);
  lv_obj_set_size(shutdown_popup, 240, 90);
  lv_obj_align(shutdown_popup, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_bg_color(shutdown_popup, lv_color_make(20, 20, 40), 0);
  lv_obj_set_style_bg_opa(shutdown_popup, LV_OPA_COVER, 0);
  lv_obj_set_style_border_color(shutdown_popup, lv_color_make(80, 80, 160), 0);
  lv_obj_set_style_border_width(shutdown_popup, 1, 0);
  lv_obj_set_style_radius(shutdown_popup, 8, 0);
  lv_obj_set_style_pad_all(shutdown_popup, 0, 0);
  lv_obj_clear_flag(shutdown_popup, LV_OBJ_FLAG_SCROLLABLE);

  shutdown_cntdown_lbl = lv_label_create(shutdown_popup);
  lv_label_set_text_fmt(shutdown_cntdown_lbl,
                        "Shutting down in %d...", shutdown_count);
  lv_obj_set_style_text_font(shutdown_cntdown_lbl, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(shutdown_cntdown_lbl, lv_color_make(220, 220, 255), 0);
  lv_obj_align(shutdown_cntdown_lbl, LV_ALIGN_CENTER, 0, -18);

  lv_obj_t *btn = lv_btn_create(shutdown_popup);
  lv_obj_set_size(btn, 90, 28);
  lv_obj_align(btn, LV_ALIGN_CENTER, 0, 22);
  lv_obj_set_style_bg_color(btn, lv_color_make(180, 60, 60), 0);
  lv_obj_set_style_radius(btn, 6, 0);
  lv_obj_add_event_cb(btn, shutdown_cancel_cb, LV_EVENT_CLICKED, nullptr);

  lv_obj_t *btn_lbl = lv_label_create(btn);
  lv_label_set_text(btn_lbl, "Cancel");
  lv_obj_set_style_text_font(btn_lbl, &lv_font_montserrat_14, 0);
  lv_obj_center(btn_lbl);

  shutdown_timer = lv_timer_create(shutdown_tick_cb, 1000, nullptr);
}

static void battery_longpress_cb(lv_event_t *e)
{
  if (lv_event_get_code(e) != LV_EVENT_LONG_PRESSED) return;
  lv_indev_wait_release(lv_indev_get_act());
  show_shutdown_popup();
}

// ══════════════════════════════════════════════════════════════════════════════
//  BATTERY TIMER
// ══════════════════════════════════════════════════════════════════════════════
static void battery_timer_callback(lv_timer_t * /*timer*/)
{
  char     buf[24];
  uint16_t raw   = analogRead(BAT_PIN);
  uint16_t mv    = analogReadMilliVolts(BAT_PIN);
  float    volts = mv * 3.0f / 1000.0f;
  int      pct   = battery_voltage_to_percent(volts);

  // ── Update battery screen labels if open ────────────────────────────────
  if (label_adc_raw) lv_label_set_text_fmt(label_adc_raw, "%d", raw);
  if (label_voltage) {
    snprintf(buf, sizeof(buf), "%.2f V", volts);
    lv_label_set_text(label_voltage, buf);
  }
  if (label_percent)
    lv_label_set_text_fmt(label_percent, "%s  %d%%", battery_icon(pct), pct);

  // ── Home screen clock colour by battery level ────────────────────────────
  //   > 25%  : white  (normal)
  //   11-25% : orange (low warning)
  //   ≤ 10%  : red    (critical — triggers auto-poweroff after 60 s)
  if (home_time_lbl) {
    lv_color_t clr = lv_color_white();
    if      (pct <= 10) clr = lv_color_make(220, 50,  50);   // red
    else if (pct <= 25) clr = lv_color_make(255, 165,  0);   // orange
    lv_obj_set_style_text_color(home_time_lbl, clr, 0);
  }

  // ── Auto-poweroff at ≤ 10% ───────────────────────────────────────────────
  // Uses a one-shot flag so we only trigger once per session, not every second.
  static bool low_bat_triggered = false;
  if (pct <= 10 && !low_bat_triggered && !overlay_cont && !alarm_cont) {
    low_bat_triggered = true;
    Serial.printf("[BAT] Critical: %d%% — auto-poweroff in 60s\n", pct);
    // Open battery screen so the user sees the warning
    show_battery_screen();
    // Start shutdown countdown at 60 seconds instead of the normal 5
    shutdown_count = 60;
    show_shutdown_popup();
    // Override the countdown label to explain why
    if (shutdown_cntdown_lbl)
      lv_label_set_text_fmt(shutdown_cntdown_lbl,
        "Low battery (%d%%)!\nSleeping in %ds...", pct, shutdown_count);
  }
}

// ══════════════════════════════════════════════════════════════════════════════
//  WIFI + NTP BACKGROUND POLL TIMER  (every 5 s)
//  Runs entirely from the LVGL timer so it never blocks the display.
// ══════════════════════════════════════════════════════════════════════════════
static void wifi_poll_cb(lv_timer_t *t)
{
  wl_status_t wst = WiFi.status();  // instant read, never blocks
  if (wst == WL_CONNECTED) {
    wifiConnected = true;
    time_t now = time(nullptr);
    timeSynced = (now >= 8 * 3600 * 2);
    lv_timer_set_period(t, 5000);   // back to 5s when connected
  } else {
    wifiConnected = false;
    timeSynced    = false;
    // wifiMulti.run() briefly takes the radio lock and stalls the
    // FreeRTOS scheduler for 20-80ms, causing visible UI stutter.
    // Only attempt reconnect when no modal/editor is open, and slow
    // down to every 30s so the stall is infrequent.
    if (!modal_cont && !overlay_cont && !apps_cont && cfg.wifi_enabled) {
      wifiMulti.run(0);
    }
    lv_timer_set_period(t, 30000);  // 30s between reconnect attempts
  }
}

// ══════════════════════════════════════════════════════════════════════════════
//  SCHEDULED ANIMATION  —  every 2 min, configurable duration, 800 ms fade
//  Day (07:00–19:59): smile GIF.  Night (20:00–06:59): sleep GIF.
//  Only fires when the clock face is visible and no overlay is open.
//  Touch the screen at any time to dismiss immediately.
// ══════════════════════════════════════════════════════════════════════════════
static bool is_night_time(int hour)
{
  return (hour >= 20 || hour < 7);
}

static void sched_gif_close_cb(lv_timer_t *t)
{
  lv_timer_del(t);
  sched_close_timer = nullptr;
  if (!overlay_cont) return;

  lv_anim_t a;
  lv_anim_init(&a);
  lv_anim_set_var(&a, overlay_cont);
  lv_anim_set_exec_cb(&a, [](void *obj, int32_t v) {
    if (obj) lv_obj_set_style_opa((lv_obj_t *)obj, (lv_opa_t)v, 0);
  });
  lv_anim_set_values(&a, LV_OPA_COVER, LV_OPA_TRANSP);
  lv_anim_set_duration(&a, 800);
  lv_anim_set_ready_cb(&a, [](lv_anim_t * /*a*/) {
    if (overlay_cont) {
      if (tilt_timer) { lv_timer_del(tilt_timer); tilt_timer = nullptr; }
      label_brightness = nullptr;
      lv_obj_del(overlay_cont);
      overlay_cont = nullptr;
      Serial.println("[ANIM] Scheduled GIF closed (fade)");
    }
  });
  lv_anim_start(&a);
}

// Force-close a scheduled GIF overlay so alarm/timer can take priority.
// Cancels the fade timer and deletes the overlay synchronously.
static void close_scheduled_gif()
{
  if (sched_close_timer) {
    lv_timer_del(sched_close_timer);
    sched_close_timer = nullptr;
  }
  if (overlay_cont) {
    lv_obj_del(overlay_cont);
    overlay_cont = nullptr;
    Serial.println("[SCHED] Scheduled GIF closed — alarm/timer taking priority");
  }
}

static void run_scheduled_animation(int hour)
{
  if (!cfg.anim_schedule_enabled) return;
  if (!sdCardAvailable)           return;
  if (overlay_cont)               return;  // another screen already open
  if (!home_time_lbl)             return;  // clock face not visible yet

  const char *path = is_night_time(hour) ? GIF_SLEEP_PATH : GIF_SMILE_PATH;
  Serial.printf("[ANIM] %s GIF for %ds\n",
                is_night_time(hour) ? "sleep" : "smile", cfg.anim_duration_sec);

  show_gif_fullscreen(path);

  if (overlay_cont) {
    sched_close_timer = lv_timer_create(sched_gif_close_cb,
                                         (uint32_t)cfg.anim_duration_sec * 1000UL,
                                         nullptr);
    lv_timer_set_repeat_count(sched_close_timer, 1);
  }
}

// ══════════════════════════════════════════════════════════════════════════════
//  DAILY AUTOMATION  —  brightness schedule + sleep GIF
//  Called once per minute from clock_tick_cb (only when minute changes).
// ══════════════════════════════════════════════════════════════════════════════
static void run_daily_automation(int hour, int minute)
{
  Serial.printf("[SCHED] %02d:%02d\n", hour, minute);

  // ── Scheduled animation every 2 minutes ──────────────────────────────────
  // Skip if alarm fires this same minute — alarm takes priority
  bool alarm_fires_now = cfg.alarm_enabled
                         && hour   == cfg.alarm_hour
                         && minute == cfg.alarm_minute;
  if (minute % 2 == 0 && !alarm_fires_now) run_scheduled_animation(hour);

  // ── Evening dimming ───────────────────────────────────────────────────────
  if (hour == 19 && minute ==  0) set_brightness(25);
  if (hour == 19 && minute == 30) set_brightness(10);
  if (hour == 20 && minute ==  0) set_brightness(1);

  // ── Sleep animation: auto-start at 20:15 ─────────────────────────────────
  if (hour == 20 && minute == 15) {
    Serial.println("[SCHED] Starting sleep animation");
    show_gif_fullscreen(GIF_SLEEP_PATH);   // opens overlay if not already open
  }

  // ── Sleep animation: auto-stop at 21:00 if still playing ─────────────────
  if (hour == 21 && minute ==  0) {
    if (overlay_cont) {
      Serial.println("[SCHED] Auto-closing sleep animation at 21:00");
      // Simulate a close — reuse the same logic as tapping to return
      if (tilt_timer) { lv_timer_del(tilt_timer); tilt_timer = nullptr; }
      label_brightness = nullptr;
      lv_obj_del(overlay_cont);
      overlay_cont = nullptr;
    }
  }

  // ── Morning brightness ramp ───────────────────────────────────────────────
  if (hour ==  6 && minute ==  0) set_brightness(10);
  if (hour ==  6 && minute == 30) set_brightness(25);
  if (hour ==  7 && minute ==  0) set_brightness(50);

  // ── Alarm (from config.ini [alarm]) ──────────────────────────────────────
  //
  // NTP-guard logic after deep-sleep wakeup:
  //
  //  A) Uptime > 5 min  → device was already running before alarm time.
  //     Time is assumed reliable. Fire normally.
  //
  //  B) Uptime ≤ 5 min AND alarm is HH:MM match:
  //     B1) NTP synced   → time is accurate. Fire normally.
  //     B2) NTP not synced yet → hold in alarm_ntp_pending = true.
  //         The pending check runs every minute until NTP syncs or
  //         15 min have passed (giving up with a warning alarm).
  //
  //  C) Uptime ≤ 5 min AND alarm is NOT yet at HH:MM (woke 5 min early)
  //     → normal pre-alarm window, let automation continue.
  //
  const uint32_t UPTIME_MS       = millis() - boot_millis;
  const uint32_t FIVE_MIN_MS     = 5UL * 60 * 1000;
  const uint32_t NTP_GIVE_UP_MS  = 15UL * 60 * 1000; // 15 min total uptime
  bool           at_alarm_time   = cfg.alarm_enabled
                                   && hour   == cfg.alarm_hour
                                   && minute == cfg.alarm_minute;

  // ── Pending alarm: check each minute whether NTP has synced ──────────
  if (alarm_ntp_pending) {
    if (timeSynced) {
      // NTP finally synced — fire alarm now at the correct time
      alarm_ntp_pending = false;
      Serial.println("[ALARM] NTP synced — firing pending alarm");
      close_scheduled_gif();
      set_brightness(50);
      show_gif_fullscreen(GIF_ALARM_PATH);
      buzzer_start_alarm();
    } else if (UPTIME_MS > NTP_GIVE_UP_MS) {
      // 15 min elapsed, NTP never synced — show warning alarm
      alarm_ntp_pending = false;
      Serial.println("[ALARM] NTP timeout — showing warning alarm");
      close_scheduled_gif();
      set_brightness(80);
      // Warning overlay: no GIF, just text
      if (!overlay_cont) {
        overlay_cont = make_overlay(lv_color_make(20, 20, 20));
        lv_obj_t *h = lv_label_create(overlay_cont);
        lv_label_set_text(h, "HELLO!");
        lv_obj_set_style_text_font(h, &lv_font_montserrat_48, 0);
        lv_obj_set_style_text_color(h, lv_color_white(), 0);
        lv_obj_align(h, LV_ALIGN_CENTER, 0, -24);
        lv_obj_t *w = lv_label_create(overlay_cont);
        lv_label_set_text(w, "NTP not in sync\nCheck the time!");
        lv_obj_set_style_text_font(w, &lv_font_montserrat_16, 0);
        lv_obj_set_style_text_color(w, lv_color_make(255, 180, 60), 0);
        lv_label_set_long_mode(w, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(w, 280);
        lv_obj_set_style_text_align(w, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_align(w, LV_ALIGN_CENTER, 0, 36);
        lv_obj_add_event_cb(overlay_cont, overlay_close_event_cb, LV_EVENT_CLICKED, nullptr);
      }
      buzzer_start_alarm();
    } else {
      Serial.printf("[ALARM] Pending — waiting for NTP (uptime %lus)\n",
                    UPTIME_MS / 1000);
    }
    return;  // pending state consumed this minute
  }

  // ── Normal alarm match ────────────────────────────────────────────────
  if (at_alarm_time) {
    bool long_uptime = (UPTIME_MS > FIVE_MIN_MS);
    if (long_uptime || timeSynced) {
      // Case A or B1: reliable time — fire immediately
      Serial.printf("[ALARM] Firing at %02d:%02d (uptime=%lus, NTP=%s)\n",
                    hour, minute, UPTIME_MS / 1000, timeSynced ? "yes" : "no");
      close_scheduled_gif();
      set_brightness(50);
      show_gif_fullscreen(GIF_ALARM_PATH);
      buzzer_start_alarm();
    } else {
      // Case B2: fresh boot, NTP not yet synced — hold
      alarm_ntp_pending = true;
      Serial.printf("[ALARM] Holding at %02d:%02d — waiting for NTP sync\n",
                    hour, minute);
    }
  }
}

// ══════════════════════════════════════════════════════════════════════════════
//  CLOCK TICK  (every 1 s — updates HH:mm on home screen)
// ══════════════════════════════════════════════════════════════════════════════
static void clock_tick_cb(lv_timer_t * /*t*/)
{
  if (!home_time_lbl) return;
  time_t    now = time(nullptr);
  struct tm tm_info;
  localtime_r(&now, &tm_info);

  // Only redraw when the minute changes — avoids label churn every second
  // which was causing touch-event latency during the LVGL render cycle.
  static int last_min = -1;
  if (tm_info.tm_min == last_min) return;
  last_min = tm_info.tm_min;

  // Update clock display
  char buf[6];
  snprintf(buf, sizeof(buf), "%02d:%02d", tm_info.tm_hour, tm_info.tm_min);
  lv_label_set_text(home_time_lbl, buf);

  // Run daily automation (brightness schedule + sleep GIF) once per minute.
  // Only after NTP is synced so we have a reliable time.
  // if (timeSynced) {
  //   run_daily_automation(tm_info.tm_hour, tm_info.tm_min);
  // }
  if (now > 1735689600UL) {   // RTC holds a sane time (> 2026-01-01)
    run_daily_automation(tm_info.tm_hour, tm_info.tm_min);
  }

  // Hourly Log: Trigger exactly at the start of the hour (00 mins, 00 secs)
  if (tm_info.tm_min == 0 && tm_info.tm_sec == 0) {
    log_last_seen();
  }
}

// ══════════════════════════════════════════════════════════════════════════════
//  OVERLAY HELPERS
// ══════════════════════════════════════════════════════════════════════════════
static void overlay_close_event_cb(lv_event_t *e)
{
  if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
  if (overlay_cont) {
    label_adc_raw = nullptr;
    label_voltage = nullptr;
    label_percent = nullptr;
    // Cancel shutdown countdown if battery screen closed via tap
    if (shutdown_timer) { lv_timer_del(shutdown_timer); shutdown_timer = nullptr; }
    if (shutdown_popup) { lv_obj_del(shutdown_popup); shutdown_popup = nullptr; }
    shutdown_cntdown_lbl = nullptr;
    shutdown_count = 5;
    // Stop tilt timer (status screen brightness or emotion GIF mode)
    if (tilt_timer) {
      lv_timer_del(tilt_timer);
      tilt_timer = nullptr;
    }
    emotion_tilt_active = false;
    emotion_current_gif = nullptr;
    label_brightness = nullptr;  // label is about to be destroyed
    // Cancel scheduled GIF auto-close if user taps during animation
    if (sched_close_timer) { lv_timer_del(sched_close_timer); sched_close_timer = nullptr; }
    // Stop buzzer if alarm was playing (screen touch = acknowledge alarm)
    buzzer_stop();
    lv_obj_del(overlay_cont);  // frees GIF decoder + canvas automatically
    overlay_cont = nullptr;
    Serial.printf("[GIF] closed, heap free=%u\n",
                  (unsigned)heap_caps_get_free_size(MALLOC_CAP_8BIT));
  }
}

static lv_obj_t *make_overlay(lv_color_t bg_color)
{
  lv_obj_t *cont = lv_obj_create(lv_scr_act());
  lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
  lv_obj_align(cont, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_bg_color(cont, bg_color, 0);
  lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(cont, 0, 0);
  lv_obj_set_style_pad_all(cont, 0, 0);
  lv_obj_set_style_radius(cont, 0, 0);
  lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_event_cb(cont, overlay_close_event_cb, LV_EVENT_CLICKED, nullptr);
  return cont;
}

static void add_back_hint(lv_obj_t *parent)
{
  lv_obj_t *hint = lv_label_create(parent);
  lv_label_set_text(hint, LV_SYMBOL_LEFT " tap to return");
  lv_obj_set_style_text_color(hint, lv_color_make(120, 120, 120), 0);
  lv_obj_set_style_text_opa(hint, LV_OPA_50, 0);
  lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -6);
  lv_obj_add_flag(hint, LV_OBJ_FLAG_IGNORE_LAYOUT);
}

// ══════════════════════════════════════════════════════════════════════════════
//  SUB-SCREENS
// ══════════════════════════════════════════════════════════════════════════════

// ── GIF fullscreen (upper-left = smile, upper-right = sleep) ─────────────────
static void show_gif_fullscreen(const char *path)
{
  if (overlay_cont) return;
  overlay_cont = make_overlay(lv_color_black());
#if LV_USE_GIF
  if (sdCardAvailable) {
    uint32_t free_b  = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    uint32_t largest = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
    Serial.printf("[GIF] heap free=%u  largest=%u\n", free_b, largest);

    // ── RAM requirement ───────────────────────────────────────────────────────
    // LVGL v9 GIF decoder allocates an ARGB8888 canvas = width × height × 4 bytes
    //   320×172 px → 220 160 bytes — DOES NOT FIT on ESP32-C6 (max ~77 KB free)
    //   160× 86 px →  54 880 bytes — fits easily
    //
    // !! YOU MUST RESIZE BOTH GIF FILES ON THE SD CARD TO 160×86 PIXELS !!
    //    Tool: https://ezgif.com/resize
    //    Steps: Upload GIF → Width=160, Height=86, Resize → Download
    //    Save back to SD card overwriting the original filename.
    //
    // The widget is then scaled 2× by LVGL to fill the 320×172 screen.
    // ─────────────────────────────────────────────────────────────────────────

    // 160×86×4 = 54880 bytes canvas + ~20KB decoder overhead = ~75KB needed
    if (largest < 75000) {
      Serial.printf("[GIF] need 75KB, only %u available\n", largest);
      lv_obj_t *err = lv_label_create(overlay_cont);
      lv_label_set_text(err, LV_SYMBOL_WARNING "  Not enough RAM — resize GIF to 160x86");
      lv_obj_set_style_text_color(err, lv_color_white(), 0);
      lv_label_set_long_mode(err, LV_LABEL_LONG_WRAP);
      lv_obj_set_width(err, screenWidth - 20);
      lv_obj_align(err, LV_ALIGN_CENTER, 0, 0);
    } else {
      lv_obj_t *gif = lv_gif_create(overlay_cont);
      lv_obj_set_user_data(overlay_cont, gif);
      lv_gif_set_src(gif, path);

      uint32_t free_a = heap_caps_get_free_size(MALLOC_CAP_8BIT);
      Serial.printf("[GIF] after set_src: free=%u  consumed=%u\n",
                    free_a, free_b - free_a);

      // Scale 2× so a 160×86 source fills the 320×172 screen
      // (if you kept the original 320×172 GIF, remove the lv_image_set_scale line)
      lv_image_set_scale(gif, 512);                 // 256=1x  512=2x
      lv_obj_align(gif, LV_ALIGN_CENTER, 0, 0);
      lv_obj_set_style_pad_all(gif, 0, 0);
      lv_obj_add_flag(gif, LV_OBJ_FLAG_CLICKABLE);
      lv_obj_add_event_cb(gif, overlay_close_event_cb, LV_EVENT_CLICKED, nullptr);
    }
  } else {
    lv_obj_t *err = lv_label_create(overlay_cont);
    lv_label_set_text(err, LV_SYMBOL_WARNING "  SD card not available");
    lv_obj_set_style_text_color(err, lv_color_white(), 0);
    lv_obj_align(err, LV_ALIGN_CENTER, 0, 0);
  }
#else
  lv_obj_t *err = lv_label_create(overlay_cont);
  lv_label_set_text(err, LV_SYMBOL_WARNING "  Set LV_USE_GIF=1 in lv_conf.h");
  lv_obj_set_style_text_color(err, lv_color_white(), 0);
  lv_obj_align(err, LV_ALIGN_CENTER, 0, 0);
#endif
  // add_back_hint(overlay_cont);
}

// ── Tilt poll: runs every 400 ms ONLY while status screen is open ────────────
static void tilt_poll_cb(lv_timer_t * /*t*/)
{
  if (!imuReady || !overlay_cont) return;

  imu.update();
  imu.getAccel(&accelData);

  if (emotion_tilt_active) {
    // ── Emotion GIF mode (upper-left tap) ─────────────────────────────
    // Axes on this board (landscape, screen facing user):
    //   accelX > +0.4  → tilt backwards  → sleep
    //   accelX < -0.4  → tilt forward     → sad
    //   |accelY| > 0.4 → tilt left/right  → joy
    //   all near zero  → upright           → smile
    float x = accelData.accelX;
    float y = accelData.accelY;

    const char *desired = GIF_SMILE_PATH;  // safe default
    
    // DEAD_ZONE = 0.2 → more sensitive  
    // DEAD_ZONE = 0.3 → more stable
    const float DEAD_ZONE = 0.2f;

    if (fabsf(x) < DEAD_ZONE && fabsf(y) < DEAD_ZONE) {
      desired = GIF_SLEEP_PATH;   // one extreme
    }
    else if (x > 0.5f) {
      desired = GIF_SMILE_PATH;   // upright / neutral
    }
    else if (x < -0.5f) {
      desired = GIF_SAD_PATH;     // opposite extreme
    }
    else if (fabsf(y) > 0.5f) {
      desired = GIF_JOY_PATH;     // side tilt
    }

    if (desired && (!emotion_current_gif || strcmp(desired, emotion_current_gif) != 0)) {
      emotion_current_gif = desired;
      // Swap GIF in-place using the widget stored as overlay user_data
      lv_obj_t *gif = (lv_obj_t *)lv_obj_get_user_data(overlay_cont);
      if (gif) {
        lv_gif_set_src(gif, desired);
        Serial.printf("[EMOTION] GIF → %s\n", desired);
      }
    }
  } else {
    // ── Brightness mode (status screen) ───────────────────────────────
    float y = accelData.accelY;
    if      (y >  0.5f) set_brightness(brightnessPercent - 10);
    else if (y < -0.5f) set_brightness(brightnessPercent + 10);
  }
}

// ── WiFi / NTP / Date status (lower-left) ────────────────────────────────────
static void show_status_screen(void)
{
  if (overlay_cont) return;
  overlay_cont = make_overlay(lv_color_make(10, 14, 26));

  // ── Title: date replaces plain "Status" label ─────────────────────────────
  // Screen is 172px tall (landscape). Title sits at top, separator at y=30,
  // leaving three evenly-spaced rows below for WiFi, NTP and Brightness.
  lv_obj_t *title = lv_label_create(overlay_cont);
  {
    time_t    now = time(nullptr);
    struct tm t;
    localtime_r(&now, &t);
    if (now > 1735689600UL) {  // RTC sane (> 2026) — show date
      static const char *wday[]  = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
      static const char *month[] = {"Jan","Feb","Mar","Apr","May","Jun",
                                     "Jul","Aug","Sep","Oct","Nov","Dec"};
      char date_buf[28];
      snprintf(date_buf, sizeof(date_buf), "%s %d %s %d",
               wday[t.tm_wday], t.tm_mday, month[t.tm_mon], 1900 + t.tm_year);
      lv_label_set_text(title, date_buf);
    } else {
      lv_label_set_text(title, "Status");  // RTC not set yet
    }
  }
  lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(title, lv_color_make(180, 180, 220), 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 8);
  lv_obj_add_flag(title, LV_OBJ_FLAG_IGNORE_LAYOUT);

  // ── Separator ─────────────────────────────────────────────────────────────
  lv_obj_t *sep = lv_obj_create(overlay_cont);
  lv_obj_set_size(sep, LV_PCT(85), 1);
  lv_obj_set_style_bg_color(sep, lv_color_make(50, 60, 100), 0);
  lv_obj_set_style_bg_opa(sep, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(sep, 0, 0);
  lv_obj_set_style_radius(sep, 0, 0);
  lv_obj_align(sep, LV_ALIGN_TOP_MID, 0, 30);
  lv_obj_add_flag(sep, LV_OBJ_FLAG_IGNORE_LAYOUT);

  // ── Row 1: WiFi  (y = -28 from mid = ~58px from top) ─────────────────────
  lv_obj_t *wifi_icon = lv_label_create(overlay_cont);
  lv_label_set_text(wifi_icon, LV_SYMBOL_WIFI);
  lv_obj_set_style_text_color(wifi_icon,
    wifiConnected ? lv_color_make(80, 200, 120) : lv_color_make(200, 80, 80), 0);
  lv_obj_align(wifi_icon, LV_ALIGN_LEFT_MID, 20, -28);
  lv_obj_add_flag(wifi_icon, LV_OBJ_FLAG_IGNORE_LAYOUT);

  lv_obj_t *wifi_val = lv_label_create(overlay_cont);
  if (wifiConnected) {
    char ssid_buf[48];
    snprintf(ssid_buf, sizeof(ssid_buf), "WiFi: %s", WiFi.SSID().c_str());
    lv_label_set_text(wifi_val, ssid_buf);
    lv_obj_set_style_text_color(wifi_val, lv_color_make(80, 200, 120), 0);
  } else {
    lv_label_set_text(wifi_val, "WiFi: disconnected");
    lv_obj_set_style_text_color(wifi_val, lv_color_make(200, 80, 80), 0);
  }
  lv_obj_align(wifi_val, LV_ALIGN_LEFT_MID, 44, -28);
  lv_obj_add_flag(wifi_val, LV_OBJ_FLAG_IGNORE_LAYOUT);

  // ── Row 2: NTP  (y = 0 from mid = 86px from top) ─────────────────────────
  lv_obj_t *ntp_icon = lv_label_create(overlay_cont);
  lv_label_set_text(ntp_icon, LV_SYMBOL_REFRESH);
  lv_obj_set_style_text_color(ntp_icon,
    timeSynced ? lv_color_make(80, 200, 120) : lv_color_make(200, 160, 50), 0);
  lv_obj_align(ntp_icon, LV_ALIGN_LEFT_MID, 20, 0);
  lv_obj_add_flag(ntp_icon, LV_OBJ_FLAG_IGNORE_LAYOUT);

  lv_obj_t *ntp_val = lv_label_create(overlay_cont);
  lv_label_set_text(ntp_val, timeSynced ? "NTP: synced" : "NTP: not synced");
  lv_obj_set_style_text_color(ntp_val,
    timeSynced ? lv_color_make(80, 200, 120) : lv_color_make(200, 160, 50), 0);
  lv_obj_align(ntp_val, LV_ALIGN_LEFT_MID, 44, 0);
  lv_obj_add_flag(ntp_val, LV_OBJ_FLAG_IGNORE_LAYOUT);

  // ── Row 3: Brightness  (y = +28 from mid = ~114px from top) ──────────────
  label_brightness = lv_label_create(overlay_cont);
  lv_label_set_text_fmt(label_brightness,
    LV_SYMBOL_IMAGE "  Brightness: %d%%  (tilt to adjust)",
    brightnessPercent);
  lv_obj_set_style_text_color(label_brightness, lv_color_make(200, 200, 100), 0);
  lv_obj_align(label_brightness, LV_ALIGN_LEFT_MID, 20, 28);
  lv_obj_add_flag(label_brightness, LV_OBJ_FLAG_IGNORE_LAYOUT);

  // Start tilt poll timer — 400 ms, runs while this screen is open
  tilt_timer = lv_timer_create(tilt_poll_cb, 400, nullptr);

  add_back_hint(overlay_cont);
}

// ── Battery (lower-right) ─────────────────────────────────────────────────────
static void show_battery_screen(void)
{
  if (overlay_cont) return;
  overlay_cont = make_overlay(lv_color_make(14, 14, 26));

  // Read immediately for the initial title
  uint16_t mv_now  = analogReadMilliVolts(BAT_PIN);
  float    v_now   = mv_now * 3.0f / 1000.0f;
  int      pct_now = battery_voltage_to_percent(v_now);

  // ── Title: icon + "Battery" + live % (updated by battery_timer_callback) ──
  label_percent = lv_label_create(overlay_cont);
  lv_label_set_text_fmt(label_percent, "%s  Battery  %d%%",
                        battery_icon(pct_now), pct_now);
  lv_obj_set_style_text_font(label_percent, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(label_percent, lv_color_make(180, 180, 220), 0);
  lv_obj_align(label_percent, LV_ALIGN_TOP_MID, 0, 8);
  lv_obj_add_flag(label_percent, LV_OBJ_FLAG_IGNORE_LAYOUT);

  lv_obj_t *sep = lv_obj_create(overlay_cont);
  lv_obj_set_size(sep, LV_PCT(85), 1);
  lv_obj_set_style_bg_color(sep, lv_color_make(50, 60, 100), 0);
  lv_obj_set_style_bg_opa(sep, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(sep, 0, 0);
  lv_obj_set_style_radius(sep, 0, 0);
  lv_obj_align(sep, LV_ALIGN_TOP_MID, 0, 30);
  lv_obj_add_flag(sep, LV_OBJ_FLAG_IGNORE_LAYOUT);

  // ── Row 1: Voltage ────────────────────────────────────────────────────────
  lv_obj_t *v_key = lv_label_create(overlay_cont);
  lv_label_set_text(v_key, "Voltage");
  lv_obj_set_style_text_color(v_key, lv_color_make(140, 140, 180), 0);
  lv_obj_align(v_key, LV_ALIGN_LEFT_MID, 24, -22);
  lv_obj_add_flag(v_key, LV_OBJ_FLAG_IGNORE_LAYOUT);

  label_voltage = lv_label_create(overlay_cont);
  lv_label_set_text(label_voltage, "--- V");
  lv_obj_set_style_text_font(label_voltage, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(label_voltage, lv_color_make(100, 220, 120), 0);
  lv_obj_align(label_voltage, LV_ALIGN_RIGHT_MID, -24, -22);
  lv_obj_add_flag(label_voltage, LV_OBJ_FLAG_IGNORE_LAYOUT);

  // ── Row 2: ADC raw ────────────────────────────────────────────────────────
  lv_obj_t *adc_key = lv_label_create(overlay_cont);
  lv_label_set_text(adc_key, "ADC raw");
  lv_obj_set_style_text_color(adc_key, lv_color_make(140, 140, 180), 0);
  lv_obj_align(adc_key, LV_ALIGN_LEFT_MID, 24, 4);
  lv_obj_add_flag(adc_key, LV_OBJ_FLAG_IGNORE_LAYOUT);

  label_adc_raw = lv_label_create(overlay_cont);
  lv_label_set_text(label_adc_raw, "---");
  lv_obj_set_style_text_font(label_adc_raw, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(label_adc_raw, lv_color_white(), 0);
  lv_obj_align(label_adc_raw, LV_ALIGN_RIGHT_MID, -24, 4);
  lv_obj_add_flag(label_adc_raw, LV_OBJ_FLAG_IGNORE_LAYOUT);

  // ── Row 3: Power-off hint ─────────────────────────────────────────────────
  lv_obj_t *pwr_hint = lv_label_create(overlay_cont);
  lv_label_set_text(pwr_hint, LV_SYMBOL_POWER "  hold to power off");
  lv_obj_set_style_text_color(pwr_hint, lv_color_make(160, 100, 100), 0);
  lv_obj_set_style_text_opa(pwr_hint, LV_OPA_70, 0);
  lv_obj_align(pwr_hint, LV_ALIGN_LEFT_MID, 24, 30);
  lv_obj_add_flag(pwr_hint, LV_OBJ_FLAG_IGNORE_LAYOUT);

  // Long-press triggers the 5-second shutdown countdown popup
  lv_obj_add_event_cb(overlay_cont, battery_longpress_cb,
                      LV_EVENT_LONG_PRESSED, nullptr);

  battery_timer_callback(nullptr);
  add_back_hint(overlay_cont);
}

// ══════════════════════════════════════════════════════════════════════════════
//  ALARM SCREEN
//  Layout (landscape 320×172):
//
//    ┌─────────────────────────────────────────┐
//    │           Set Alarm                     │  y=8  title
//    │  ─────────────────────────────────────  │  y=28 separator
//    │     ▲            ▲                      │
//    │   [ HH ]  :  [ mm ]    [ ON / OFF ]     │  tap ▲/▼ to step, tap ON/OFF
//    │     ▼            ▼                      │
//    │                                         │
//    │       hold anywhere to save & exit      │  hint
//    └─────────────────────────────────────────┘
//
//  Each of HH, mm and ON/OFF has two invisible tap zones (top/bottom half).
//  Long-press anywhere saves and closes.
// ══════════════════════════════════════════════════════════════════════════════

// ══════════════════════════════════════════════════════════════════════════════
//  CAROUSEL + EDITORS
//  Long-press → carousel: ◀ [ CLOCK | TIMER | ALARM | WiFi ] ▶
//  Tap centre → open editor (HH:MM + toggle). Long-press → save + exit to clock.
//  WiFi: tap centre to toggle inline — no sub-screen.
// ══════════════════════════════════════════════════════════════════════════════

// ── Carousel / editor state ───────────────────────────────────────────────────
static lv_obj_t *editor_cont   = nullptr;
static lv_obj_t *se_hour_lbl   = nullptr;
static lv_obj_t *se_min_lbl    = nullptr;
static lv_obj_t *se_onoff_lbl  = nullptr;
static int  edit_hour           = 0;
static int  edit_min            = 0;
static bool edit_enabled        = false;
static int  edit_day            = 1;
static int  edit_month          = 1;   // 1-12
static int  edit_year           = 2026;
static lv_obj_t *se_day_lbl     = nullptr;
static lv_obj_t *se_mon_lbl     = nullptr;
static lv_obj_t *se_yr_lbl      = nullptr;

// ── Flash animation ───────────────────────────────────────────────────────────
static void se_flash_cb(void *obj, int32_t v)
{ lv_obj_set_style_text_opa((lv_obj_t*)obj,(lv_opa_t)v,0); }

static void se_flash(lv_obj_t *lbl)
{
  if (!lbl) return;
  lv_anim_t a; lv_anim_init(&a);
  lv_anim_set_var(&a,lbl);
  lv_anim_set_exec_cb(&a,se_flash_cb);
  lv_anim_set_values(&a,LV_OPA_30,LV_OPA_COVER);
  lv_anim_set_duration(&a,120);
  lv_anim_start(&a);
}

static void se_refresh()
{
  if (!se_hour_lbl) return;
  lv_label_set_text_fmt(se_hour_lbl,"%02d",edit_hour);
  lv_label_set_text_fmt(se_min_lbl, "%02d",edit_min);
  if (se_onoff_lbl) {
    if (carousel_idx == 1)  // Timer: Ready!/Not yet
      lv_label_set_text(se_onoff_lbl, edit_enabled?"#00e070 Ready!#":"#808080 Not yet#");
    else                    // Alarm: ON/OFF
      lv_label_set_text(se_onoff_lbl, edit_enabled?"#00e070 ON#":"#808080 OFF#");
  }
  se_flash(se_hour_lbl); se_flash(se_min_lbl); se_flash(se_onoff_lbl);
  // Date labels (only populated by open_clock_editor)
  static const char *mon_names[] = {"Jan","Feb","Mar","Apr","May","Jun",
                                     "Jul","Aug","Sep","Oct","Nov","Dec"};
  if (se_day_lbl) lv_label_set_text_fmt(se_day_lbl, "%02d", edit_day);
  if (se_mon_lbl) lv_label_set_text(se_mon_lbl, mon_names[edit_month-1]);
  if (se_yr_lbl)  lv_label_set_text_fmt(se_yr_lbl,  "%d",   edit_year);
  se_flash(se_day_lbl); se_flash(se_mon_lbl); se_flash(se_yr_lbl);
}

// ── Forward refs ──────────────────────────────────────────────────────────────
static void modal_longpress_cb(lv_event_t *e);
static void carousel_build(void);

// ── Invisible tap-zone helper ─────────────────────────────────────────────────
static lv_obj_t *se_zone(lv_obj_t *p,int x,int y,int w,int h,lv_event_cb_t cb)
{
  lv_obj_t *z=lv_obj_create(p);
  lv_obj_set_size(z,w,h); lv_obj_set_pos(z,x,y);
  lv_obj_set_style_bg_opa(z,LV_OPA_TRANSP,0);
  lv_obj_set_style_border_width(z,0,0); lv_obj_set_style_pad_all(z,0,0);
  lv_obj_set_style_radius(z,0,0); lv_obj_set_style_shadow_width(z,0,0);
  lv_obj_clear_flag(z,LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_event_cb(z,cb,LV_EVENT_PRESSED,nullptr);
  lv_obj_add_event_cb(z,modal_longpress_cb,LV_EVENT_LONG_PRESSED,nullptr);
  return z;
}

// ── Value edit callbacks ───────────────────────────────────────────────────────
static void se_h_up(lv_event_t*e){if(lv_event_get_code(e)==LV_EVENT_PRESSED){edit_hour=(edit_hour+1)%24;se_refresh();}}
static void se_h_dn(lv_event_t*e){if(lv_event_get_code(e)==LV_EVENT_PRESSED){edit_hour=(edit_hour+23)%24;se_refresh();}}
static void se_m_up(lv_event_t*e){if(lv_event_get_code(e)==LV_EVENT_PRESSED){edit_min=(edit_min+1)%60;se_refresh();}}
static void se_m_dn(lv_event_t*e){if(lv_event_get_code(e)==LV_EVENT_PRESSED){edit_min=(edit_min+59)%60;se_refresh();}}
static void se_tog(lv_event_t*e) {if(lv_event_get_code(e)==LV_EVENT_PRESSED){edit_enabled=!edit_enabled;se_refresh();}}

// Days in month (leap-year aware)
static int days_in_month(int m, int y)
{
  static const int dim[] = {31,28,31,30,31,30,31,31,30,31,30,31};
  if (m==2 && ((y%4==0&&y%100!=0)||(y%400==0))) return 29;
  return dim[m-1];
}
static void se_day_up(lv_event_t*e){if(lv_event_get_code(e)==LV_EVENT_PRESSED){int d=days_in_month(edit_month,edit_year);edit_day=edit_day%d+1;se_refresh();}}
static void se_day_dn(lv_event_t*e){if(lv_event_get_code(e)==LV_EVENT_PRESSED){int d=days_in_month(edit_month,edit_year);edit_day=(edit_day-2+d)%d+1;se_refresh();}}
static void se_mon_up(lv_event_t*e){if(lv_event_get_code(e)==LV_EVENT_PRESSED){edit_month=edit_month%12+1;int d=days_in_month(edit_month,edit_year);if(edit_day>d)edit_day=d;se_refresh();}}
static void se_mon_dn(lv_event_t*e){if(lv_event_get_code(e)==LV_EVENT_PRESSED){edit_month=(edit_month-2+12)%12+1;int d=days_in_month(edit_month,edit_year);if(edit_day>d)edit_day=d;se_refresh();}}
static void se_yr_up(lv_event_t*e){if(lv_event_get_code(e)==LV_EVENT_PRESSED){edit_year++;se_refresh();}}
static void se_yr_dn(lv_event_t*e){if(lv_event_get_code(e)==LV_EVENT_PRESSED){if(edit_year>2026)edit_year--;se_refresh();}}

// ── Clock editor: HH:MM on top row + DD/MON/YYYY on second row ───────────────
static void open_clock_editor()
{
  if (editor_cont) { lv_obj_del(editor_cont); editor_cont=nullptr; }
  se_hour_lbl=se_min_lbl=se_onoff_lbl=nullptr;
  se_day_lbl=se_mon_lbl=se_yr_lbl=nullptr;

  // Pre-load current RTC
  time_t n=time(nullptr); struct tm t; localtime_r(&n,&t);
  edit_hour  = t.tm_hour;
  edit_min   = t.tm_min;
  edit_day   = t.tm_mday;
  edit_month = t.tm_mon + 1;
  edit_year  = 1900 + t.tm_year;

  editor_cont=lv_obj_create(modal_cont);
  lv_obj_set_size(editor_cont,320,172); lv_obj_set_pos(editor_cont,0,0);
  lv_obj_set_style_bg_color(editor_cont,lv_color_make(8,12,28),0);
  lv_obj_set_style_bg_opa(editor_cont,LV_OPA_COVER,0);
  lv_obj_set_style_border_width(editor_cont,0,0);
  lv_obj_set_style_pad_all(editor_cont,0,0);
  lv_obj_set_style_radius(editor_cont,0,0);
  lv_obj_clear_flag(editor_cont,LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_event_cb(editor_cont,modal_longpress_cb,LV_EVENT_LONG_PRESSED,nullptr);

  // ── Layout constants ────────────────────────────────────────────────────────
  // Time row  (montserrat_48): centred at y=24, height=48
  const int TY=24, TH=48, TA=16;  // top-row y, height, arrow height
  // Date row  (montserrat_16): centred at y=96, height=20
  const int DY=112, DH=20, DA=12;  // date-row y, height, arrow height
  // Time columns
  const int HX=60,HW=60, MX=168,MW=60;
  // Date columns
  const int DDX=50,DDW=36, MOX=110,MOW=44, YX=180,YW=58;

  auto mkcont=[&](int x,int w,int y,int h)->lv_obj_t*{
    lv_obj_t*cont=lv_obj_create(editor_cont);
    lv_obj_set_size(cont,w,h); lv_obj_set_pos(cont,x,y);
    lv_obj_set_style_bg_opa(cont,LV_OPA_TRANSP,0);
    lv_obj_set_style_border_width(cont,0,0); lv_obj_set_style_pad_all(cont,0,0);
    lv_obj_set_style_radius(cont,0,0); lv_obj_clear_flag(cont,LV_OBJ_FLAG_SCROLLABLE);
    return cont;
  };
  auto mkarr=[&](int x,int w,int y,const char*s,bool small){
    lv_obj_t*a=lv_label_create(editor_cont); lv_label_set_text(a,s);
    lv_obj_set_style_text_color(a,lv_color_make(100,120,200),0);
    if (small) lv_obj_set_style_text_font(a,&lv_font_montserrat_14,0);
    lv_obj_set_pos(a,x,y); lv_obj_set_width(a,w);
    lv_obj_set_style_text_align(a,LV_TEXT_ALIGN_CENTER,0);
  };

  // ── Time row ─────────────────────────────────────────────────────────────
  lv_obj_t*hc=mkcont(HX,HW,TY,TH);
  se_hour_lbl=lv_label_create(hc);
  lv_obj_set_style_text_font(se_hour_lbl,&lv_font_montserrat_48,0);
  lv_obj_set_style_text_color(se_hour_lbl,lv_color_white(),0);
  lv_obj_set_size(se_hour_lbl,LV_SIZE_CONTENT,LV_SIZE_CONTENT);
  lv_obj_align(se_hour_lbl,LV_ALIGN_CENTER,0,0);

  lv_obj_t*col=lv_label_create(editor_cont); lv_label_set_text(col,":");
  lv_obj_set_style_text_font(col,&lv_font_montserrat_48,0);
  lv_obj_set_style_text_color(col,lv_color_make(140,140,180),0);
  lv_obj_set_pos(col,142,TY);

  lv_obj_t*mc=mkcont(MX,MW,TY,TH);
  se_min_lbl=lv_label_create(mc);
  lv_obj_set_style_text_font(se_min_lbl,&lv_font_montserrat_48,0);
  lv_obj_set_style_text_color(se_min_lbl,lv_color_white(),0);
  lv_obj_set_size(se_min_lbl,LV_SIZE_CONTENT,LV_SIZE_CONTENT);
  lv_obj_align(se_min_lbl,LV_ALIGN_CENTER,0,0);

  mkarr(HX,HW,TY-TA,LV_SYMBOL_UP,false);
  mkarr(HX,HW,TY+TH,LV_SYMBOL_DOWN,false);
  mkarr(MX,MW,TY-TA,LV_SYMBOL_UP,false);
  mkarr(MX,MW,TY+TH,LV_SYMBOL_DOWN,false);

  int t_mid = TY+TH/2;
  se_zone(editor_cont,HX,TA,HW,t_mid-TA,se_h_up);
  se_zone(editor_cont,HX,t_mid,HW,TY+TH-t_mid,se_h_dn);
  se_zone(editor_cont,MX,TA,MW,t_mid-TA,se_m_up);
  se_zone(editor_cont,MX,t_mid,MW,TY+TH-t_mid,se_m_dn);

  // ── Thin divider between time and date rows ───────────────────────────────
  lv_obj_t*div=lv_obj_create(editor_cont);
  lv_obj_set_size(div,240,1); lv_obj_set_pos(div,40,DY-16);
  lv_obj_set_style_bg_color(div,lv_color_make(50,60,100),0);
  lv_obj_set_style_bg_opa(div,LV_OPA_COVER,0);
  lv_obj_set_style_border_width(div,0,0); lv_obj_set_style_radius(div,0,0);

  // ── Date row ─────────────────────────────────────────────────────────────
  lv_obj_t*dc=mkcont(DDX,DDW,DY,DH);
  se_day_lbl=lv_label_create(dc);
  lv_obj_set_style_text_font(se_day_lbl,&lv_font_montserrat_16,0);
  lv_obj_set_style_text_color(se_day_lbl,lv_color_white(),0);
  lv_obj_set_size(se_day_lbl,LV_SIZE_CONTENT,LV_SIZE_CONTENT);
  lv_obj_align(se_day_lbl,LV_ALIGN_CENTER,0,0);

  lv_obj_t*s1=lv_label_create(editor_cont); lv_label_set_text(s1,"/");
  lv_obj_set_style_text_font(s1,&lv_font_montserrat_16,0);
  lv_obj_set_style_text_color(s1,lv_color_make(140,140,180),0);
  lv_obj_set_pos(s1,DDX+DDW,DY+1);

  lv_obj_t*mnc=mkcont(MOX,MOW,DY,DH);
  se_mon_lbl=lv_label_create(mnc);
  lv_obj_set_style_text_font(se_mon_lbl,&lv_font_montserrat_16,0);
  lv_obj_set_style_text_color(se_mon_lbl,lv_color_white(),0);
  lv_obj_set_size(se_mon_lbl,LV_SIZE_CONTENT,LV_SIZE_CONTENT);
  lv_obj_align(se_mon_lbl,LV_ALIGN_CENTER,0,0);

  lv_obj_t*s2=lv_label_create(editor_cont); lv_label_set_text(s2,"/");
  lv_obj_set_style_text_font(s2,&lv_font_montserrat_16,0);
  lv_obj_set_style_text_color(s2,lv_color_make(140,140,180),0);
  lv_obj_set_pos(s2,MOX+MOW,DY+1);

  lv_obj_t*yc=mkcont(YX,YW,DY,DH);
  se_yr_lbl=lv_label_create(yc);
  lv_obj_set_style_text_font(se_yr_lbl,&lv_font_montserrat_16,0);
  lv_obj_set_style_text_color(se_yr_lbl,lv_color_white(),0);
  lv_obj_set_size(se_yr_lbl,LV_SIZE_CONTENT,LV_SIZE_CONTENT);
  lv_obj_align(se_yr_lbl,LV_ALIGN_CENTER,0,0);

  int d_mid=DY+DH/2;
  mkarr(DDX,DDW,DY-DA,LV_SYMBOL_UP,true);
  mkarr(DDX,DDW,DY+DH+2,LV_SYMBOL_DOWN,true);
  mkarr(MOX,MOW,DY-DA,LV_SYMBOL_UP,true);
  mkarr(MOX,MOW,DY+DH+2,LV_SYMBOL_DOWN,true);
  mkarr(YX, YW, DY-DA,LV_SYMBOL_UP,true);
  mkarr(YX, YW, DY+DH+2,LV_SYMBOL_DOWN,true);

  se_zone(editor_cont,DDX,DY-DA,DDW,DH/2+DA,se_day_up);
  se_zone(editor_cont,DDX,d_mid,DDW,DH/2+DA+4,se_day_dn);
  se_zone(editor_cont,MOX,DY-DA,MOW,DH/2+DA,se_mon_up);
  se_zone(editor_cont,MOX,d_mid,MOW,DH/2+DA+4,se_mon_dn);
  se_zone(editor_cont,YX, DY-DA,YW, DH/2+DA,se_yr_up);
  se_zone(editor_cont,YX, d_mid,YW, DH/2+DA+4,se_yr_dn);

  lv_obj_t*hint=lv_label_create(editor_cont);
  lv_label_set_text(hint,"hold to save & exit");
  lv_obj_set_style_text_color(hint,lv_color_make(100,100,120),0);
  lv_obj_set_style_text_opa(hint,LV_OPA_60,0);
  lv_obj_align(hint,LV_ALIGN_BOTTOM_MID,0,-4);

  se_refresh();
}

// ── Open shared HH:MM (+ optional toggle) editor ─────────────────────────────
static void open_editor(int h,int m,bool enabled,bool show_toggle)
{
  if (editor_cont) { lv_obj_del(editor_cont); editor_cont=nullptr; }
  se_hour_lbl=se_min_lbl=se_onoff_lbl=nullptr;
  edit_hour=h; edit_min=m; edit_enabled=enabled;

  editor_cont=lv_obj_create(modal_cont);
  lv_obj_set_size(editor_cont,320,172); lv_obj_set_pos(editor_cont,0,0);
  lv_obj_set_style_bg_color(editor_cont,lv_color_make(8,12,28),0);
  lv_obj_set_style_bg_opa(editor_cont,LV_OPA_COVER,0);
  lv_obj_set_style_border_width(editor_cont,0,0);
  lv_obj_set_style_pad_all(editor_cont,0,0);
  lv_obj_set_style_radius(editor_cont,0,0);
  lv_obj_clear_flag(editor_cont,LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_event_cb(editor_cont,modal_longpress_cb,LV_EVENT_LONG_PRESSED,nullptr);

  const int VY=54,VH=52,AH=20;
  const int HX=42,HW=60,MX=150,MW=60,TX=240,TW=60;

  auto mkcont=[&](int x,int w,int y,int h2)->lv_obj_t*{
    lv_obj_t*cont=lv_obj_create(editor_cont);
    lv_obj_set_size(cont,w,h2); lv_obj_set_pos(cont,x,y);
    lv_obj_set_style_bg_opa(cont,LV_OPA_TRANSP,0);
    lv_obj_set_style_border_width(cont,0,0); lv_obj_set_style_pad_all(cont,0,0);
    lv_obj_set_style_radius(cont,0,0); lv_obj_clear_flag(cont,LV_OBJ_FLAG_SCROLLABLE);
    return cont;
  };
  auto mkarr=[&](int x,int w,int y,const char*s){
    lv_obj_t*a=lv_label_create(editor_cont); lv_label_set_text(a,s);
    lv_obj_set_style_text_color(a,lv_color_make(100,120,200),0);
    lv_obj_set_pos(a,x,y); lv_obj_set_width(a,w);
    lv_obj_set_style_text_align(a,LV_TEXT_ALIGN_CENTER,0);
  };

  // HH
  lv_obj_t*hc=mkcont(HX,HW,VY,VH);
  se_hour_lbl=lv_label_create(hc);
  lv_obj_set_style_text_font(se_hour_lbl,&lv_font_montserrat_48,0);
  lv_obj_set_style_text_color(se_hour_lbl,lv_color_white(),0);
  lv_obj_set_size(se_hour_lbl,LV_SIZE_CONTENT,LV_SIZE_CONTENT);
  lv_obj_align(se_hour_lbl,LV_ALIGN_CENTER,0,0);

  // Colon
  lv_obj_t*col=lv_label_create(editor_cont); lv_label_set_text(col,":");
  lv_obj_set_style_text_font(col,&lv_font_montserrat_48,0);
  lv_obj_set_style_text_color(col,lv_color_make(140,140,180),0);
  lv_obj_set_pos(col,124,VY);

  // MM
  lv_obj_t*mc=mkcont(MX,MW,VY,VH);
  se_min_lbl=lv_label_create(mc);
  lv_obj_set_style_text_font(se_min_lbl,&lv_font_montserrat_48,0);
  lv_obj_set_style_text_color(se_min_lbl,lv_color_white(),0);
  lv_obj_set_size(se_min_lbl,LV_SIZE_CONTENT,LV_SIZE_CONTENT);
  lv_obj_align(se_min_lbl,LV_ALIGN_CENTER,0,0);

  if (show_toggle) {
    lv_obj_t*tc=mkcont(TX,TW,VY+12,VH-24);
    se_onoff_lbl=lv_label_create(tc);
    lv_obj_set_style_text_font(se_onoff_lbl,&lv_font_montserrat_16,0);
    lv_label_set_recolor(se_onoff_lbl,true);
    lv_obj_set_size(se_onoff_lbl,LV_SIZE_CONTENT,LV_SIZE_CONTENT);
    lv_obj_align(se_onoff_lbl,LV_ALIGN_CENTER,0,0);
    mkarr(TX,TW,VY-AH,LV_SYMBOL_UP);
    mkarr(TX,TW,VY+VH,LV_SYMBOL_DOWN);
    se_zone(editor_cont,TX,28,TW,(VY+VH/2)-28,se_tog);
    se_zone(editor_cont,TX,VY+VH/2,TW,172-(VY+VH/2)-18,se_tog);
  }

  mkarr(HX,HW,VY-AH,LV_SYMBOL_UP); mkarr(HX,HW,VY+VH,LV_SYMBOL_DOWN);
  mkarr(MX,MW,VY-AH,LV_SYMBOL_UP); mkarr(MX,MW,VY+VH,LV_SYMBOL_DOWN);
  se_zone(editor_cont,HX,28,HW,(VY+VH/2)-28,se_h_up);
  se_zone(editor_cont,HX,VY+VH/2,HW,172-(VY+VH/2)-18,se_h_dn);
  se_zone(editor_cont,MX,28,MW,(VY+VH/2)-28,se_m_up);
  se_zone(editor_cont,MX,VY+VH/2,MW,172-(VY+VH/2)-18,se_m_dn);

  lv_obj_t*hint=lv_label_create(editor_cont);
  lv_label_set_text(hint,"hold to save & exit");
  lv_obj_set_style_text_color(hint,lv_color_make(100,100,120),0);
  lv_obj_set_style_text_opa(hint,LV_OPA_60,0);
  lv_obj_align(hint,LV_ALIGN_BOTTOM_MID,0,-4);
  se_refresh();
}

// ── Save helpers ──────────────────────────────────────────────────────────────
static void close_clock_editor()
{
  struct tm cur = {};
  cur.tm_hour  = edit_hour;
  cur.tm_min   = edit_min;
  cur.tm_sec   = 0;
  cur.tm_mday  = edit_day;
  cur.tm_mon   = edit_month - 1;
  cur.tm_year  = edit_year - 1900;
  cur.tm_isdst = -1;
  struct timeval tv={mktime(&cur),0}; settimeofday(&tv,nullptr);
  if (home_time_lbl) {
    char buf[6]; snprintf(buf,sizeof(buf),"%02d:%02d",edit_hour,edit_min);
    lv_label_set_text(home_time_lbl,buf);
  }
  Serial.printf("[SETTINGS] RTC set to %04d-%02d-%02d %02d:%02d\n",
                edit_year,edit_month,edit_day,edit_hour,edit_min);
  log_last_seen();
}
static void close_timer_editor()
{
  cfg.timer_hours=edit_hour; cfg.timer_minutes=edit_min;
  save_config();
  if (edit_enabled) {
    timer_start_countdown();  // Ready! — start the countdown
  } else {
    timer_stop();             // Not yet — cancel any running timer
  }
}
static void close_alarm_editor()
{
  cfg.alarm_enabled=edit_enabled;
  cfg.alarm_hour=edit_hour; cfg.alarm_minute=edit_min;
  save_config();
  if (home_bell_lbl) {
    if (cfg.alarm_enabled) {
      lv_label_set_text_fmt(home_bell_lbl,LV_SYMBOL_BELL " %02d:%02d",
                            cfg.alarm_hour,cfg.alarm_minute);
      lv_obj_clear_flag(home_bell_lbl,LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_add_flag(home_bell_lbl,LV_OBJ_FLAG_HIDDEN);
    }
  }
}

// ── Modal close ───────────────────────────────────────────────────────────────
static void modal_close()
{
  se_hour_lbl=se_min_lbl=se_onoff_lbl=nullptr;
  se_day_lbl=se_mon_lbl=se_yr_lbl=nullptr;
  editor_cont=nullptr;
  if (modal_cont) { lv_obj_del(modal_cont); modal_cont=nullptr; alarm_cont=nullptr; }
}

// ── Long-press: save and exit all the way to clock ───────────────────────────
static void modal_longpress_cb(lv_event_t *e)
{
  if (lv_event_get_code(e)!=LV_EVENT_LONG_PRESSED) return;
  lv_indev_wait_release(lv_indev_get_act());
  if (!editor_cont) { modal_close(); return; }
  switch (carousel_idx) {
    case 0: close_clock_editor(); break;
    case 1: close_timer_editor(); break;
    case 2: close_alarm_editor(); break;
    default: break;
  }
  modal_close();
}

// ── Carousel tap: enter the selected item ────────────────────────────────────
static void carousel_tap_cb(lv_event_t *e)
{
  if (lv_event_get_code(e)!=LV_EVENT_CLICKED) return;
  switch (carousel_idx) {
    case 0: open_clock_editor(); break;
    case 1: // Timer — always open with Not yet
      open_editor(cfg.timer_hours,cfg.timer_minutes,false,true); break;
    case 2: // Alarm
      open_editor(cfg.alarm_hour,cfg.alarm_minute,cfg.alarm_enabled,true); break;
    case 3: // WiFi — inline toggle
      cfg.wifi_enabled=!cfg.wifi_enabled;
      apply_wifi_state(); save_config();
      carousel_build(); break;
  }
}

// ── Left/right navigation ────────────────────────────────────────────────────
static void carousel_left_cb(lv_event_t*e)
{ if(lv_event_get_code(e)==LV_EVENT_PRESSED){carousel_idx=(carousel_idx+3)%4;carousel_build();} }
static void carousel_right_cb(lv_event_t*e)
{ if(lv_event_get_code(e)==LV_EVENT_PRESSED){carousel_idx=(carousel_idx+1)%4;carousel_build();} }

// ── Rebuild carousel view ─────────────────────────────────────────────────────
static void carousel_build()
{
  if (editor_cont) { lv_obj_del(editor_cont); editor_cont=nullptr; }
  se_hour_lbl=se_min_lbl=se_onoff_lbl=nullptr;
  lv_obj_clean(modal_cont);

  struct CarouselItem { const char *icon; const char *name; const char *desc; };
  static const CarouselItem items[4]={
    {LV_SYMBOL_HOME, "CLOCK",  "Set current time"},
    {LV_SYMBOL_STOP, "TIMER",  "Set countdown"},
    {LV_SYMBOL_BELL, "ALARM",  "Set wake-up alarm"},
    {LV_SYMBOL_WIFI, "WiFi",   nullptr},
  };
  char wifi_desc[20]={};
  if (carousel_idx==3)
    snprintf(wifi_desc,sizeof(wifi_desc),"Currently: %s",cfg.wifi_enabled?"ON":"OFF");
  const char *desc=(carousel_idx==3)?wifi_desc:items[carousel_idx].desc;

  lv_obj_add_event_cb(modal_cont,modal_longpress_cb,LV_EVENT_LONG_PRESSED,nullptr);

  // Left arrow + zone
  lv_obj_t*larr=lv_label_create(modal_cont);
  lv_label_set_text(larr,LV_SYMBOL_LEFT);
  lv_obj_set_style_text_font(larr,&lv_font_montserrat_48,0);
  lv_obj_set_style_text_color(larr,lv_color_make(80,100,180),0);
  lv_obj_align(larr,LV_ALIGN_LEFT_MID,6,0);
  se_zone(modal_cont,0,0,60,172,carousel_left_cb);

  // Right arrow + zone
  lv_obj_t*rarr=lv_label_create(modal_cont);
  lv_label_set_text(rarr,LV_SYMBOL_RIGHT);
  lv_obj_set_style_text_font(rarr,&lv_font_montserrat_48,0);
  lv_obj_set_style_text_color(rarr,lv_color_make(80,100,180),0);
  lv_obj_align(rarr,LV_ALIGN_RIGHT_MID,-6,0);
  se_zone(modal_cont,260,0,60,172,carousel_right_cb);

  // Icon
  lv_obj_t*icon=lv_label_create(modal_cont);
  lv_label_set_text(icon,items[carousel_idx].icon);
  lv_obj_set_style_text_font(icon,&lv_font_montserrat_48,0);
  lv_obj_set_style_text_color(icon,
    carousel_idx==3&&!cfg.wifi_enabled?lv_color_make(180,60,60):lv_color_make(120,200,255),0);
  lv_obj_align(icon,LV_ALIGN_CENTER,0,-28);

  // Name
  lv_obj_t*name_lbl=lv_label_create(modal_cont);
  lv_label_set_text(name_lbl,items[carousel_idx].name);
  lv_obj_set_style_text_font(name_lbl,&lv_font_montserrat_16,0);
  lv_obj_set_style_text_color(name_lbl,lv_color_white(),0);
  lv_obj_align(name_lbl,LV_ALIGN_CENTER,0,18);

  // Description
  lv_obj_t*desc_lbl=lv_label_create(modal_cont);
  lv_label_set_text(desc_lbl,desc);
  lv_obj_set_style_text_font(desc_lbl,&lv_font_montserrat_14,0);
  lv_obj_set_style_text_color(desc_lbl,
    carousel_idx==3?(cfg.wifi_enabled?lv_color_make(80,200,120):lv_color_make(200,80,80))
                   :lv_color_make(160,160,180),0);
  lv_obj_align(desc_lbl,LV_ALIGN_CENTER,0,40);

  // Centre tap zone — uses CLICKED so long-press and tap are mutually
  // exclusive: CLICKED only fires when the finger lifts without triggering
  // LONG_PRESSED, so the editor never opens immediately before closing.
  {
    lv_obj_t *z = lv_obj_create(modal_cont);
    lv_obj_set_size(z,200,172); lv_obj_set_pos(z,60,0);
    lv_obj_set_style_bg_opa(z,LV_OPA_TRANSP,0);
    lv_obj_set_style_border_width(z,0,0); lv_obj_set_style_pad_all(z,0,0);
    lv_obj_set_style_radius(z,0,0); lv_obj_set_style_shadow_width(z,0,0);
    lv_obj_clear_flag(z,LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(z,carousel_tap_cb,LV_EVENT_CLICKED,nullptr);
    lv_obj_add_event_cb(z,modal_longpress_cb,LV_EVENT_LONG_PRESSED,nullptr);
  }

  // Hint — shown above the position dots
  lv_obj_t*hint=lv_label_create(modal_cont);
  lv_label_set_text(hint,"tap in or hold to exit");
  lv_obj_set_style_text_color(hint,lv_color_make(80,80,100),0);
  lv_obj_set_style_text_opa(hint,LV_OPA_60,0);
  lv_obj_set_style_text_font(hint,&lv_font_montserrat_14,0);
  lv_obj_align(hint,LV_ALIGN_BOTTOM_MID,0,-18);  // above the dots

  // Position dots — bottom row
  for (int i=0;i<4;i++) {
    lv_obj_t*dot=lv_label_create(modal_cont);
    lv_label_set_text(dot,i==carousel_idx?"●":"○");
    lv_obj_set_style_text_color(dot,
      i==carousel_idx?lv_color_white():lv_color_make(80,80,100),0);
    lv_obj_set_pos(dot,137+i*14,156);
  }
}

// ── Open the carousel (called from home long-press) ───────────────────────────
static void show_carousel(void)
{
  if (modal_cont || overlay_cont) return;
  carousel_idx=0;  // always start at CLOCK

  modal_cont=lv_obj_create(lv_scr_act());
  lv_obj_set_size(modal_cont,LV_PCT(100),LV_PCT(100));
  lv_obj_align(modal_cont,LV_ALIGN_CENTER,0,0);
  lv_obj_set_style_bg_color(modal_cont,lv_color_make(8,12,28),0);
  lv_obj_set_style_bg_opa(modal_cont,LV_OPA_COVER,0);
  lv_obj_set_style_border_width(modal_cont,0,0);
  lv_obj_set_style_pad_all(modal_cont,0,0);
  lv_obj_set_style_radius(modal_cont,0,0);
  lv_obj_clear_flag(modal_cont,LV_OBJ_FLAG_SCROLLABLE);
  alarm_cont=modal_cont;  // keep existing guards working
  carousel_build();
}

// Backward-compat alias (home_longpress still calls show_alarm_screen)
static inline void show_alarm_screen() { show_carousel(); }

// Helper: refresh the bell label on the home screen
static void update_home_bell()
{
  if (!home_bell_lbl) return;
  if (cfg.alarm_enabled) {
    lv_label_set_text_fmt(home_bell_lbl, LV_SYMBOL_BELL " %02d:%02d",
                          cfg.alarm_hour, cfg.alarm_minute);
    lv_obj_clear_flag(home_bell_lbl, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(home_bell_lbl, LV_OBJ_FLAG_HIDDEN);
  }
}

// ══════════════════════════════════════════════════════════════════════════════
//  HOME SCREEN  —  "Hello!" splash → big clock face + 4-zone invisible touch
// ══════════════════════════════════════════════════════════════════════════════

// One-shot callback: fired 2.5 s after boot to switch Hello → clock
static void clock_face_show(lv_timer_t *t)
{
  lv_timer_del(t);

  if (home_hello_lbl) {
    lv_obj_add_flag(home_hello_lbl, LV_OBJ_FLAG_HIDDEN);
    home_hello_lbl = nullptr;
  }

  // Large HH:mm label — populate from RTC immediately when waking from sleep
  home_time_lbl = lv_label_create(lv_scr_act());
  if (boot_from_sleep) {
    time_t rtc_now = time(nullptr);
    struct tm rtc_tm; localtime_r(&rtc_now, &rtc_tm);
    char tbuf[6];
    snprintf(tbuf, sizeof(tbuf), "%02d:%02d", rtc_tm.tm_hour, rtc_tm.tm_min);
    lv_label_set_text(home_time_lbl, tbuf);  // real time — no "--:--" flash
  } else {
    lv_label_set_text(home_time_lbl, "--:--");  // cold boot: wait for NTP
  }
  lv_obj_set_style_text_font(home_time_lbl, &montserrat_96, 0);
  lv_obj_set_style_text_color(home_time_lbl, lv_color_white(), 0);
  lv_obj_set_style_text_letter_space(home_time_lbl, 4, 0);
  lv_obj_align(home_time_lbl, LV_ALIGN_CENTER, 0, 0);

  // ── Timer label (bottom-left, hidden when timer not running) ─────────────
  home_timer_lbl=lv_label_create(lv_scr_act());
  lv_label_set_text(home_timer_lbl,"");
  lv_obj_set_style_text_font(home_timer_lbl,&lv_font_montserrat_14,0);
  lv_obj_set_style_text_color(home_timer_lbl,lv_color_make(100,200,255),0);
  lv_obj_align(home_timer_lbl,LV_ALIGN_BOTTOM_LEFT,8,-4);
  if (!timer_running) lv_obj_add_flag(home_timer_lbl,LV_OBJ_FLAG_HIDDEN);

  // ── Bell icon (bottom-right, shown only when alarm is enabled) ──────────
  home_bell_lbl = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_color(home_bell_lbl, lv_color_make(255, 220, 60), 0);
  lv_obj_set_style_text_opa(home_bell_lbl, LV_OPA_70, 0);
  lv_obj_align(home_bell_lbl, LV_ALIGN_BOTTOM_RIGHT, -8, -4);
  update_home_bell();  // set text + hidden state from cfg

  // Show immediately rather than waiting one full second
  clock_tick_cb(nullptr);
  clock_timer = lv_timer_create(clock_tick_cb, 1000, nullptr);
}

// ══════════════════════════════════════════════════════════════════════════════
//  APPS MENU — long-press the smile GIF (upper-left tap) → math gate → 3 games
//
//  Flow:  UL tap → smile GIF opens → long-press GIF → math challenge
//         Correct answer → apps carousel (RPS / Dice / Coin)
//         Tap item → game  Tap to play again  Long-press to go back
//         Long-press in carousel → close all, return to clock
// ══════════════════════════════════════════════════════════════════════════════

// ── Note frequencies and menu tone helpers ────────────────────────────────────
// menu_tone() is a short blocking call — safe in LVGL timer callbacks because
// the longest note (500ms) is still shorter than any LVGL watchdog threshold.
#define NOTE_C4  262
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_A5  880
#define NOTE_B5  988
#define NOTE_HI  1046  // high C6

static void menu_tone(int freq, int ms)
{
  if (!cfg.menu_sounds) return;
  ledcChangeFrequency(BUZZER_PIN, freq, 8);   // change freq on already-attached pin
  ledcWrite(BUZZER_PIN, 96);
  delay(ms);
  ledcWrite(BUZZER_PIN, 0);
  ledcChangeFrequency(BUZZER_PIN, 2000, 8);   // restore alarm freq
}

static void menu_play_success()
{
  if (!cfg.menu_sounds) return;
  // successMelody: A5 B5 C5 B5 C5 D5 C5 D5 E5 D5 E5 E5 @100ms each
  static const int mel[] = {
    880,988,523,494, 523,587,523,587, 659,587,659,659
  };
  for (int i = 0; i < 12; i++) menu_tone(mel[i], 100);
}

static void menu_play_failure()
{
  if (!cfg.menu_sounds) return;
  menu_tone(NOTE_G4, 250);
  menu_tone(NOTE_C4, 500);
}

static void menu_tone_beep() { menu_tone(NOTE_A4, 60); }
static void menu_tone_hi()   { menu_tone(NOTE_HI, 80); }

// ── Apps state ────────────────────────────────────────────────────────────────
// apps_cont declared globally above wifi_poll_cb
static int         apps_idx      = 0;   // 0=RPS  1=Dice  2=Coin
static int         app_subphase  = 0;   // 0=carousel  1=game
static lv_timer_t *app_anim_timer  = nullptr;
static int         app_anim_step   = 0;
static int         app_anim_result = 0;  // cpu choice for RPS, roll for Dice
static lv_obj_t   *app_anim_lbl    = nullptr;  // main art label
static lv_obj_t   *app_anim_num    = nullptr;  // countdown number (RPS)
static lv_obj_t   *math_cont     = nullptr;
static int         math_answer   = 0;
static lv_timer_t *math_fail_tmr = nullptr;

// Forward declarations
static void show_apps(void);
static void apps_carousel_build(void);
static void app_screen_start(void);
static void app_screen_result(int data);
static void app_anim_stop(void);

// ── Math problem generator ────────────────────────────────────────────────────
static void math_generate(char *buf, int blen, int opts[4])
{
  int a, b, op = random(4);
  switch (op) {
    case 0: math_answer = random(5, 94); a = random(1, math_answer); b = math_answer - a;
            snprintf(buf, blen, "%d + %d = ?", a, b); break;
    case 1: a = random(11, 99); b = random(1, a - 1); math_answer = a - b;
            snprintf(buf, blen, "%d - %d = ?", a, b); break;
    case 2: a = random(2, 9); b = random(2, max(2, (int)(99 / a)));
            math_answer = a * b; snprintf(buf, blen, "%d x %d = ?", a, b); break;
    default: math_answer = random(2, 15); b = random(2, 8); a = math_answer * b;
             snprintf(buf, blen, "%d / %d = ?", a, b); break;
  }
  opts[0] = math_answer;
  int wi = 1;
  while (wi < 4) {
    int delta = (random(2) ? 1 : -1) * random(1, 19);
    int cand  = math_answer + delta;
    if (cand < 1)  cand = math_answer + wi * 13;
    if (cand > 99) cand = math_answer - wi * 11;
    if (cand < 1)  cand = wi + 2;
    bool dup = false;
    for (int k = 0; k < wi; k++) if (opts[k] == cand) { dup = true; break; }
    if (!dup) opts[wi++] = cand;
  }
  // Fisher-Yates shuffle
  for (int i = 3; i > 0; i--) {
    int j = random(i + 1), t = opts[i]; opts[i] = opts[j]; opts[j] = t;
  }
}

static void math_close_overlay()
{
  if (tilt_timer) { lv_timer_del(tilt_timer); tilt_timer = nullptr; }
  emotion_tilt_active = false; emotion_current_gif = nullptr;
  buzzer_stop();
  if (overlay_cont) { lv_obj_del(overlay_cont); overlay_cont = nullptr; }
}

static void math_fail_cb(lv_timer_t *t)
{
  lv_timer_del(t); math_fail_tmr = nullptr;
  if (math_cont) { lv_obj_del(math_cont); math_cont = nullptr; }
  math_close_overlay();
}

static void math_btn_cb(lv_event_t *e)
{
  if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
  int chosen = (int)(intptr_t)lv_event_get_user_data(e);
  if (math_cont) { lv_obj_del(math_cont); math_cont = nullptr; }

  if (chosen == math_answer) {
    menu_play_success();
    math_close_overlay();
    show_apps();
  } else {
    menu_play_failure();
    // Wrong: show big white X for 3 s, then return to clock
    math_cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(math_cont, 320, 172);
    lv_obj_align(math_cont, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(math_cont, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(math_cont, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(math_cont, 0, 0);
    lv_obj_set_style_pad_all(math_cont, 0, 0);
    lv_obj_clear_flag(math_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t *xl = lv_label_create(math_cont);
    lv_label_set_text(xl, "X");
    lv_obj_set_style_text_font(xl, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(xl, lv_color_white(), 0);
    lv_obj_align(xl, LV_ALIGN_CENTER, 0, 0);
    math_fail_tmr = lv_timer_create(math_fail_cb, 3000, nullptr);
    lv_timer_set_repeat_count(math_fail_tmr, 1);
  }
}

static void show_math_challenge()
{
  if (math_cont) return;
  char prob[32]; int opts[4];
  math_generate(prob, sizeof(prob), opts);

  math_cont = lv_obj_create(lv_scr_act());
  lv_obj_set_size(math_cont, 310, 162);
  lv_obj_align(math_cont, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_bg_color(math_cont, lv_color_make(6, 6, 28), 0);
  lv_obj_set_style_bg_opa(math_cont, LV_OPA_COVER, 0);
  lv_obj_set_style_border_color(math_cont, lv_color_make(60, 80, 220), 0);
  lv_obj_set_style_border_width(math_cont, 2, 0);
  lv_obj_set_style_radius(math_cont, 6, 0);
  lv_obj_set_style_pad_all(math_cont, 0, 0);
  lv_obj_clear_flag(math_cont, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *title = lv_label_create(math_cont);
  lv_label_set_text(title, "Solve to enter:");
  lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(title, lv_color_make(140, 140, 210), 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 8);

  lv_obj_t *prob_lbl = lv_label_create(math_cont);
  lv_label_set_text(prob_lbl, prob);
  lv_obj_set_style_text_font(prob_lbl, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(prob_lbl, lv_color_white(), 0);
  lv_obj_align(prob_lbl, LV_ALIGN_CENTER, 0, -18);

  for (int i = 0; i < 4; i++) {
    lv_obj_t *btn = lv_btn_create(math_cont);
    lv_obj_set_size(btn, 60, 32);
    lv_obj_set_pos(btn, 20 + i * 68, 120);
    lv_obj_set_style_bg_color(btn, lv_color_make(30, 50, 160), 0);
    lv_obj_set_style_radius(btn, 4, 0);
    lv_obj_add_event_cb(btn, math_btn_cb, LV_EVENT_CLICKED, (void*)(intptr_t)opts[i]);
    lv_obj_t *lbl = lv_label_create(btn);
    char ns[8]; snprintf(ns, sizeof(ns), "%d", opts[i]);
    lv_label_set_text(lbl, ns);
    lv_obj_set_style_text_font(lbl, &dejavu_mono_14, 0);
    lv_obj_center(lbl);
  }
}

static void apps_gif_longpress_cb(lv_event_t *e)
{
  if (lv_event_get_code(e) != LV_EVENT_LONG_PRESSED) return;
  lv_indev_wait_release(lv_indev_get_act());
  show_math_challenge();
}

// ── Apps lifecycle ────────────────────────────────────────────────────────────
static void app_anim_stop()
{
  if (app_anim_timer) { lv_timer_del(app_anim_timer); app_anim_timer = nullptr; }
  app_anim_lbl = nullptr;
  app_anim_num = nullptr;
  app_anim_step = 0;
}

static void apps_close()
{
  app_anim_stop();
  if (apps_cont) { lv_obj_del(apps_cont); apps_cont = nullptr; }
  app_subphase = 0;
}

static void apps_longpress_cb(lv_event_t *e)
{
  if (lv_event_get_code(e) != LV_EVENT_LONG_PRESSED) return;
  lv_indev_wait_release(lv_indev_get_act());
  if (app_subphase > 0) {
    app_subphase = 0;
    apps_carousel_build();
  } else {
    apps_close();
  }
}

static void apps_left_cb(lv_event_t *e)
{ if(lv_event_get_code(e)==LV_EVENT_PRESSED){apps_idx=(apps_idx+3)%4;apps_carousel_build();} }
static void apps_right_cb(lv_event_t *e)
{ if(lv_event_get_code(e)==LV_EVENT_PRESSED){apps_idx=(apps_idx+1)%4;apps_carousel_build();} }

// Transparent full-screen tap zone helper for game screens
static lv_obj_t *app_tapzone(lv_obj_t *p, lv_event_cb_t cb)
{
  lv_obj_t *z = lv_obj_create(p);
  lv_obj_set_size(z, 320, 172); lv_obj_set_pos(z, 0, 0);
  lv_obj_set_style_bg_opa(z, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(z, 0, 0); lv_obj_set_style_pad_all(z, 0, 0);
  lv_obj_set_style_radius(z, 0, 0); lv_obj_clear_flag(z, LV_OBJ_FLAG_SCROLLABLE);
  if (cb) lv_obj_add_event_cb(z, cb, LV_EVENT_CLICKED, nullptr);
  lv_obj_add_event_cb(z, apps_longpress_cb, LV_EVENT_LONG_PRESSED, nullptr);
  return z;
}

// ── ASCII art ─────────────────────────────────────────────────────────────────
static const char *dice_art_roll(int n)
{
  switch (n) {
    case 1: return
      "    ______    \n"
      "  /\\     \\  \n"
      " /o \\  o  \\ \n"
      "/   o\\_____\\\n"
      "\\o   /o    / \n"
      " \\ o/  o  /  \n"
      "  \\/____o/   ";
    case 2: return
      "    ______    \n"
      "  /\\     \\  \n"
      " /  \\  o  \\ \n"
      "/ o  \\_____\\\n"
      "\\  o /o    / \n"
      " \\  /  o  /  \n"
      "  \\/____o/   ";
    default: return
      // "  .-------.\n"
      // " /   o   /|\n"
      // "/_______/o|\n"
      // "| o     | |\n"
      // "|   o   |o/\n"
      // "|     o |/ \n"
      // "'-------'  ";
      "    ______    \n"
      "  /\\ o   \\  \n"
      " /o \\  o  \\ \n"
      "/   o\\___o_\\\n"
      "\\o   /     / \n"
      " \\ o/  o  /  \n"
      "  \\/_____/   ";
  }
}

static const char *dice_art(int n)
{
  switch (n) {
    case 1: return
      ".-------.\n"
      "|       |\n"
      "|   o   |\n"
      "|       |\n"
      "\'-------\'";
    case 2: return
      ".-------.\n"
      "| o     |\n"
      "|       |\n"
      "|     o |\n"
      "\'-------\'";
    case 3: return
      ".-------.\n"
      "| o     |\n"
      "|   o   |\n"
      "|     o |\n"
      "\'-------\'";
    case 4: return
      ".-------.\n"
      "| o   o |\n"
      "|       |\n"
      "| o   o |\n"
      "\'-------\'";
    case 5: return
      ".-------.\n"
      "| o   o |\n"
      "|   o   |\n"
      "| o   o |\n"
      "\'-------\'";
    default: return
      ".-------.\n"
      "| o   o |\n"
      "| o   o |\n"
      "| o   o |\n"
      "\'-------\'";
  }
}

// ── Shake animation art ──────────────────────────────────────────────────────
static const char *rps_art_shake(bool up)
{
  if (up) return
    "    _______      \n"
    "---'   ____)     \n"
    "      (_____)    \n"
    "      (_____)    \n"
    "      (____)     \n"
    "---.__(___)      \n"
    "                   ";
  return
    "                 \n"
    "    _______      \n"
    "---'   ____)     \n"
    "      (_____)    \n"
    "      (_____)    \n"
    "      (____)     \n"
    " ---.__(___)       ";
}

static const char *rps_art(int c)
{
  if (c==1) return
    "    _______       \n"
    "---'   ____)      \n"
    "      (_____)     \n"
    "      (_____)     \n"
    "      (____)      \n"
    " ---.__(___)        ";

  if (c==2) return
    "    ________      \n"
    "---'    ____)____ \n"
    "           ______)\n"
    "          _______)\n"
    "         _______) \n"
    " ---.__________)    ";

  return
    "    _______       \n"
    "---'   ____)____  \n"
    "          ______) \n"
    "       __________)\n"
    "      (____)      \n"
    " ---.__(___)        ";
}

static const char *rps_name(int c)
{ return c==1?"Rock":c==2?"Paper":"Scissors"; }
static const char *rps_result(int u, int cpu_c)
{
  if (u==cpu_c) return "TIE!";
  if ((u==1&&cpu_c==3)||(u==2&&cpu_c==1)||(u==3&&cpu_c==2)) return "YOU WIN!";
  return "CPU WINS!";
}

static const char *coin_art(bool heads)
{
  return heads
    ? "       \\:.             .:/\n"
      "        \\``._________.''/\n"
      "         \\             /\n"
      " .--.--, / .':.   .':. \\\n"
      "/__:  /  | '::' . '::' |\n"
      "   / /   |`.   ._.   .'|\n"
      "  / /    |.'         '.|\n"
      " /___-_-,|.\\  \\   /  /.|\n"
      "      // |''\\.;   ;,/ '|\n"
      "      `==|:=         =:|\n"
      "         `.          .'\n"
      "           :-._____.-:\n"
      "          `''       `''\n"
    : "                                 ,'\\\n"
      "   _.----.        ____         ,'  _\\   ___    ___     ____\n"
      ",-'       `.     |    |  /`.   \\,-'    |   \\  /   |   |    \\  |`.\n"
      "\\      __    \\    '-.  | /   `.  ___    |    \\/    |   '-.   \\ |  |\n"
      " \\.    \\ \\   |  __  |  |/    ,','_  `.  |          | __  |    \\|  |\n"
      "   \\    \\/   /,' _`.|      ,' / / / /   |          ,' _`.|     |  |\n"
      "    \\     ,-'/  /   \\    ,'   | \\/ / ,`.|         /  /   \\  |     |\n"
      "     \\    \\ |   \\_/  |   `-.  \\    `'  /|  |    ||   \\_/  | |\\    |\n"
      "      \\    \\ \\      /       `-.`.___,-' |  |\\  /| \\      /  | |   |\n"
      "       \\    \\ `.__,'|  |`-._    `|      |__| \\/ |  `.__,'|  | |   |\n"
      "        \\_.-'       |__|    `-._ |              '-.|     '-.| |   |\n"
      "                                `'                            '-._|\n";
}

// Common bottom hint
static void app_hint(lv_obj_t *p, const char *text)
{
  lv_obj_t *h = lv_label_create(p);
  lv_label_set_text(h, text);
  lv_obj_set_style_text_font(h, &dejavu_mono_14, 0);
  lv_obj_set_style_text_color(h, lv_color_make(70, 70, 95), 0);
  lv_obj_set_style_text_opa(h, LV_OPA_70, 0);
  lv_obj_align(h, LV_ALIGN_BOTTOM_MID, 0, -4);
}

// ── Game result screen ────────────────────────────────────────────────────────
static void app_repeat_tap_cb(lv_event_t *e)
{ if(lv_event_get_code(e)==LV_EVENT_CLICKED) app_screen_start(); }

static void app_screen_result(int data)
{
  lv_obj_clean(apps_cont);
  app_subphase = 1;
  lv_obj_add_event_cb(apps_cont, apps_longpress_cb, LV_EVENT_LONG_PRESSED, nullptr);

  lv_obj_t *art = lv_label_create(apps_cont);
  lv_obj_set_style_text_font(art, &dejavu_mono_14, 0);
  lv_obj_set_style_text_color(art, lv_color_white(), 0);
  lv_obj_set_style_text_align(art, LV_TEXT_ALIGN_CENTER, 0);

  lv_obj_t *res = lv_label_create(apps_cont);
  lv_obj_set_style_text_font(res, &dejavu_mono_16, 0);
  lv_obj_set_style_text_align(res, LV_TEXT_ALIGN_CENTER, 0);

  if (apps_idx == 0) {
    // RPS: data = user choice, pick cpu now
    int cpu = random(1, 4);
    const char *verdict = rps_result(data, cpu);
    char buf[120];
    snprintf(buf, sizeof(buf), "%s\nvs\n%s", rps_art(data), rps_art(cpu));
    lv_label_set_text(art, buf);
    lv_label_set_long_mode(art, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(art, 280);
    lv_obj_align(art, LV_ALIGN_CENTER, 0, -28);
    lv_label_set_text(res, verdict);
    lv_obj_set_style_text_color(res,
      strcmp(verdict,"TIE!")==0     ? lv_color_make(220,220,60) :
      strcmp(verdict,"YOU WIN!")==0 ? lv_color_make(60,220,60)  :
                                      lv_color_make(220,60,60), 0);
    lv_obj_align(res, LV_ALIGN_BOTTOM_MID, 0, -36);
  } else if (apps_idx == 1) {
    // Dice: data = 1-6
    lv_label_set_text(art, dice_art(data));
    lv_obj_align(art, LV_ALIGN_CENTER, 0, -16);
    char buf[782]; snprintf(buf, sizeof(buf), "Rolled: %d", data);
    lv_label_set_text(res, buf);
    lv_obj_set_style_text_color(res, lv_color_make(100, 200, 255), 0);
    lv_obj_align(res, LV_ALIGN_BOTTOM_MID, 0, -36);
  } else {
    // Coin: data = 0=heads 1=tails
    menu_tone_hi();  // hi-tone on flip result
    lv_obj_set_style_text_font(art, &dejavu_mono_8, 0);
    lv_label_set_long_mode(art, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(art, LV_TEXT_ALIGN_LEFT, 0);
    lv_label_set_text(art, coin_art(data == 0));
    lv_obj_align(art, LV_ALIGN_CENTER, 0, -16);
    lv_label_set_text(res, data==0 ? "HEADS!" : "TAILS!");
    lv_obj_set_style_text_color(res,
      data==0 ? lv_color_make(255,220,60) : lv_color_make(180,180,255), 0);
    lv_obj_align(res, LV_ALIGN_BOTTOM_MID, 0, -26);
  }

  app_hint(apps_cont, "tap again  .  hold to exit");
  app_tapzone(apps_cont, app_repeat_tap_cb);  // on top, transparent
}

// ── RPS animation timer (3-2-1 shake then reveal) ────────────────────────────
static void rps_anim_tick_cb(lv_timer_t * /*t*/)
{
  if (!apps_cont || !app_anim_lbl) { app_anim_stop(); return; }

  if (app_anim_step < 7) {
    // Steps 0-5: up/down × 3, countdown 3-2-1
    bool up = (app_anim_step % 2 == 0);
    lv_label_set_text(app_anim_lbl, rps_art_shake(up));
    if (!up) menu_tone_beep();  // beep on each DOWN move
    if (app_anim_num) {
      int count = 3 - (app_anim_step / 2);
      char buf[4]; snprintf(buf, sizeof(buf), "%d", count);
      lv_label_set_text(app_anim_num, buf);
    }
    app_anim_step++;
  } else {
    // Animation done — reveal cpu art + GO!
    menu_tone_hi();  // high tone on GO!
    app_anim_stop();
    lv_obj_clean(apps_cont);
    lv_obj_add_event_cb(apps_cont, apps_longpress_cb, LV_EVENT_LONG_PRESSED, nullptr);

    lv_obj_t *art = lv_label_create(apps_cont);
    lv_obj_set_style_text_font(art, &dejavu_mono_14, 0);
    lv_obj_set_style_text_color(art, lv_color_white(), 0);
    lv_obj_set_style_text_align(art, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(art, rps_art(app_anim_result));
    lv_label_set_long_mode(art, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(art, 290);
    lv_obj_align(art, LV_ALIGN_CENTER, 0, -20);

    lv_obj_t *go = lv_label_create(apps_cont);
    lv_label_set_text(go, "GO!");
    lv_obj_set_style_text_font(go, &dejavu_mono_16, 0);
    lv_obj_set_style_text_color(go, lv_color_make(60, 220, 60), 0);
    lv_obj_align(go, LV_ALIGN_BOTTOM_MID, 0, -36);

    app_hint(apps_cont, "tap again  .  hold to exit");
    app_tapzone(apps_cont, app_repeat_tap_cb);
  }
}

// ── Dice animation timer (rolling → result) ───────────────────────────────────
static void dice_anim_tick_cb(lv_timer_t * /*t*/)
{
  if (!apps_cont || !app_anim_lbl) { app_anim_stop(); return; }

  if (app_anim_step < 3) {
    // Steps 0-2: show three shake frames (no text)
    lv_label_set_text(app_anim_lbl, dice_art_roll(app_anim_step + 1));
    menu_tone_beep();
    app_anim_step++;
  } else {
    // Animation done — show final dice face
    menu_tone_hi();  // hi-tone on result
    app_anim_stop();
    app_screen_result(app_anim_result);  // app_anim_result = 1-6
  }
}

// ── Start RPS animation ───────────────────────────────────────────────────────
static void app_screen_rps_start()
{
  lv_obj_clean(apps_cont);
  app_subphase = 1;
  app_anim_step   = 0;
  app_anim_result = random(1, 4);  // cpu choice decided now

  lv_obj_add_event_cb(apps_cont, apps_longpress_cb, LV_EVENT_LONG_PRESSED, nullptr);

  app_anim_lbl = lv_label_create(apps_cont);
  lv_obj_set_style_text_font(app_anim_lbl, &dejavu_mono_14, 0);
  lv_obj_set_style_text_color(app_anim_lbl, lv_color_white(), 0);
  lv_obj_set_style_text_align(app_anim_lbl, LV_TEXT_ALIGN_CENTER, 0);
  lv_label_set_long_mode(app_anim_lbl, LV_LABEL_LONG_WRAP);
  lv_obj_set_width(app_anim_lbl, 290);
  lv_label_set_text(app_anim_lbl, rps_art_shake(true));
  lv_obj_align(app_anim_lbl, LV_ALIGN_CENTER, 0, -24);

  app_anim_num = lv_label_create(apps_cont);
  lv_obj_set_style_text_font(app_anim_num, &dejavu_mono_16, 0);
  lv_obj_set_style_text_color(app_anim_num, lv_color_make(220, 220, 60), 0);
  lv_obj_set_style_text_align(app_anim_num, LV_TEXT_ALIGN_CENTER, 0);
  lv_label_set_text(app_anim_num, "3");
  lv_obj_align(app_anim_num, LV_ALIGN_BOTTOM_MID, 0, -20);

  app_anim_timer = lv_timer_create(rps_anim_tick_cb, 250, nullptr);
}

// ── Start Dice animation ──────────────────────────────────────────────────────
static void app_screen_dice_start()
{
  lv_obj_clean(apps_cont);
  app_subphase = 1;
  app_anim_step   = 0;
  app_anim_result = random(1, 7);  // 1-6, decided now

  lv_obj_add_event_cb(apps_cont, apps_longpress_cb, LV_EVENT_LONG_PRESSED, nullptr);

  app_anim_lbl = lv_label_create(apps_cont);
  lv_obj_set_style_text_font(app_anim_lbl, &dejavu_mono_14, 0);
  lv_obj_set_style_text_color(app_anim_lbl, lv_color_white(), 0);
  lv_obj_set_style_text_align(app_anim_lbl, LV_TEXT_ALIGN_CENTER, 0);
  lv_label_set_text(app_anim_lbl, dice_art_roll(1));
  lv_obj_align(app_anim_lbl, LV_ALIGN_CENTER, 0, -10);
  app_anim_num = nullptr;

  app_anim_timer = lv_timer_create(dice_anim_tick_cb, 500, nullptr);
}

// ── App start tap: coin only (rps+dice use their own starters) ────────────────
static void app_start_tap_cb(lv_event_t *e)
{
  if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
  if (apps_idx == 2) { app_screen_result(random(2)); return; }  // coin 0-1
}

// ── Game start screen ─────────────────────────────────────────────────────────
static void app_screen_start()
{
  app_anim_stop();  // cancel any running animation before starting new one

  if (apps_idx == 0) {
    app_screen_rps_start();
    return;
  }
  if (apps_idx == 1) {
    app_screen_dice_start();
    return;
  }

  // ── Coin: tap anywhere to flip ────────────────────────────────────────────
  lv_obj_clean(apps_cont);
  app_subphase = 1;
  lv_obj_add_event_cb(apps_cont, apps_longpress_cb, LV_EVENT_LONG_PRESSED, nullptr);

  lv_obj_t *t = lv_label_create(apps_cont);
  lv_label_set_text(t, "Flip a Coin");
  lv_obj_set_style_text_font(t, &dejavu_mono_16, 0);
  lv_obj_set_style_text_color(t, lv_color_white(), 0);
  lv_obj_align(t, LV_ALIGN_CENTER, 0, -28);

  lv_obj_t *action = lv_label_create(apps_cont);
  lv_label_set_text(action, "[ tap to flip ]");
  lv_obj_set_style_text_font(action, &dejavu_mono_16, 0);
  lv_obj_set_style_text_color(action, lv_color_make(100, 200, 100), 0);
  lv_obj_align(action, LV_ALIGN_CENTER, 0, 8);

  app_hint(apps_cont, "tap to flip  .  hold to exit");
  app_tapzone(apps_cont, app_start_tap_cb);
}

// ── Apps carousel tap (enter game) ───────────────────────────────────────────
static void apps_tap_enter_cb(lv_event_t *e)
{
  if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
  if (apps_idx == 3) {
    cfg.menu_sounds = !cfg.menu_sounds;
    save_config();
    if (cfg.menu_sounds) menu_tone_hi();  // confirm it's on
    apps_carousel_build();
  } else {
    app_screen_start();
  }
}

// ── Apps carousel builder ─────────────────────────────────────────────────────
static void apps_carousel_build()
{
  lv_obj_clean(apps_cont);
  app_subphase = 0;
  lv_obj_add_event_cb(apps_cont, apps_longpress_cb, LV_EVENT_LONG_PRESSED, nullptr);

  static const struct { const char *name; const char *desc; } items[4] = {
    {"Rock Paper Scissors", "An interactive ASCII Game"},
    {"Rolling Dice",        "An interactive ASCII Dice"},
    {"Flip a Coin",         "An interactive ASCII Coin"},
    {nullptr,               nullptr},  // item 3 = Sounds toggle
  };

  // Left arrow + zone
  lv_obj_t *larr = lv_label_create(apps_cont);
  lv_label_set_text(larr, LV_SYMBOL_LEFT);
  lv_obj_set_style_text_font(larr, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(larr, lv_color_make(80, 100, 180), 0);
  lv_obj_align(larr, LV_ALIGN_LEFT_MID, 6, 0);
  { lv_obj_t *z = lv_obj_create(apps_cont);
    lv_obj_set_size(z,60,172); lv_obj_set_pos(z,0,0);
    lv_obj_set_style_bg_opa(z,LV_OPA_TRANSP,0); lv_obj_set_style_border_width(z,0,0);
    lv_obj_set_style_pad_all(z,0,0); lv_obj_set_style_radius(z,0,0);
    lv_obj_clear_flag(z,LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(z,apps_left_cb,LV_EVENT_PRESSED,nullptr);
    lv_obj_add_event_cb(z,apps_longpress_cb,LV_EVENT_LONG_PRESSED,nullptr); }

  // Right arrow + zone
  lv_obj_t *rarr = lv_label_create(apps_cont);
  lv_label_set_text(rarr, LV_SYMBOL_RIGHT);
  lv_obj_set_style_text_font(rarr, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(rarr, lv_color_make(80, 100, 180), 0);
  lv_obj_align(rarr, LV_ALIGN_RIGHT_MID, -6, 0);
  { lv_obj_t *z = lv_obj_create(apps_cont);
    lv_obj_set_size(z,60,172); lv_obj_set_pos(z,260,0);
    lv_obj_set_style_bg_opa(z,LV_OPA_TRANSP,0); lv_obj_set_style_border_width(z,0,0);
    lv_obj_set_style_pad_all(z,0,0); lv_obj_set_style_radius(z,0,0);
    lv_obj_clear_flag(z,LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(z,apps_right_cb,LV_EVENT_PRESSED,nullptr);
    lv_obj_add_event_cb(z,apps_longpress_cb,LV_EVENT_LONG_PRESSED,nullptr); }

  if (apps_idx == 3) {
    // ── Sounds toggle (inline, mirrors WiFi toggle in settings) ──────────
    lv_obj_t *sicon = lv_label_create(apps_cont);
    lv_label_set_text(sicon, cfg.menu_sounds ? LV_SYMBOL_VOLUME_MAX : LV_SYMBOL_MUTE);
    lv_obj_set_style_text_font(sicon, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(sicon,
      cfg.menu_sounds ? lv_color_make(80,200,120) : lv_color_make(180,60,60), 0);
    lv_obj_align(sicon, LV_ALIGN_CENTER, 0, -18);
    lv_obj_t *sdesc = lv_label_create(apps_cont);
    lv_label_set_text(sdesc, cfg.menu_sounds ? "Sounds: ON" : "Sounds: OFF");
    lv_obj_set_style_text_font(sdesc, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(sdesc,
      cfg.menu_sounds ? lv_color_make(80,200,120) : lv_color_make(180,60,60), 0);
    lv_obj_align(sdesc, LV_ALIGN_CENTER, 0, 28);
  } else {
    // ── Game item ─────────────────────────────────────────────────────────
    lv_obj_t *name_lbl = lv_label_create(apps_cont);
    lv_label_set_text(name_lbl, items[apps_idx].name);
    lv_obj_set_style_text_font(name_lbl, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(name_lbl, lv_color_white(), 0);
    lv_label_set_long_mode(name_lbl, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(name_lbl, 180);
    lv_obj_set_style_text_align(name_lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(name_lbl, LV_ALIGN_CENTER, 0, -14);

    // Description
    lv_obj_t *desc_lbl = lv_label_create(apps_cont);
    lv_label_set_text(desc_lbl, items[apps_idx].desc);
    lv_obj_set_style_text_font(desc_lbl, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(desc_lbl, lv_color_make(100, 180, 100), 0);
    lv_obj_align(desc_lbl, LV_ALIGN_CENTER, 0, 20);
  }

  // Centre tap zone — CLICKED enters, LONG_PRESSED exits
  { lv_obj_t *z = lv_obj_create(apps_cont);
    lv_obj_set_size(z,200,172); lv_obj_set_pos(z,60,0);
    lv_obj_set_style_bg_opa(z,LV_OPA_TRANSP,0); lv_obj_set_style_border_width(z,0,0);
    lv_obj_set_style_pad_all(z,0,0); lv_obj_set_style_radius(z,0,0);
    lv_obj_clear_flag(z,LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(z,apps_tap_enter_cb,LV_EVENT_CLICKED,nullptr);
    lv_obj_add_event_cb(z,apps_longpress_cb,LV_EVENT_LONG_PRESSED,nullptr); }

  // Hint above dots
  lv_obj_t *hint = lv_label_create(apps_cont);
  lv_label_set_text(hint, apps_idx == 3 ? "tap to toggle  .  hold to exit"
                                         : "tap to play  .  hold to exit");
  lv_obj_set_style_text_font(hint, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(hint, lv_color_make(70, 70, 95), 0);
  lv_obj_set_style_text_opa(hint, LV_OPA_60, 0);
  lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -18);

  // 4 position dots
  for (int i = 0; i < 4; i++) {
    lv_obj_t *dot = lv_label_create(apps_cont);
    lv_label_set_text(dot, i==apps_idx ? "\xe2\x97\x8f" : "\xe2\x97\x8b");
    lv_obj_set_style_text_color(dot, i==apps_idx ? lv_color_white() : lv_color_make(80,80,100), 0);
    lv_obj_set_pos(dot, 137 + i * 14, 156);
  }
}

// ── Open apps container ───────────────────────────────────────────────────────
static void show_apps()
{
  if (apps_cont) return;
  apps_idx = 0; app_subphase = 0;
  apps_cont = lv_obj_create(lv_scr_act());
  lv_obj_set_size(apps_cont, 320, 172);
  lv_obj_align(apps_cont, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_bg_color(apps_cont, lv_color_make(5, 8, 22), 0);
  lv_obj_set_style_bg_opa(apps_cont, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(apps_cont, 0, 0);
  lv_obj_set_style_pad_all(apps_cont, 0, 0);
  lv_obj_set_style_radius(apps_cont, 0, 0);
  lv_obj_clear_flag(apps_cont, LV_OBJ_FLAG_SCROLLABLE);
  apps_carousel_build();
}

// Zone callbacks
static void zone_ul_cb(lv_event_t *e)
{
  if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
  show_gif_fullscreen(GIF_SMILE_PATH);
  // Start emotion-tilt mode: tilt the device to change the GIF
  if (overlay_cont && imuReady) {
    emotion_tilt_active  = true;
    emotion_current_gif  = GIF_SMILE_PATH;
    if (!tilt_timer)
      tilt_timer = lv_timer_create(tilt_poll_cb, 400, nullptr);
    Serial.println("[EMOTION] Tilt mode active");
  }
  // Long-press on the smile GIF opens the apps menu (math challenge gate)
  if (overlay_cont) {
    lv_obj_add_event_cb(overlay_cont, apps_gif_longpress_cb, LV_EVENT_LONG_PRESSED, nullptr);
    lv_obj_t *gif_w = (lv_obj_t *)lv_obj_get_user_data(overlay_cont);
    if (gif_w)
      lv_obj_add_event_cb(gif_w, apps_gif_longpress_cb, LV_EVENT_LONG_PRESSED, nullptr);
  }
}

static void zone_ur_cb(lv_event_t *e)
{ if (lv_event_get_code(e) == LV_EVENT_CLICKED) show_gif_fullscreen(GIF_SLEEP_PATH); }

static void zone_ll_cb(lv_event_t *e)
{ if (lv_event_get_code(e) == LV_EVENT_CLICKED) show_status_screen(); }

static void zone_lr_cb(lv_event_t *e)
{ if (lv_event_get_code(e) == LV_EVENT_CLICKED) show_battery_screen(); }

static void home_screen_init(void)
{
  lv_obj_t *scr = lv_scr_act();
  lv_obj_set_style_bg_color(scr, lv_color_make(8, 8, 16), 0);
  lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
  lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

  // ── Splash: "Hello!" on cold boot, brief "Salut!" on wake-from-sleep
  home_hello_lbl = lv_label_create(scr);
  if (boot_from_sleep) {
    lv_label_set_text(home_hello_lbl, LV_SYMBOL_HOME "  Salut!");
    lv_obj_set_style_text_font(home_hello_lbl, &lv_font_montserrat_48, 0);
  } else {
    lv_label_set_text(home_hello_lbl, "Hello!");
    lv_obj_set_style_text_font(home_hello_lbl, &lv_font_montserrat_48, 0);
  }
  lv_obj_set_style_text_color(home_hello_lbl, lv_color_white(), 0);
  lv_obj_align(home_hello_lbl, LV_ALIGN_CENTER, 0, 0);

  // Cold boot: 2.5 s splash. Wake from sleep: 1.0 s (user pressed intentionally)
  uint32_t splash_ms = boot_from_sleep ? 1000 : 2500;
  lv_timer_t *splash = lv_timer_create(clock_face_show, splash_ms, nullptr);
  lv_timer_set_repeat_count(splash, 1);

  // ── Invisible 4-zone touch overlay ───────────────────────────────────────
  // Landscape 320×172. Each quadrant = 160×86 px.
  const struct { int16_t x; int16_t y; lv_event_cb_t cb; } zones[4] = {
    {   0,   0, zone_ul_cb },   // upper-left  → smile GIF
    { 160,   0, zone_ur_cb },   // upper-right → sleep GIF
    {   0,  86, zone_ll_cb },   // lower-left  → WiFi/NTP/date status
    { 160,  86, zone_lr_cb },   // lower-right → battery
  };
  // Long-press callback for all zones — opens alarm editor
  auto home_longpress = [](lv_event_t *e) {
    if (lv_event_get_code(e) != LV_EVENT_LONG_PRESSED) return;
    // Swallow the CLICKED that would fire on finger-lift after long press.
    // Without this the zone's CLICKED callback (show_gif / show_battery)
    // fires immediately after the alarm screen opens.
    lv_indev_wait_release(lv_indev_get_act());
    show_carousel();
  };

  for (int i = 0; i < 4; i++) {
    lv_obj_t *z = lv_obj_create(scr);
    lv_obj_set_size(z, 160, 86);
    lv_obj_set_pos(z, zones[i].x, zones[i].y);
    lv_obj_set_style_bg_opa(z, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(z, 0, 0);
    lv_obj_set_style_pad_all(z, 0, 0);
    lv_obj_set_style_radius(z, 0, 0);
    lv_obj_set_style_shadow_width(z, 0, 0);
    lv_obj_clear_flag(z, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(z, zones[i].cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_add_event_cb(z, home_longpress, LV_EVENT_LONG_PRESSED, nullptr);
  }

  // ── Background timers ─────────────────────────────────────────────────────
  battery_timer = lv_timer_create(battery_timer_callback, 1000, nullptr);
  wifi_timer    = lv_timer_create(wifi_poll_cb, 5000, nullptr);
}

// ══════════════════════════════════════════════════════════════════════════════
//  SETUP
// ══════════════════════════════════════════════════════════════════════════════
void setup()
{
  // ── Wake cause — must be read before any peripheral init ───────────────────
  esp_sleep_wakeup_cause_t wakeup_cause = esp_sleep_get_wakeup_cause();
  boot_millis = millis();
  // Timer wakeup = alarm auto-wake (only wakeup source configured)
  boot_from_sleep = (wakeup_cause == ESP_SLEEP_WAKEUP_TIMER);

  // Snapshot RTC time immediately if waking from sleep
  if (boot_from_sleep) {
    time_t rtc_now = time(nullptr);
    // Sanity: RTC epoch must be > 2026-01-01; if not, battery died during sleep
    if (rtc_now < 1735689600UL) boot_from_sleep = false;
  }

  Serial.begin(115200);
  delay(500);  // give serial monitor time to connect
  Serial.println("\n\n========== BOOT ==========");
  Serial.printf("[BOOT] Wake cause: %s\n",
    wakeup_cause == ESP_SLEEP_WAKEUP_TIMER     ? "TIMER — alarm auto-wake" :
    wakeup_cause == ESP_SLEEP_WAKEUP_UNDEFINED ? "cold boot / RESET button" : "other");

  // ── Step 1: Pull ALL SPI CS lines HIGH before the bus starts ──────────────
  // This prevents any device from misinterpreting the SPI init sequence.
  Serial.println("[1] Pulling CS pins HIGH...");
  pinMode(SD_CS,  OUTPUT); digitalWrite(SD_CS,  HIGH);
  pinMode(14,     OUTPUT); digitalWrite(14,     HIGH);  // display CS
  Serial.println("    Done.");

  // ── Step 2: Start the shared SPI bus ONCE with all four pins ──────────────
  // gfx->begin() would call SPI.begin() internally, but only with SCK/MOSI.
  // By calling it here first with MISO included, both display and SD share
  // the already-configured peripheral. On ESP32-C6 a second SPI.begin() on
  // the same bus is a no-op, so this must come before gfx->begin().
  Serial.printf("[2] SPI.begin(SCK=%d, MISO=%d, MOSI=%d, CS=%d)...\n",
                SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  Serial.println("    Done.");

  // ── Step 3: Display ───────────────────────────────────────────────────────
  Serial.println("[3] Initialising display...");
  if (!gfx->begin()) {
    Serial.println("    ERROR: gfx->begin() failed!");
  } else {
    Serial.println("    gfx->begin() OK.");
  }
  lcd_reg_init();
  gfx->setRotation(ROTATION);
  gfx->fillScreen(RGB565_BLACK);
  // PWM backlight at 50% — replaces raw digitalWrite(HIGH)
  backlight_init();
  // Buzzer pin — attach PWM channel now so first beep has no latency
  ledcAttach(BUZZER_PIN, 2000, 8);
  ledcWrite(BUZZER_PIN, 0);  // ensure silence at boot
  Serial.println("    Display ready.");

  // ── Step 4: Touch ─────────────────────────────────────────────────────────
  Serial.println("[4] Initialising touch...");
  Wire.begin(Touch_I2C_SDA, Touch_I2C_SCL);
  bsp_touch_init(&Wire, Touch_RST, Touch_INT,
                 gfx->getRotation(), gfx->width(), gfx->height());
  Serial.println("    Touch ready.");

  // ── IMU (QMI8658) — shares the I2C bus already started for touch ─────────
  Serial.println("[4b] Initialising IMU...");  
  int imuErr = imu.init(imuCalib, IMU_ADDRESS);
  if (imuErr != 0) {
    Serial.printf("    IMU init failed (err=%d) — tilt control disabled\n", imuErr);
    imuReady = false;
  } else {
    Serial.println("    IMU ready.");
    imuReady = true;
  }

  // ── Step 5: SD card ───────────────────────────────────────────────────────
  // SPI bus is already up (Step 2). We pass the same SPI instance and a safe
  // 4 MHz clock. The display runs faster but SD is more sensitive to speed.
  Serial.println("[5] Mounting SD card...");
  Serial.printf("    CS=%d  SCK=%d  MISO=%d  MOSI=%d  speed=4MHz\n",
                SD_CS, SD_SCK, SD_MISO, SD_MOSI);

  bool mounted = SD.begin(SD_CS, SPI, 4000000);
  Serial.printf("    SD.begin() returned: %s\n", mounted ? "true" : "false");

  if (!mounted) {
    // Try once more at a lower speed in case of signal integrity issues
    Serial.println("    Retrying at 1 MHz...");
    SD.end();
    delay(100);
    mounted = SD.begin(SD_CS, SPI, 1000000);
    Serial.printf("    Retry returned: %s\n", mounted ? "true" : "false");
  }

  if (!mounted) {
    Serial.println("    ERROR: SD card mount failed!");
    Serial.println("    Check: card inserted? FAT32 formatted? wiring correct?");
    sdCardAvailable = false;
  } else {
    uint8_t  cardType = SD.cardType();
    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("    SD mounted OK — type: %s  size: %llu MB\n",
                  cardType == CARD_MMC  ? "MMC"  :
                  cardType == CARD_SD   ? "SD"   :
                  cardType == CARD_SDHC ? "SDHC" : "UNKNOWN",
                  cardSize);
    sdCardAvailable = true;

    // ── Load config.ini ──────────────────────────────────────────────────
    load_config();

    // BUG FIX: Only restore time from log if NOT waking from a scheduled alarm
    if (!boot_from_sleep) {
      restore_time_from_log();
    } else {
      Serial.println("    [TIME] Wakeup from alarm: skipping file restore to keep RTC sync.");
    }

    // Verify GIF file exists
    Serial.printf("    Checking for GIF at: %s\n", "/cruzr_emotions/cruzr_smile.gif");
    File chk = SD.open("/cruzr_emotions/cruzr_smile.gif", FILE_READ);
    if (chk) {
      Serial.printf("    GIF found — %u bytes\n", (unsigned)chk.size());
      chk.close();
    } else {
      Serial.println("    WARNING: GIF not found! Check path and filename exactly.");
      // List root directory to help diagnose path issues
      Serial.println("    SD root contents:");
      File root = SD.open("/");
      File entry = root.openNextFile();
      while (entry) {
        Serial.printf("      %s %s (%u bytes)\n",
                      entry.isDirectory() ? "[DIR]" : "     ",
                      entry.name(), (unsigned)entry.size());
        entry = root.openNextFile();
      }
      root.close();
    }
  }

  // ── Step 6: LVGL ──────────────────────────────────────────────────────────
  Serial.println("[6] Initialising LVGL...");
  lv_init();
  lv_tick_set_cb((lv_tick_get_cb_t)millis);  // v9: replaces LV_TICK_CUSTOM in lv_conf.h
#if LV_USE_LOG != 0
  lv_log_register_print_cb(my_print);
#endif

  screenWidth  = gfx->width();
  screenHeight = gfx->height();
  // ── LVGL v9 display driver ───────────────────────────────────────────────
  // 20-line partial render buffer. GIFs are 160×86 so RAM is not a bottleneck.
  uint32_t bufSize = screenWidth * 20 * sizeof(uint16_t);
  uint8_t *disp_buf = (uint8_t *)heap_caps_malloc(bufSize, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  if (!disp_buf) disp_buf = (uint8_t *)heap_caps_malloc(bufSize, MALLOC_CAP_8BIT);
  if (!disp_buf) { Serial.println("LVGL buffer alloc failed!"); return; }

  lv_display_t *disp = lv_display_create(screenWidth, screenHeight);
  lv_display_set_flush_cb(disp, my_disp_flush);
  // LV_DISPLAY_RENDER_MODE_PARTIAL = buffer smaller than full screen (saves RAM)
  lv_display_set_buffers(disp, disp_buf, nullptr, bufSize, LV_DISPLAY_RENDER_MODE_PARTIAL);
  lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);

  // ── v9 input driver ─────────────────────────────────────────────────────
  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, touchpad_read_cb);

  // ── Step 7: Register SD → LVGL filesystem drive 'S' ──────────────────────
  Serial.println("[7] Registering LVGL SD filesystem driver...");
  lvgl_sd_fs_init();
  // ── Step 7b: WiFi + NTP ───────────────────────────────────────────────────
  // addAP() queues the credential; actual connection happens in wifi_poll_cb()
  // (an LVGL timer, every 5 s) so setup() is never blocked.
  // configTime() starts the SNTP client; it syncs automatically once online.
  Serial.println("[7b] Applying WiFi state from config...");
  apply_wifi_state();
  Serial.println("     Done.");

  Serial.println("[8] Building UI...");
  home_screen_init();

  Serial.println("========== SETUP DONE ==========\n");
}

// ══════════════════════════════════════════════════════════════════════════════
//  LOOP — intentionally minimal
//  All logic (clock ticks, WiFi polling, brightness schedule, buzzer pattern,
//  alarm) runs as LVGL timer callbacks. loop() never blocks.
// ══════════════════════════════════════════════════════════════════════════════
void loop()
{
  lv_timer_handler();  // drive LVGL: renders, animations, timers
  delay(5);
}
