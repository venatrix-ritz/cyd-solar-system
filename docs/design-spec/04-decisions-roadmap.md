# 04 - Design decisions and roadmap

Each decision says where its reason comes from. If the repo does not state a reason, the entry says so and gives no cause.

## Decisions

### D1 - Use the compile date as the clock source
- **Choice:** derive "today" from `__DATE__` at compile time (`src/main.cpp:84-91`).
- **Reason (stated):** no RTC and no network (`src/main.cpp:3`, `README.md:16`).
- **Consequence:** the displayed day is the day of compilation. The time of day is not known (`src/main.cpp:90`, `227`). Reflashing is the only way to re-sync.
- **Alternatives:** none recorded in the repo. An RTC or network time would need hardware or code this project does not have.

### D2 - Composite each frame in an 8-bit sprite and push it once
- **Choice:** full-screen 8-bpp `TFT_eSprite` (`src/main.cpp:17`, `188-189`) and one `pushSprite(0, 0)` per loop (`src/main.cpp:239`).
- **Reason (stated):** flicker-free output (`src/main.cpp:4`, `README.md:5`); 8 bpp because the WROOM-32 has no PSRAM and 16 bpp "won't allocate" (`README.md:24`).
- **Consequence:** colours go through an 8-bit palette (`color565` is used for stars, `src/main.cpp:109`). Colour fidelity at 8 bpp is **unverified** on hardware.

### D3 - Use mean longitude with per-planet constants, not a full orbital element set
- **Choice:** `L0 + NDAY x D` with 16 constants and no eccentricity or perturbation terms (`src/main.cpp:50-53`, `234`).
- **Reason:** not written in the repo. The README calls the speeds "physically real" (`README.md:25`), and `NDAY` is defined as 360 over the sidereal period (`src/main.cpp:49`).
- **Consequence:** positions are approximate, and the README's "actual heliocentric longitudes" wording (`README.md:5`) overstates the match (see [application spec 04](../application-spec/04-requirements-testing-limitations.md#code-and-documentation-discrepancies-verified-by-reading-the-source)).
- **Rejected alternatives:** none documented.

### D4 - Scale orbit radii for legibility, not to AU
- **Choice:** screen radii 26 to 120 px, with `SCALE_X` and `SCALE_Y` (`src/main.cpp:31`, `37-47`, `68-69`).
- **Reason (stated):** "legibility, NOT to AU scale" (`src/main.cpp:31`); the README says the same (`README.md:25`).
- **Consequence:** the orbits are not to scale. Only the angular rates are meant to be real.

### D5 - Scale X and Y differently
- **Choice:** `SCALE_X = 1.17`, `SCALE_Y = 0.95` (`src/main.cpp:68-69`).
- **Reason (stated, partly):** the X comment says "widen orbits horizontally (~+20px at the rim)", which matches 120 x 1.17 - 120 = 20.4 px (derived). The Y comment says "taller orbits vertically (~+40px at the rim)", which does not match a value of 0.95 (see [application spec 04](../application-spec/04-requirements-testing-limitations.md#code-and-documentation-discrepancies-verified-by-reading-the-source)). The actual reason for the Y value is **unverified**.

### D6 - Act only on the touch press edge
- **Choice:** handle a touch once, on the transition from not-touched to touched (`src/main.cpp:197-198`, `214`).
- **Reason:** not stated in the repo. **Unverified.** A plausible effect is that holding a button does not repeat its action, but that has not been run or confirmed.

### D7 - Clamp frame time to 0.1 s
- **Choice:** `if (dt > 0.1f) dt = 0.1f` (`src/main.cpp:221`).
- **Reason:** not stated. **Unverified.** A plausible guess is that it stops a large jump after a stall; that is unconfirmed.
- **Consequence:** simulated time falls behind real time when a frame takes more than 100 ms (see [02](02-algorithms-timing.md#simulated-clock-and-time-warp)).

### D8 - Use a separate SPI bus for touch
- **Choice:** `SPIClass touchSPI(VSPI)` for the XPT2046, and HSPI for the display (`src/main.cpp:27`; `platformio.ini:38`).
- **Reason:** the pins differ (`src/main.cpp:22-26` against `platformio.ini:21-24`). Whether a shared bus would fail is not stated and **unverified**.

### D9 - Single source file, global state
- **Choice:** all code in `src/main.cpp`, with global state (`src/main.cpp:19-65`).
- **Reason:** not stated. The repo is small (240 lines) and has one target.
- **Consequence:** no module boundaries, so tests of the date or orbit maths would need the file split first.

### D10 - Star field on real time
- **Choice:** star twinkle uses `tsec`, which is not warped (`src/main.cpp:222`, `107`).
- **Reason:** not stated. Its effect (stars do not speed up with warp) follows from the code.

## Roadmap

**The repo has no roadmap.** There is no ROADMAP, TODO, or issue file in the tracked files (`git ls-files` on `origin/main` lists `.gitignore`, `README.md`, `platformio.ini`, `src/main.cpp`). Nothing here is a plan from the owner.

Open items that follow from the discrepancies in this spec. These are not priorities and not a plan. Each is a candidate for a future change:
1. Correct the `simSeconds` comment at `src/main.cpp:62`.
2. Decide whether the Y-scale comment at `src/main.cpp:69` or the value `0.95` is the intended design. Then align them.
3. Decide whether README:5 should say "mean longitude" or the code should add the missing terms.
4. Pin `XPT2046_Touchscreen` to a tag or commit (`platformio.ini:14`), and the platform version (`platformio.ini:4`).
5. Decide what the firmware should do when the sprite cannot be allocated (`src/main.cpp:189`).
6. Remove or justify the `TFT_WIDTH`/`TFT_HEIGHT` defines (`platformio.ini:19-20`), after checking their effect on the build.

Items 1 to 3 are documentation or wording. Items 4 to 6 change the build or behaviour and would need on-device checking.
