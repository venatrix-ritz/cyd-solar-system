# CYD Solar System

A real-time solar-system orrery for the **ESP32-2432S028 (CYD2USB)** — "Cheap Yellow Display", 2.8" ILI9341 320×240 with resistive touch.

Planets are placed at their **actual heliocentric longitudes** for the firmware build date, using J2000 mean orbital elements + mean daily motion (Kepler-exact period ratios). A flicker-free 8 bpp sprite is composited each frame and pushed in one shot.

## Hardware
- ESP32-2432S028, CYD2USB variant (one USB-C + one micro-USB)
- Display: ILI9341, driven with the **`ILI9341_2_DRIVER`** alternate init (the plain `ILI9341_DRIVER` corrupts this panel), BGR order, inversion on
- Touch: XPT2046 on a separate SPI bus

## Build & flash (PlatformIO)
```
pio run -e cyd2usb -t upload
```
Board enumerates as a CH340 USB-serial port (set in `platformio.ini`). "Today" comes from the compile-time `__DATE__`, so reflashing re-syncs the planets to the current day.

## Controls (touch)
- **Top-right corner** — show / hide the UI
- **− / +** (bottom row) — time warp: real time → 1 hr/s → 1 day/s → 1 week/s → 1 month/s → 1 year/s
- **RESET TO TODAY** — snap planets to the build-date positions at real time

## Notes
- No PSRAM on the WROOM-32, so the back buffer is 8 bpp (76 KB); 16 bpp won't allocate.
- Orbit **radii** are scaled for legibility, not to true AU; the **speeds/ratios** are physically real.
