# USB Mouse (ESP32-S3)

Tiny standalone firmware: the board enumerates as a plain USB mouse named
"USB Mouse" and nudges the pointer 2 px out and back every 30-40 s, so the
computer never goes idle. Plug it in and it works; no pairing or driver.

Needs native USB-OTG, so ESP32-S2/S3 only. The ESP32-C3/C6 USB port is
serial/JTAG and cannot act as a mouse. Built for the Seeed XIAO ESP32-S3.
Separate from the clock firmware and not part of the CI board matrix.

Tuning: `JIGGLE_EVERY_MS`, `JIGGLE_RANDOM_MS`, `JIGGLE_PIXELS` at the top of the sketch.

## Build

Same pinned core as the clock (`esp32:esp32@3.3.11`), from the repo root:

```sh
arduino-cli compile \
  --fqbn "esp32:esp32:XIAO_ESP32S3:USBMode=default,CDCOnBoot=cdc,PartitionScheme=default_8MB,DebugLevel=none" \
  --output-dir build-out/usb-jiggler usb_mouse_jiggler
```

`USBMode=default` is USB-OTG (TinyUSB), required. `CDCOnBoot=cdc` is "Disabled"
(the menu labels are swapped in this board definition), so the board shows up as
a mouse only, with no serial port.

## Flash

The core's `*.merged.bin` is padded to the whole chip; trim it to end after the
app (app starts at 0x10000), as `.devcontainer/scripts/build-full-bin.sh` does,
then write at 0x0:

```sh
esptool --chip esp32s3 --port /dev/ttyACM0 write-flash 0x0 usb-mouse-xiao-esp32s3-full.bin
```

Because the firmware has no serial port, re-flashing needs download mode:
hold BOOT while plugging in the USB cable, then run esptool.

## CI

`.github/workflows/usb-mouse.yml` builds this sketch and uploads the trimmed
`usb-mouse-xiao-esp32s3-<sha>-full.bin` as a workflow artifact whenever files in
this folder change. It is independent of `build.yml` / `release.yml`, so it never
affects clock releases or the web flasher.
