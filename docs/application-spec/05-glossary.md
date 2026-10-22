# 05 - Glossary

| Term | Meaning in this project |
|---|---|
| **Build epoch** | The number of days from J2000 to the build date, `buildEpochDays` (`src/main.cpp:63`, `90`). |
| **CYD / CYD2USB** | "Cheap Yellow Display": ESP32-2432S028 board with a 2.8" ILI9341 panel and XPT2046 touch. The "2USB" variant has USB-C and micro-USB (`README.md:3`, `8`). |
| **Days since J2000 (D)** | `buildEpochDays + simSeconds / 86400` (`src/main.cpp:227`). |
| **J2000** | The standard epoch, Julian Day 2451545 (`src/main.cpp:90`). |
| **JDN** | Julian Day Number, an integer day count used for date arithmetic (`gregToJDN`, `src/main.cpp:73-76`). |
| **Mean longitude (L0 + NDAY x D)** | The angle the code uses for each planet's position, in degrees (`src/main.cpp:50-53`, `234`). It is not the true longitude. |
| **Mean motion (NDAY)** | Degrees per day, 360 divided by the sidereal period in days (`src/main.cpp:52-53`). |
| **Orbit radius (legibility)** | The on-screen radius in pixels, scaled for display, not in AU (`src/main.cpp:31`). |
| **Planet ring (Saturn)** | The three-ellipse ring drawn around Saturn (`src/main.cpp:44`, `127-131`). |
| **pushSprite** | Copies the full 8-bpp sprite to the display in one call (`src/main.cpp:239`). |
| **Sim time** | The warped clock, `simSeconds`, in seconds since the last reset (`src/main.cpp:62`, `223`). |
| **Sprite (8 bpp)** | The off-screen buffer that each frame is drawn into before it is pushed (`src/main.cpp:17`, `188`). |
| **Time warp** | Speed multiplier from real time up to 1 year/s, indexed by `warpIdx` (`src/main.cpp:139-141`). |
| **Touch press edge** | A change from not-touched to touched; the only moment touch acts (`src/main.cpp:198`). |
| **VSPI / HSPI** | The two ESP32 SPI buses. Touch uses VSPI (`src/main.cpp:27`); the display uses HSPI (`platformio.ini:38`). |
| **XPT2046** | The resistive touch controller on the CYD (`src/main.cpp:28`). |
| **ILI9341** | The 240x320 TFT controller on the CYD (`platformio.ini:18`). |
| **PSRAM** | External RAM. The WROOM-32 module has none, which limits the sprite to 8 bpp (`README.md:24`). |
