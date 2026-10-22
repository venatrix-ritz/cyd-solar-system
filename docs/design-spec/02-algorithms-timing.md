# 02 - Algorithms and timing

Each section gives the code, its parameters, and the timing where it matters. Periods in the tables are derived from `NDAY` by arithmetic (`360 / NDAY`); they are not read from a source in the repo.

## Build date
`__DATE__` is parsed at runtime by `parseBuildDate()` (`src/main.cpp:84-91`).
- `sscanf(__DATE__, "%3s %d %d", mon, &bD, &bY)` (`src/main.cpp:87`). The return value is not checked. If the parse failed, the globals would keep stale values. The compiler always emits this format, so the failure path is **unverified** as reachable and untested.
- The month is found by `strstr` in the 36-character string `"JanFebMar...Dec"` (`src/main.cpp:85`, `88`). The index is `offset / 3 + 1` (`89`). If not found, the month defaults to 1 (`89`).
- `buildEpochDays = gregToJDN(bY, bM, bD) - 2451545` (`90`). The result is a whole number of days stored as a double.

## Gregorian to and from Julian Day Number
`gregToJDN` (`src/main.cpp:73-76`) uses an integer form: `a = (14 - m) / 12`, `y' = y + 4800 - a`, `m' = m + 12a - 3`, then `JDN = d + (153m' + 2)/5 + 365y' + y'/4 - y'/100 + y'/400 - 32045`.

`jdnToGreg` (`src/main.cpp:77-83`) reverses it with the blocks shown there. Whether both functions match a reference calendar for every date is **unverified** (no test or run was done). They are used on the build date (`gregToJDN`) and on the displayed date (`jdnToGreg`, `src/main.cpp:153-154`).

## Epoch, time offset, and the Julian Day convention
- J2000 is JDN 2451545 (`src/main.cpp:90`).
- The sim offset in days is `D = buildEpochDays + simSeconds / 86400` (`src/main.cpp:227`).
- `buildEpochDays` is an integer at boot, and the code adds no time of day (verified by reading `src/main.cpp:90` and `227`). In the standard Julian Day convention, an integer JD is 12:00 on that calendar day. Whether that is the intended behaviour is **unverified**. The README says the planets are at "the firmware build date" (`README.md:5`), which the code satisfies at the level of the day.

## Mean longitude and screen position

Constants (`src/main.cpp:50-53`, values as coded):

| Planet | `L0` (deg, at J2000) | `NDAY` (deg/day) | Period = 360/NDAY (days, derived) |
|---|---|---|---|
| Mercury | 252.25084 | 4.0923351 | 87.97 |
| Venus | 181.97973 | 1.6021303 | 224.70 |
| Earth | 100.46435 | 0.9856091 | 365.26 |
| Mars | 355.45332 | 0.5240208 | 686.98 |
| Jupiter | 34.40438 | 0.0830912 | 4332.6 |
| Saturn | 49.94432 | 0.0334597 | about 10759 |
| Uranus | 313.23218 | 0.0117308 | about 30688 |
| Neptune | 304.88003 | 0.0059811 | about 60190 |

The code comment says `NDAY` is "360 / sidereal period" (`src/main.cpp:49`). The periods above follow from the constants by arithmetic. The source of `L0` and `NDAY` is not cited in the repo, and checking them against an ephemeris was not done (**unverified**).

Per frame, for each planet `i` (`src/main.cpp:233-236`):
- Angle, in radians: `a = (float)((L0[i] + NDAY[i] * D) * DEG2RAD)` (`src/main.cpp:234`). `DEG2RAD = 0.01745329f` (`src/main.cpp:70`). The angle is **not wrapped** to 0-360 degrees, so it grows with time. The effect of the float cast on precision after long warped runs is **unverified**.
- Screen position (`src/main.cpp:125-126`): `x = CX + (int)(cos(a) * orbit * SCALE_X)`, `y = CY + (int)(sin(a) * orbit * SCALE_Y)`, with `SCALE_X = 1.17` and `SCALE_Y = 0.95` (`src/main.cpp:68-69`).
- The position is the parametric point on the scaled ellipse at angle `a`, not a polar angle on a circle. That follows from the formula.
- The `(int)` cast truncates toward zero, so the rounding differs for negative offsets (a one-pixel effect).
- Direction: screen y increases downward, so as `a` increases a planet moves from the +x side toward the +y side. On screen that is clockwise. Whether that orientation is intended is **unverified**.
- Mean longitude is used in place of true longitude. The code has no eccentricity or equation-of-centre term (`src/main.cpp:234`). The size of the resulting angular error for each planet is **unverified**. General astronomy knowledge (not checked in this repo) says Mercury has the largest orbital eccentricity of the eight, so its error is probably the largest. That has not been computed (**unverified**).

Derived geometry at 320x240 (`CX = 160`, `CY = 120`, from `src/main.cpp:179`; Neptune orbit 120 from `src/main.cpp:46`; scales from `src/main.cpp:68-69`): the Neptune orbit reaches about 140 px horizontally (x 20 to 300) and about 114 px vertically (y 6 to 234).

## Simulated clock and time warp

Per frame (`src/main.cpp:218-223`):
1. `nowMs = millis()`, `dt = (nowMs - lastMs) / 1000.0f` (`218-219`). Both values are `uint32_t`, so the subtraction stays correct across a single `millis()` wrap (the wrap is every 2^32 ms, about 49.7 days). `lastMs = nowMs` (`220`).
2. `if (dt > 0.1f) dt = 0.1f` (`221`). The clamp is the only bound. `dt` can be 0 if two loops run within one millisecond.
3. `tsec += dt` (`222`). This is real time, used only by the starfield (`107`).
4. `simSeconds += (double)dt * WARP[warpIdx]` (`223`).

Warp steps (`src/main.cpp:140`): 1, 3600, 86400, 604800, 2629800, and 31557600 sim seconds per real second. The index is clamped to 0..5 (`210-211`).

Derived rates:
- At 1 year/s, one real second is 365.25 sim days. Mercury advances about 4.0923351 x 365.25, which is about 1494.7 deg per real second (about 4.15 revolutions). Neptune advances about 2.18 deg per real second.
- Reset sets `simSeconds = 0` and `warpIdx = 0` (`src/main.cpp:208`), so `D` returns to `buildEpochDays`.
- Because `simSeconds` is a double (`src/main.cpp:62`), the sim clock keeps precision over long runs far better than `tsec` does as a float. The effect on the on-screen result is **unverified**.

Frame-time coupling: if a frame takes longer than 0.1 s, `dt` is clamped and the sim clock runs slower than real time. Frame time is not measured in the repo, so how often this happens is **unverified**.

## Date label
`jdn = lround(2451545.0 + buildEpochDays + simSeconds / 86400.0)` (`src/main.cpp:153`). `jdnToGreg` converts it, and the label is `"%04d-%02d-%02d"` (`155`), drawn at top-left (`157-158`).

Because `lround` rounds to the nearest integer, the label changes when the sim offset reaches +0.5 day, not at a 00:00 boundary. Derived timings: at 1 hr/s the first change comes about 12 real seconds after start or reset, then one day every 24 real seconds. At real time, the label does not change for about the first 12 hours of sim time.

## Starfield
- Seeding (`src/main.cpp:98-104`): `x` in `[0, SCR_W)`, `y` in `[0, SCR_H)`, brightness `starB = 70 + random(150)` (70 to 219), and phase `starPh = random(255)` (0 to 254). A star inside the box of +/-20 px around the centre is moved down by 60 px, modulo the height (`102`).
- Twinkle (`src/main.cpp:107-109`): `tw = 0.6 + 0.4 * sin(2 * tsec + phase)`, range 0.6 to 1.0. `v = starB * tw` and the pixel colour is `color565(v, v, v)`. The sine's period is pi seconds, about 3.14 s (derived). The phase is used as radians without scaling, which is intended only as an arbitrary offset (derived).
- The starfield animates on real time, so it does not speed up with the time warp (`tsec` is not warped, `src/main.cpp:222`).

## Colour helpers
- `dim565(c, num, den)` (`src/main.cpp:93-96`) scales each channel (5-bit red and blue, 6-bit green) by `num/den` with integer division. The Saturn ring uses `dim565(0xC618, 1, 2)` (`src/main.cpp:128`), which halves each channel.

## Planet drawing (`src/main.cpp:124-134`)
- Saturn only: three concentric ellipses, with the outermost in a dimmed colour (`127-131`).
- Every planet: a filled circle of radius `size` (`132`), then a highlight dot at offset `-size/3` in x and y, radius `max(1, size/2)` (`133`).
- The Sun (`src/main.cpp:116-123`) is six concentric filled circles, radii 22, 18, 14, 10, 6, 3, drawn at the centre.

## Touch timing and mapping
- `ts.touched()` is read once per loop (`src/main.cpp:197`). `ts.getPoint()` is read only on the press edge (`198-199`), and `wasTouched` is updated every loop (`214`).
- Mapping (`src/main.cpp:200-201`): `px = constrain(map(p.x, 0, 4095, 0, SCR_W), 0, SCR_W)`, and the same for `py` with `SCR_H`. There is no axis swap and no calibration. Whether this matches the landscape orientation set by `ts.setRotation(1)` (`src/main.cpp:185`) is **unverified**.
- A touch is handled in the same loop it is read, before drawing (`src/main.cpp:225`), so its latency is at most one loop (derived from the loop structure).

## Timing summary

| Quantity | Value | Anchor |
|---|---|---|
| Blocking delay in loop | None (no `delay()` call in the file) | `src/main.cpp:217-240` (search) |
| Frame rate | Not measured. **Unverified.** | n/a |
| `dt` upper clamp | 0.1 s | `src/main.cpp:221` |
| `dt` lower clamp | None | `src/main.cpp:219-221` |
| Display updates per loop | One `pushSprite` | `src/main.cpp:239` |
| Touch sampling | Once per loop | `src/main.cpp:197` |
| Twinkle period | About 3.14 s | `src/main.cpp:107` (derived) |
| Date label change at 1 hr/s | Every 24 real s after the first at about 12 s | `src/main.cpp:153` (derived) |
