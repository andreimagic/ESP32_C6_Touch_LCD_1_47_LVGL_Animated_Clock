/*
 * board_config.h — hardware abstraction for the LVGL Animated Clock
 *
 * Supported targets
 *   ESP32-C6-Touch-LCD-1.47   (Waveshare, 8MB flash, no PSRAM, QMI8658 IMU)
 *   ESP32-S3-Touch-LCD-1.47   (Waveshare, 16MB flash, 8MB OPI PSRAM, NO IMU)
 *
 * Both boards use the SAME 1.47" 172x320 panel: JD9853 controller driven with
 * the ST7789 command set, plus an AXS5106L capacitive touch controller on I2C.
 * lcd_reg_init() is therefore identical on both and needs no board switch.
 *
 * Selection is on CONFIG_IDF_TARGET_*, which the ESP32 core always defines.
 * This deliberately does NOT key off ARDUINO_<BOARD> variant macros, because
 * those depend on which board entry the user happens to pick in the IDE.
 *
 * ---------------------------------------------------------------------------
 * PROVENANCE — where these numbers come from
 *   C6: the values this firmware has been running on since v1.0, cross-checked
 *       against docs.waveshare.com/ESP32-C6-Touch-LCD-1.47 pinout table.
 *   S3: decoded from the official Waveshare schematic netlist
 *       (ESP32-S3-Touch-LCD-1.47 schematic, net labels LCD_*, TP_*, SD_*,
 *       BAT_ADC). Display pins additionally confirmed against Waveshare's own
 *       01_gfx_helloworld Arduino demo.
 *
 *   !! ONE KNOWN CONFLICT — LCD_RST on the S3 !!
 *   The schematic nets say  IO40 = LCD_RST  and  IO47 = TP_RST.
 *   Waveshare's Arduino demo passes 47 as the ST7789 reset pin, which is the
 *   TOUCH reset line. We follow the schematic (40) because it is authoritative.
 *   If the panel stays black on the S3, try S3_LCD_RST = 47 before anything
 *   else: the demo may "work" only because the JD9853 self-resets at power-on
 *   while pin 47 incidentally resets the touch chip.
 * ---------------------------------------------------------------------------
 */

#pragma once

// ══════════════════════════════════════════════════════════════════════════════
//  ESP32-C6-Touch-LCD-1.47
// ══════════════════════════════════════════════════════════════════════════════
#if defined(CONFIG_IDF_TARGET_ESP32C6)

  #define BOARD_NAME        "ESP32-C6-Touch-LCD-1.47"

  // Display — shares the SPI bus with the TF card slot
  #define LCD_SCK           1
  #define LCD_MOSI          2
  #define LCD_CS            14
  #define LCD_DC            15
  #define LCD_RST           22
  #define GFX_BL            23

  // TF card — SAME bus as the display (GPIO1/2 shared, see Waveshare notes)
  #define SD_CS             4
  #define SD_SCK            1
  #define SD_MOSI           2
  #define SD_MISO           3

  // Touch — AXS5106L on the shared I2C bus
  #define Touch_I2C_SDA     18
  #define Touch_I2C_SCL     19
  #define Touch_RST         20
  #define Touch_INT         21

  #define BAT_PIN           0     // BAT_ADC, 200K/100K divider -> VBAT = ADC x 3
  #define BUZZER_PIN        5     // external passive buzzer, GPIO5 -> GND

  #define BOARD_HAS_IMU     1     // QMI8658A at 0x6B, shares I2C with touch
  #define BOARD_SD_SHARES_LCD_BUS 1

  // The C6 has only one general-purpose SPI host, shared by LCD and SD.
  // Arduino_HWSPI rides the global `SPI` object that setup() already begins.
  #define BOARD_NEW_LCD_BUS() \
      new Arduino_HWSPI(LCD_DC, LCD_CS, LCD_SCK, LCD_MOSI)

// ══════════════════════════════════════════════════════════════════════════════
//  ESP32-S3-Touch-LCD-1.47
// ══════════════════════════════════════════════════════════════════════════════
#elif defined(CONFIG_IDF_TARGET_ESP32S3)

  #define BOARD_NAME        "ESP32-S3-Touch-LCD-1.47"

  // Display — on its OWN pins, completely separate from the TF card
  #define LCD_SCK           38
  #define LCD_MOSI          39
  #define LCD_CS            21
  #define LCD_DC            45
  #define LCD_RST           40    // schematic net LCD_RST; see conflict note above
  #define GFX_BL            46    // drives LEDK through the 8050 transistor

  // TF card — dedicated pins. The slot is wired for BOTH SDMMC and SPI mode
  // (nets are labelled SD_D0..SD_D3/SD_CLK/SD_CMD *and* SD_MISO/SD_MOSI/
  // SD_SCLK/SD_CS, with 10K pull-ups on every line). We use SPI mode so the
  // existing SD.h / LVGL filesystem bridge needs no changes at all.
  #define SD_CS             14    // SD_D3 / SD_CS
  #define SD_SCK            16    // SD_CLK
  #define SD_MOSI           15    // SD_CMD
  #define SD_MISO           17    // SD_D0

  // Touch — AXS5106L, same part as the C6, different pins
  #define Touch_I2C_SDA     42
  #define Touch_I2C_SCL     41
  #define Touch_RST         47
  #define Touch_INT         48

  #define BAT_PIN           12    // BAT_ADC, also a 200K/100K divider -> x3
  #define BUZZER_PIN        5     // GPIO5 is free and broken out on header P1

  #define BOARD_HAS_IMU     0     // no IMU on this board — tilt features self-disable
  #define BOARD_SD_SHARES_LCD_BUS 0

  // The S3 has two free SPI hosts. Arduino's global `SPI` defaults to FSPI
  // (SPI2) and we keep that for the TF card, so the panel is given HSPI (SPI3)
  // explicitly. Passing the default would put both on FSPI and they would fight.
  #define BOARD_NEW_LCD_BUS() \
      new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCK, LCD_MOSI, GFX_NOT_DEFINED, HSPI)

#else
  #error "Unsupported target. Select 'ESP32C6 Dev Module' or 'ESP32S3 Dev Module' in Tools > Board."
#endif

// ══════════════════════════════════════════════════════════════════════════════
//  Common to every target
// ══════════════════════════════════════════════════════════════════════════════

// Panel geometry — identical on both boards (JD9853, 172x320, 34px col offset)
#define LCD_H_RES         172
#define LCD_V_RES         320
#define LCD_COL_OFFSET    34
#define LCD_ROW_OFFSET    0

#define ROTATION          1     // landscape, 320x172

