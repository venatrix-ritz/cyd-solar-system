# 01 - Purpose, users and workflows

## Purpose
A real-time orrery for a 2.8" ILI9341 320x240 touch display on an ESP32-2432S028 board ("Cheap Yellow Display", CYD2USB variant) (`README.md:3`). It draws the Sun at the centre and eight planets on concentric orbit rings, with each planet placed at its mean longitude for a date (`src/main.cpp:37-53`, `README.md:5`).

Planets are placed for the **firmware build date**. There is no real-time clock and no network access in the firmware (`src/main.cpp:3`), so "today" is the compile-time `__DATE__` string (`src/main.cpp:84-91`).

## Scope
In scope (implemented in `src/main.cpp`):
- Orbit rings, Sun, eight planets (Mercury to Neptune), Saturn ring, a twinkling starfield (`src/main.cpp:105-134`).
- A simulated-date label and a time-warp control with six speeds (`src/main.cpp:140-141`, `151-170`).
- Touch handling for UI toggle, reset, and speed change (`src/main.cpp:196-215`).

Out of scope (not present in the code):
- Moons, asteroids, comets, the Moon, dwarf planets, or any body other than the eight planets and the Sun.
- Any data source other than the compile-time date and the constant tables in the file.
- Storage of settings or time between boots (no NVS, no `Preferences`, no SD code; the source has no such calls, verified by search of `src/main.cpp`).
- Any network, Bluetooth, or serial command input. Serial is output-only in the file (`src/main.cpp:173`, `190`).

## Users
- **Owner / builder:** a hobbyist who flashes the board and uses the touch screen. The README describes build-and-flash steps for this user (`README.md:12-16`).
- There is no multi-user, login, or remote role in the code.

## Workflows

### W1 - Build and flash
1. Set the upload and monitor ports in `platformio.ini` (`platformio.ini:9-10`; see [03](03-hardware-configuration.md)).
2. Run `pio run -e cyd2usb -t upload` from the project folder (`README.md:14`).
3. On boot, the firmware sets the simulated clock to the build date and starts at real time (`src/main.cpp:84-91`, `223`, `62`).

The README states that reflashing re-syncs the planets to the current day (`README.md:16`). The date comes from the compile machine's `__DATE__`, so the displayed date is the day the binary was compiled, not the day it was flashed (the code reads `__DATE__` at compile time, `src/main.cpp:87`).

### W2 - Watch the orrery
Boot, then the planets advance at real time (`WARP[0] = 1.0`, `src/main.cpp:140`). At real time, the date label advances only when the simulated offset passes half a day (see [design 02](../design-spec/02-algorithms-timing.md)).

### W3 - Change time warp
Press the `+` or `-` button in the bottom row to step through six speeds: real time, 1 hr/s, 1 day/s, 1 week/s, 1 month/s, 1 year/s (`src/main.cpp:140-141`, `209-211`). The index is clamped to 0..5 (`src/main.cpp:210-211`).

### W4 - Reset to the build date
Press **RESET TO TODAY** (`src/main.cpp:206-208`). This sets the simulated offset back to zero and the speed back to real time. Planets return to the build-date positions at the current real time.

### W5 - Hide the UI
Tap the top-right corner (`src/main.cpp:204-205`). This toggles the UI, which hides the orbit rings, the date label, the buttons, and the speed label (`src/main.cpp:231`, `237`). The corner tap still works while the UI is hidden; the other controls do not (`src/main.cpp:206`, `209` check `uiVisible`).
