# 03 - Hardware and build configuration

## Target hardware

| Item | Value | Source |
|---|---|---|
| Board | ESP32-2432S028, CYD2USB variant (one USB-C and one micro-USB) | `README.md:3`, `README.md:8` |
| PlatformIO board ID | `esp32dev` (generic) | `platformio.ini:5` |
| Platform | `espressif32` (not version-pinned) | `platformio.ini:4` |
| Framework | Arduino | `platformio.ini:6` |
| Display | ILI9341, 240x320 native, used in landscape rotation 1 | `platformio.ini:19-20`, `src/main.cpp:178` |
| Display colour order and inversion | BGR and inversion on | `platformio.ini:29-30`, `README.md:9` |
| Display driver | `ILI9341_2_DRIVER` (the README says the plain driver corrupts this panel) | `platformio.ini:18`, `README.md:9` |
| Touch | XPT2046 on a separate SPI bus | `src/main.cpp:22-28`, `README.md:10` |
| PSRAM | None assumed (README says WROOM-32 has none) | `README.md:24` |

The code does not name the board. The CYD identification comes from the README and the `platformio.ini` header comment (`platformio.ini:1-2`). The `esp32dev` board ID is the generic PlatformIO board.

## Pin map

Display (TFT_eSPI, from `build_flags`):

| Signal | GPIO | Source |
|---|---|---|
| MISO | 12 | `platformio.ini:21` |
| MOSI | 13 | `platformio.ini:22` |
| SCLK | 14 | `platformio.ini:23` |
| CS | 15 | `platformio.ini:24` |
| DC | 2 | `platformio.ini:25` |
| RST | -1 (none) | `platformio.ini:26` |
| Backlight | 21, active HIGH | `platformio.ini:27-28`, `src/main.cpp:183` |

Touch (XPT2046, from `src/main.cpp`):

| Signal | GPIO | Source |
|---|---|---|
| CS | 33 | `src/main.cpp:22` |
| IRQ | 36 | `src/main.cpp:23` |
| CLK | 25 | `src/main.cpp:24` |
| MOSI | 32 | `src/main.cpp:25` |
| MISO | 39 | `src/main.cpp:26` |

The touch bus is a separate `SPIClass touchSPI(VSPI)` (`src/main.cpp:27`), started with `touchSPI.begin(T_CLK, T_MISO, T_MOSI, T_CS)` (`src/main.cpp:184`).

## Build configuration

| Setting | Value | Source |
|---|---|---|
| Environment | `[env:cyd2usb]` (single env) | `platformio.ini:3` |
| Monitor speed | 115200 | `platformio.ini:7` |
| Upload speed | 921600 | `platformio.ini:8` |
| Upload and monitor port | Pinned in `platformio.ini` (`upload_port`, `monitor_port`); the value is not reproduced here | `platformio.ini:9-10` |
| `lib_deps` | `bodmer/TFT_eSPI@^2.5.43`; `XPT2046_Touchscreen` from a git URL with no tag or commit | `platformio.ini:12-14` |
| Config method | `-DUSER_SETUP_LOADED=1` plus `build_flags`; no `User_Setup.h` | `platformio.ini:16-17` |
| Driver | `-DILI9341_2_DRIVER=1` | `platformio.ini:18` |
| Size defines | `-DTFT_WIDTH=240`, `-DTFT_HEIGHT=320` | `platformio.ini:19-20` |
| Fonts | `LOAD_GLCD`, `LOAD_FONT2`, `LOAD_FONT4`, `SMOOTH_FONT` | `platformio.ini:31-34` |
| SPI clocks | Display 55 MHz write, 20 MHz read; touch 2.5 MHz | `platformio.ini:35-37` |
| SPI host | `USE_HSPI_PORT=1` | `platformio.ini:38` |

Notes:
- `LOAD_FONT4` is set in `platformio.ini:33`, but `src/main.cpp` uses only font IDs 1 and 2 (`src/main.cpp:148`, `158`, `165`, `169`). A search of `src/main.cpp` found no `4` font call. Whether the `LOAD_FONT4` flag affects the build beyond size is **unverified**.
- The `TFT_WIDTH`/`TFT_HEIGHT` defines (`platformio.ini:19-20`) are portrait values. The code switches to landscape with `setRotation(1)` and reads the size at runtime (`src/main.cpp:178-179`). The effect of the defines on the library is **unverified**.
- A memory-hub note outside this repo says `ILI9341_2_DRIVER` sets the size, so those defines should not be defined. That note is context only and **unverified** against this build.
- `.gitignore` excludes `.pio/`, `.vscode/`, and `*.bin` (`.gitignore:1-3`).

## Data and secrets
The repo contains no Wi-Fi credentials, tokens, or keys. The only device-specific item is the serial port pin in `platformio.ini`, which is intentionally left out of this spec.
