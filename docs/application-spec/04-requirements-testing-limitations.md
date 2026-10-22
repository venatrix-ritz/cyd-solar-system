# 04 - Requirements, testing and known limitations

## As-coded requirements
No requirements document exists in the repo. The list below states what the code does, so a change can be checked against it. Each item is anchored to the source.

| ID | Requirement (as coded) | Anchor |
|---|---|---|
| R1 | Planet positions depend only on the build date and time since boot. | `src/main.cpp:90`, `223`, `227`, `234` |
| R2 | The simulated clock starts at the build date and real time on every boot. | `src/main.cpp:62`, `63`, `90` |
| R3 | The display shows a full frame composited in one 8-bpp sprite and pushed once per loop. | `src/main.cpp:17`, `188-189`, `229-239` |
| R4 | Touch acts only on a new press, not while held. | `src/main.cpp:197-198`, `214` |
| R5 | The UI can be hidden and shown by tapping the top-right corner, in either state. | `src/main.cpp:204-205` |
| R6 | Time warp is limited to six steps, from real time to 1 year/s. | `src/main.cpp:139-141`, `210-211` |
| R7 | Reset sets the simulated offset to zero and the warp to real time. | `src/main.cpp:206-208` |
| R8 | The display runs in landscape rotation 1 with BGR colour order and inversion on. | `src/main.cpp:178`; `platformio.ini:19-20`, `29-30` |
| R9 | Touch uses its own SPI bus (VSPI), separate from the display bus (HSPI). | `src/main.cpp:27`; `platformio.ini:38` |
| R10 | Serial output is 115200 baud and prints only at startup. | `src/main.cpp:173`, `189-190`; `platformio.ini:7` |

## Testing

**None.** The repo contains no test files, no test framework in `lib_deps` (`platformio.ini:12-14`), and no CI configuration. The tracked files are `.gitignore`, `README.md`, `platformio.ini`, and `src/main.cpp` (`git ls-files` on `origin/main`, commit `dc87903`).

Verification in practice is on-device: flash and watch the display or serial log. The parent-folder CLAUDE.md, which is outside this repo, says the same (**context only**). No device or build step was run for this spec.

What a reader can check without hardware: the constant tables in `src/main.cpp:50-53` against the stated periods (see [design 02](../design-spec/02-algorithms-timing.md)), and the hit zones in `src/main.cpp:204-211` against the layout in `src/main.cpp:137-141`.

## Known limitations

### Model limitations (by design, as stated or coded)
- **Mean longitude, not true longitude.** The code adds `NDAY * D` to `L0` and nothing else (`src/main.cpp:234`). There is no eccentricity or equation of centre. README:5 says "actual heliocentric longitudes". That wording is stronger than the code. The size of the difference is **unverified**.
- **Orbit radii are not to scale** (`src/main.cpp:31`, `README.md:25`). Only the angular rates are meant to be real.
- **Sun is fixed at the centre**, with no motion, and there is no Moon or other body.
- **Date is the build date at boot**, with no time-of-day input (`src/main.cpp:90`, `227`). The date comes from the compile machine's `__DATE__` (`src/main.cpp:87`), not from the flash time.
- **No persistence.** Simulated time and warp are lost at power-off (`src/main.cpp:62`, `139`). Nothing is written to flash or NVS (no such calls in the file).
- **No real-time source.** Without a network or RTC, the simulated date only moves when the device runs.

### Code and documentation discrepancies (verified by reading the source)
1. **`simSeconds` comment is wrong.** `src/main.cpp:62` says "seconds elapsed since boot (real time)". The code adds `dt * WARP[warpIdx]` (`src/main.cpp:223`), so the value is warped simulated time, and reset zeroes it (`src/main.cpp:208`).
2. **SCALE_Y comment does not match the value.** `src/main.cpp:69` says "taller orbits vertically (~+40px at the rim)". The value is `0.95` (`src/main.cpp:69`), which is less than 1. At the Neptune rim (orbit 120, `src/main.cpp:46`) the vertical radius is 114 px instead of 120 (derived: 120 x 0.95). The rationale for the value is **unverified**.
3. **Header comment lists only two touch controls** (`src/main.cpp:6-8`). The code also handles the `-` and `+` buttons (`src/main.cpp:209-211`).
4. **README says "Kepler-exact period ratios"** (`README.md:5`). The file contains no Kepler computation. It stores constants `NDAY = 360 / period` (`src/main.cpp:52-53`). The source of those constants is not cited in the repo (**unverified**).

### Operational limitations (from the code)
5. **Sprite allocation failure is only logged.** If `createSprite` fails, the code prints `Sprite alloc FAILED` (`src/main.cpp:189`) and the loop keeps running. What the display does in that case is **unverified**.
6. **No touch calibration.** Raw 0..4095 values are mapped linearly to the screen (`src/main.cpp:200-201`). Accuracy at the edges is **unverified**.
7. **Simulation slows under load.** `dt` is clamped to 0.1 s (`src/main.cpp:221`). Any frame longer than 100 ms advances simulated time less than real time. Frame time is not measured, so how often this happens is **unverified**.
8. **Angle is not wrapped** to 0..360 degrees before the float cast (`src/main.cpp:234`). Precision after long warped runs is **unverified**.
9. **No press feedback.** Buttons are not redrawn on touch (`src/main.cpp:196-215`).
10. **Unused font flag.** `LOAD_FONT4` is set (`platformio.ini:33`) but font 4 is not used in `src/main.cpp`.

### Build dependencies
11. **Unpinned dependencies.** The platform is `espressif32` with no version (`platformio.ini:4`). `XPT2046_Touchscreen` comes from a git URL with no tag or commit (`platformio.ini:14`). `TFT_eSPI` is pinned to a range (`^2.5.43`, `platformio.ini:13`). Builds may differ over time (**unverified** in practice).
12. **Upload and monitor ports are pinned** to one value in `platformio.ini:9-10`, so the build assumes that port.
