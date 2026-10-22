# 03 - Interfaces, error handling and device constraints

## Functions (as defined in `src/main.cpp`)

| Function | Signature | Lines | Purpose |
|---|---|---|---|
| `gregToJDN` | `long gregToJDN(int y, int m, int d)` | `73-76` | Gregorian date to JDN |
| `jdnToGreg` | `void jdnToGreg(long jdn, int &y, int &m, int &d)` | `77-83` | JDN to Gregorian date |
| `parseBuildDate` | `void parseBuildDate()` | `84-91` | Sets `bY`, `bM`, `bD`, `buildEpochDays` from `__DATE__` |
| `dim565` | `uint16_t dim565(uint16_t c, uint8_t num, uint8_t den)` | `93-96` | Scales a 565 colour |
| `seedStars` | `void seedStars()` | `98-104` | Random star positions and phases |
| `drawStars` | `void drawStars()` | `105-111` | Twinkling starfield into `fb` |
| `drawOrbits` | `void drawOrbits()` | `112-115` | Orbit ellipses into `fb` |
| `drawSun` | `void drawSun()` | `116-123` | Sun into `fb` |
| `drawPlanet` | `void drawPlanet(Planet &pl, float angle)` | `124-134` | One planet at an angle, into `fb` |
| `drawBtn` | `void drawBtn(int x, int y, int w, int h, const char *label)` | `143-149` | Rounded button |
| `drawUI` | `void drawUI()` | `151-170` | Date label, buttons, speed label, hint |
| `setup` | `void setup()` | `172-194` | Initialisation |
| `handleTouch` | `void handleTouch()` | `196-215` | Press-edge touch handling |
| `loop` | `void loop()` | `217-240` | One frame |

`Planet` (`src/main.cpp:30-35`): `int orbit`, `int size`, `uint16_t col`, `uint16_t hi`, `bool ring`. The table has `N = 8` entries (`37-47`).

## Global state

| Name | Type | Lines | Written by |
|---|---|---|---|
| `SCR_W`, `SCR_H`, `CX`, `CY` | `int` | `19` | `setup` (`179`) |
| `rstX`, `rstY`, `rstW`, `rstH` | `int` | `137` | `setup` (`180`) |
| `minX`, `plsX`, `spdY`, `spW`, `spH` | `int` | `138` | `setup` (`181`) |
| `warpIdx` | `int` | `139` | `handleTouch` (`208`, `210-211`) |
| `uiVisible` | `bool` | `59` | `handleTouch` (`205`) |
| `lastMs` | `uint32_t` | `60` | `setup` (`193`), `loop` (`220`) |
| `tsec` | `float` | `61` | `loop` (`222`) |
| `simSeconds` | `double` | `62` | `loop` (`223`), `handleTouch` (`208`) |
| `buildEpochDays` | `double` | `63` | `parseBuildDate` (`90`) |
| `bY`, `bM`, `bD` | `int` | `64` | `parseBuildDate` (`87`, `89`) |
| `wasTouched` | `bool` | `65` | `handleTouch` (`214`) |
| `starX`, `starY`, `starB`, `starPh` | arrays of 80 | `56-57` | `seedStars` |

`simSeconds`'s declaration comment says "seconds elapsed since boot (real time)" (`src/main.cpp:62`). The code adds warped time (`223`), so the comment does not match the code. See [application spec 04](../application-spec/04-requirements-testing-limitations.md#code-and-documentation-discrepancies-verified-by-reading-the-source).

## Hardware interfaces

| Interface | Object or call | Configuration | Source |
|---|---|---|---|
| Display SPI | `TFT_eSPI tft`, `TFT_eSprite fb` | Build flags only, no `User_Setup.h` | `src/main.cpp:16-17`, `platformio.ini:16-38` |
| Sprite | `fb.setColorDepth(8)`, `fb.createSprite(SCR_W, SCR_H)` | 8 bpp, full screen | `src/main.cpp:188-189` |
| Touch SPI | `SPIClass touchSPI(VSPI)`, `touchSPI.begin(T_CLK, T_MISO, T_MOSI, T_CS)` | Pins from `src/main.cpp:22-26` | `src/main.cpp:27`, `184` |
| Touch controller | `XPT2046_Touchscreen ts(T_CS, T_IRQ)` | `ts.begin(touchSPI)`, `ts.setRotation(1)` | `src/main.cpp:28`, `185` |
| Serial | `Serial.begin(115200)` | `platformio.ini:7` | `src/main.cpp:173` |
| Backlight | `pinMode(TFT_BL, OUTPUT)`, `digitalWrite(TFT_BL, HIGH)` | GPIO 21 | `src/main.cpp:183` |
| Time | `millis()` | Arduino core | `src/main.cpp:193`, `218` |
| Random | `randomSeed(esp_random())`, `random(...)` | Arduino and ESP-IDF | `src/main.cpp:174`, `100-101` |

The file has no interface to a host, a network, storage, or a second device.

## Error handling

| Condition | What the code does | Anchor |
|---|---|---|
| Sprite allocation fails | Prints `Sprite alloc FAILED` and continues. Drawing and `pushSprite` then run without the check. What happens on screen is **unverified**. | `src/main.cpp:189` |
| Build date does not parse | `sscanf` return value is not checked. Month falls back to 1 if the name is not found. | `src/main.cpp:87`, `89` |
| Touch coordinates out of range | `constrain` to the screen size. | `src/main.cpp:200-201` |
| Touch held down | Ignored after the press edge; `wasTouched` tracks state. | `src/main.cpp:198`, `214` |
| Touch in a dead zone | No action. | `src/main.cpp:204-212` |
| Time-warp index out of range | Clamped to 0..5. | `src/main.cpp:210-211` |
| Large frame gap (for example after a pause) | `dt` clamped to 0.1 s. | `src/main.cpp:221` |
| Watchdog or reboot on fault | None in the file (read in full). | `src/main.cpp:1-240` |
| Persistent error log | None. Serial is only used at startup. | `src/main.cpp:173`, `190` |

The file has no exception handling and no error return codes. Failures show up only on the serial port at startup, or not at all.

## Device constraints

| Constraint | Value | Derivation or source |
|---|---|---|
| RAM for the sprite | 76,800 bytes at 8 bpp (320 x 240 x 1) | Derived. `src/main.cpp:188-189`. README says about 76 KB (`README.md:24`). |
| RAM for the sprite at 16 bpp | 153,600 bytes | Derived. This does not fit on a WROOM-32 without PSRAM (`README.md:24`). Whether the allocation fails on this board is **unverified** in this repo. |
| Star arrays | 80 stars x 6 bytes = 480 bytes | Derived from types (`src/main.cpp:56-57`). |
| PSRAM | Assumed none | `README.md:24`. The repo has no PSRAM setting. |
| Panel size | 320x240 landscape (rotation 1) | `src/main.cpp:178-179`; `README.md:3` |
| Panel driver | `ILI9341_2_DRIVER` | `platformio.ini:18`; `README.md:9` |
| Colour order and inversion | BGR, inverted | `platformio.ini:29-30` |
| Display SPI clocks | 55 MHz writes, 20 MHz reads | `platformio.ini:35-36` |
| Touch SPI clock | 2.5 MHz | `platformio.ini:37` |
| Backlight | Always on (no dimming code) | `src/main.cpp:183` |
| Reset line | None (`TFT_RST=-1`) | `platformio.ini:26` |
| Frame timing | No cap and no measurement; frame rate **unverified** | `src/main.cpp:217-240` |
