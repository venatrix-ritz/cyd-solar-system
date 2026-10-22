# 02 - Features, controls, inputs and outputs

## Features

| Feature | Where | Notes |
|---|---|---|
| Eight planets on screen orbits | `src/main.cpp:37-47`, `124-134` | Radii are chosen for legibility, not AU scale (`src/main.cpp:31`, `README.md:25`). |
| Mean-longitude positions | `src/main.cpp:50-53`, `233-236` | See [design 02](../design-spec/02-algorithms-timing.md) for the math and the mean-vs-true caveat. |
| Saturn ring | `src/main.cpp:44`, `127-131` | Drawn as three concentric ellipses. |
| Sun | `src/main.cpp:116-123` | Six concentric filled circles at the centre. |
| Starfield | `src/main.cpp:55-57`, `98-111` | 80 stars, each brightness-modulated over time. |
| Orbit rings | `src/main.cpp:112-115` | Drawn only when the UI is visible (`src/main.cpp:231`). |
| Simulated date label | `src/main.cpp:151-158` | Format `YYYY-MM-DD`, top-left, font 2. |
| Speed label | `src/main.cpp:163-165` | Shows the current warp name, centred in the bottom row. |
| UI-hint label | `src/main.cpp:167-169` | "UI" text in the top-right corner. |

## Touch controls

Touch is read from an XPT2046 controller (`src/main.cpp:28`, `197`). The firmware acts only on the **press edge** (`src/main.cpp:198`, `214`). Coordinates are mapped linearly from the raw 0..4095 range to screen pixels with no calibration constants (`src/main.cpp:200-201`).

Hit zones at the 320x240 landscape size (`src/main.cpp:137-141`, `180-181`, `204-212`; the size is read at runtime from `tft.width()`/`tft.height()`, `src/main.cpp:179`):

| Control | Condition in code | Effective pixel zone (320x240) | Effect |
|---|---|---|---|
| Show / hide UI | `px > SCR_W*0.72` and `py < SCR_H*0.28` (`src/main.cpp:204`) | x > 230, y < 67 | Toggles `uiVisible` (`src/main.cpp:205`). Works in both UI states. |
| RESET TO TODAY | UI visible and `rstY-6 < py < rstY+rstH+6` and `rstX < px < rstX+rstW` (`src/main.cpp:206-207`) | x 95..225, y 188..220 (derived) | Sets `simSeconds = 0` and `warpIdx = 0` (`src/main.cpp:208`). |
| Slower (`-`) | UI visible and `spdY-6 < py < spdY+spH+6` and `px < minX+spW+8` (`src/main.cpp:209-210`) | x < 42, y 211..243 (derived) | `warpIdx` decreases, floor 0. |
| Faster (`+`) | UI visible and speed row and `px > plsX-8` (`src/main.cpp:211`) | x > 278, y 211..243 (derived) | `warpIdx` increases, cap 5. |

The `-` and `+` buttons are drawn at x 4..34 and x 286..316 (`src/main.cpp:181`, `138`, `161-162`). The hit zones are wider than the drawn buttons (`+8` and `-8` margins, `src/main.cpp:210-211`).

No code path draws a press highlight or any other touch feedback (verified by reading the full `handleTouch()`, `src/main.cpp:196-215`).

## Time warp

| Index | Label (`WLAB`) | Sim seconds per real second (`WARP`) | Source |
|---|---|---|---|
| 0 | real time | 1 | `src/main.cpp:140-141` |
| 1 | 1 hr/s | 3600 | same |
| 2 | 1 day/s | 86400 | same |
| 3 | 1 week/s | 604800 | same |
| 4 | 1 month/s | 2629800 (30.4375 d) | same |
| 5 | 1 year/s | 31557600 (365.25 d) | same |

The index starts at 0 at boot (`src/main.cpp:139`).

## Inputs

| Input | Source | Used for |
|---|---|---|
| Build date | Compile-time `__DATE__` (`src/main.cpp:87`) | Epoch offset from J2000 (`src/main.cpp:90`). |
| Touch | XPT2046 on VSPI (`src/main.cpp:22-28`, `184-185`) | Controls above. |
| Elapsed time | `millis()` (`src/main.cpp:193`, `218`) | Frame `dt`, sim clock, star twinkle. |
| Random seed | `esp_random()` (`src/main.cpp:174`) | Starfield placement and phase. |
| Constant tables | `src/main.cpp:37-53`, `140-141` | Planet geometry, mean longitudes, mean motions, warp steps. |

No other inputs are read in the file. In particular, no GPIO is read as a button, and the BOOT button is not referenced in `src/main.cpp`.

## Outputs

| Output | Where | Content |
|---|---|---|
| TFT display | `fb.pushSprite(0, 0)` (`src/main.cpp:239`), backlight forced on (`src/main.cpp:183`) | One full-screen 8-bpp frame per loop. |
| Serial, 115200 baud | `Serial.begin(115200)` (`src/main.cpp:173`) | At startup only: `Build date YYYY-MM-DD  J2000+N.N d` (`src/main.cpp:190`), and `Sprite alloc FAILED` if the sprite cannot be created (`src/main.cpp:189`). |

The serial monitor speed in `platformio.ini` (`platformio.ini:7`) matches the code (`src/main.cpp:173`). No code path prints per frame.
