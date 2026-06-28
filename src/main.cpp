// Solar System orrery for ESP32-2432S028 (CYD2USB).
// Real heliocentric longitudes from J2000 mean elements + mean daily motion (Kepler-exact ratios).
// No RTC/internet: "today" = the firmware build date (__DATE__), so reflashing re-syncs to the day.
// Flicker-free 8bpp sprite. Driver MUST be ILI9341_2_DRIVER (platformio.ini).
//
// Touch:
//   - top-right corner  -> show / hide UI (orbit rings, date, button)
//   - "RESET TO TODAY"  -> snap planets to real positions for the build date, real time
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>
#include <math.h>
#include <string.h>

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite fb = TFT_eSprite(&tft);

int SCR_W, SCR_H, CX, CY;

// ---- Touch ----
#define T_CS 33
#define T_IRQ 36
#define T_CLK 25
#define T_MOSI 32
#define T_MISO 39
SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(T_CS, T_IRQ);

struct Planet {
  int   orbit;     // screen orbit radius (px) — legibility, NOT to AU scale
  int   size;
  uint16_t col, hi;
  bool  ring;
};

#define N 8
Planet planets[N] = {
  {  26, 2, TFT_LIGHTGREY, TFT_WHITE, false }, // Mercury
  {  37, 3, 0xFDA0,        0xFEE0,    false }, // Venus
  {  50, 3, 0x2D7F,        0x5DFF,    false }, // Earth
  {  63, 2, 0xF2A0,        0xFC00,    false }, // Mars
  {  84, 6, 0xDDC8,        0xFEF0,    false }, // Jupiter
  { 100, 5, 0xF7B0,        0xFFD0,    true  }, // Saturn
  { 112, 4, 0x9FFF,        0xCFFF,    false }, // Uranus
  { 120, 4, 0x33DF,        0x6CFF,    false }, // Neptune
};

// Mean longitude at J2000 (deg) and mean daily motion (deg/day = 360 / sidereal period).
const float L0[N] = { 252.25084f, 181.97973f, 100.46435f, 355.45332f,
                       34.40438f,  49.94432f, 313.23218f, 304.88003f };
const float NDAY[N] = { 4.0923351f, 1.6021303f, 0.9856091f, 0.5240208f,
                        0.0830912f, 0.0334597f, 0.0117308f, 0.0059811f };

#define NSTARS 80
int16_t  starX[NSTARS], starY[NSTARS];
uint8_t  starB[NSTARS], starPh[NSTARS];

bool     uiVisible = true;
uint32_t lastMs = 0;
float    tsec = 0;
double   simSeconds = 0;       // seconds elapsed since boot (real time)
double   buildEpochDays = 0;   // days from J2000 to the build date
int      bY, bM, bD;           // parsed build date
bool     wasTouched = false;

#define ORBIT_COL 0x2104
#define SCALE_X 1.17f      // widen orbits horizontally (~+20px at the rim)
#define SCALE_Y 0.95f      // taller orbits vertically (~+40px at the rim)
#define DEG2RAD 0.01745329f

// ---- date helpers ----
long gregToJDN(int y, int m, int d) {
  long a = (14 - m) / 12, yy = y + 4800 - a, mm = m + 12 * a - 3;
  return d + (153 * mm + 2) / 5 + 365 * yy + yy / 4 - yy / 100 + yy / 400 - 32045;
}
void jdnToGreg(long jdn, int &y, int &m, int &d) {
  long a = jdn + 32044, b = (4 * a + 3) / 146097, c = a - (146097 * b) / 4;
  long dd = (4 * c + 3) / 1461, e = c - (1461 * dd) / 4, mm = (5 * e + 2) / 153;
  d = e - (153 * mm + 2) / 5 + 1;
  m = mm + 3 - 12 * (mm / 10);
  y = 100 * b + dd - 4800 + mm / 10;
}
void parseBuildDate() {
  const char *mn = "JanFebMarAprMayJunJulAugSepOctNovDec";
  char mon[4] = {0};
  sscanf(__DATE__, "%3s %d %d", mon, &bD, &bY);
  const char *p = strstr(mn, mon);
  bM = p ? (int)((p - mn) / 3) + 1 : 1;
  buildEpochDays = (double)(gregToJDN(bY, bM, bD) - 2451545L); // JDN 2451545 = 2000-01-01
}

uint16_t dim565(uint16_t c, uint8_t num, uint8_t den) {
  uint8_t r = (c >> 11) & 0x1F, g = (c >> 5) & 0x3F, b = c & 0x1F;
  return ((r * num / den) << 11) | ((g * num / den) << 5) | (b * num / den);
}

void seedStars() {
  for (int i = 0; i < NSTARS; i++) {
    starX[i] = random(SCR_W); starY[i] = random(SCR_H);
    starB[i] = 70 + random(150); starPh[i] = random(255);
    if (abs(starX[i] - CX) < 20 && abs(starY[i] - CY) < 20) starY[i] = (starY[i] + 60) % SCR_H;
  }
}
void drawStars() {
  for (int i = 0; i < NSTARS; i++) {
    float tw = 0.6f + 0.4f * sinf(tsec * 2.0f + starPh[i]);
    uint8_t v = (uint8_t)(starB[i] * tw);
    fb.drawPixel(starX[i], starY[i], fb.color565(v, v, v));
  }
}
void drawOrbits() {
  for (int i = 0; i < N; i++)
    fb.drawEllipse(CX, CY, (int)(planets[i].orbit * SCALE_X), (int)(planets[i].orbit * SCALE_Y), ORBIT_COL);
}
void drawSun() {
  fb.fillCircle(CX, CY, 22, 0x4140);
  fb.fillCircle(CX, CY, 18, 0x8200);
  fb.fillCircle(CX, CY, 14, 0xFB00);
  fb.fillCircle(CX, CY, 10, 0xFE40);
  fb.fillCircle(CX, CY,  6, 0xFFE0);
  fb.fillCircle(CX, CY,  3, 0xFFFF);
}
void drawPlanet(Planet &pl, float angle) {
  int x = CX + (int)(cosf(angle) * pl.orbit * SCALE_X);
  int y = CY + (int)(sinf(angle) * pl.orbit * SCALE_Y);
  if (pl.ring) {
    fb.drawEllipse(x, y, pl.size + 6, (pl.size + 6) / 2, dim565(0xC618, 1, 2));
    fb.drawEllipse(x, y, pl.size + 5, (pl.size + 5) / 2, 0xC618);
    fb.drawEllipse(x, y, pl.size + 4, (pl.size + 4) / 2, 0x9CD3);
  }
  fb.fillCircle(x, y, pl.size, pl.col);
  fb.fillCircle(x - pl.size / 3, y - pl.size / 3, max(1, pl.size / 2), pl.hi);
}

// UI geometry (set in setup)
int rstX, rstY, rstW = 130, rstH = 20;     // reset button
int minX, plsX, spdY, spW = 30, spH = 20;  // speed -/+ buttons
int warpIdx = 0;
const double WARP[6] = { 1.0, 3600.0, 86400.0, 604800.0, 2629800.0, 31557600.0 };
const char  *WLAB[6] = { "real time", "1 hr/s", "1 day/s", "1 week/s", "1 month/s", "1 year/s" };

void drawBtn(int x, int y, int w, int h, const char *label) {
  fb.fillRoundRect(x, y, w, h, 4, 0x20E4);
  fb.drawRoundRect(x, y, w, h, 4, 0x6B6D);
  fb.setTextColor(0xCE59, 0x20E4);
  fb.setTextDatum(MC_DATUM);
  fb.drawString(label, x + w / 2, y + h / 2, 2);
}

void drawUI() {
  // current sim date
  long jdn = lround(2451545.0 + buildEpochDays + simSeconds / 86400.0);
  int y, m, d; jdnToGreg(jdn, y, m, d);
  char buf[24]; snprintf(buf, sizeof(buf), "%04d-%02d-%02d", y, m, d);
  fb.setTextColor(0x8C71, TFT_BLACK);
  fb.setTextDatum(TL_DATUM);
  fb.drawString(buf, 4, 4, 2);

  drawBtn(rstX, rstY, rstW, rstH, "RESET TO TODAY");      // reset
  drawBtn(minX, spdY, spW, spH, "-");                     // slower
  drawBtn(plsX, spdY, spW, spH, "+");                     // faster
  fb.setTextColor(0xCE59, TFT_BLACK);                     // speed label
  fb.setTextDatum(MC_DATUM);
  fb.drawString(WLAB[warpIdx], CX, spdY + spH / 2, 2);

  fb.setTextColor(0x4208, TFT_BLACK);                     // toggle hint
  fb.setTextDatum(TR_DATUM);
  fb.drawString("UI", SCR_W - 4, 4, 1);
}

void setup() {
  Serial.begin(115200);
  randomSeed(esp_random());
  parseBuildDate();

  tft.init();
  tft.setRotation(1);
  SCR_W = tft.width(); SCR_H = tft.height(); CX = SCR_W / 2; CY = SCR_H / 2;
  rstX = CX - rstW / 2; rstY = SCR_H - rstH - 26;          // reset row
  minX = 4; plsX = SCR_W - 4 - spW; spdY = SCR_H - spH - 3; // speed row

  pinMode(TFT_BL, OUTPUT); digitalWrite(TFT_BL, HIGH);
  touchSPI.begin(T_CLK, T_MISO, T_MOSI, T_CS);
  ts.begin(touchSPI); ts.setRotation(1);

  tft.fillScreen(TFT_BLACK);
  fb.setColorDepth(8);
  if (!fb.createSprite(SCR_W, SCR_H)) Serial.println("Sprite alloc FAILED");
  Serial.printf("Build date %04d-%02d-%02d  J2000+%.1f d\n", bY, bM, bD, buildEpochDays);

  seedStars();
  lastMs = millis();
}

void handleTouch() {
  bool now = ts.touched();
  if (now && !wasTouched) {                 // act on press edge only
    TS_Point p = ts.getPoint();
    int px = constrain(map(p.x, 0, 4095, 0, SCR_W), 0, SCR_W);
    int py = constrain(map(p.y, 0, 4095, 0, SCR_H), 0, SCR_H);

    // top-right corner -> toggle UI
    if (px > SCR_W * 0.72f && py < SCR_H * 0.28f) {
      uiVisible = !uiVisible;
    } else if (uiVisible && py > rstY - 6 && py < rstY + rstH + 6 &&
               px > rstX && px < rstX + rstW) {
      simSeconds = 0; warpIdx = 0;           // reset to build date, real time
    } else if (uiVisible && py > spdY - 6 && py < spdY + spH + 6) {
      if (px < minX + spW + 8) warpIdx = max(0, warpIdx - 1);          // slower
      else if (px > plsX - 8)  warpIdx = min(5, warpIdx + 1);          // faster
    }
  }
  wasTouched = now;
}

void loop() {
  uint32_t nowMs = millis();
  float dt = (nowMs - lastMs) / 1000.0f;
  lastMs = nowMs;
  if (dt > 0.1f) dt = 0.1f;
  tsec += dt;
  simSeconds += (double)dt * WARP[warpIdx];  // time warp

  handleTouch();

  double D = buildEpochDays + simSeconds / 86400.0;   // days since J2000

  fb.fillSprite(TFT_BLACK);
  drawStars();
  if (uiVisible) drawOrbits();
  drawSun();
  for (int i = 0; i < N; i++) {
    float a = (float)((L0[i] + NDAY[i] * D) * DEG2RAD);
    drawPlanet(planets[i], a);
  }
  if (uiVisible) drawUI();

  fb.pushSprite(0, 0);
}
