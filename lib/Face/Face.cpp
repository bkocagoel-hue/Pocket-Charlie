#include "Face.h"

#include <Arduino.h>
#include <math.h>

#include "PcConfig.h"

namespace pc {
namespace {

// ---------------------------------------------------------------------------
//  Charakter-Parameter (Stellschrauben fuer Charlies "Wesen").
//  Diese Werte definieren, wie lebendig/ruhig Charlie wirkt. Sie werden an die
//  "Grundpersoenlichkeit" (Notion) angepasst, sobald diese vorliegt.
// ---------------------------------------------------------------------------

// Gesichts-Geometrie (Display-Koordinaten)
constexpr std::int16_t kEyeW       = 60;
constexpr std::int16_t kEyeH       = 78;   // volle Augenhoehe (offen)
constexpr std::int16_t kMinEyeH    = 8;    // Augenhoehe (geschlossen -> Schlitz)
constexpr std::int16_t kEyeCornerR = 18;
constexpr std::int16_t kEyeGap     = 112;  // Abstand der Augenmittelpunkte
constexpr std::int16_t kEyeCenterY = 100;
constexpr std::int16_t kPupilR     = 15;

constexpr std::int16_t kMouthW       = 74;
constexpr std::int16_t kMouthH       = 10;
constexpr std::int16_t kMouthCornerR = 5;
constexpr std::int16_t kMouthY       = 172;

// Blick (Gaze)
constexpr std::int16_t kMaxGazeX   = 16;
constexpr std::int16_t kMaxGazeY   = 9;
constexpr float        kGazeSmooth = 0.18f;  // 0..1: hoeher = schnellere Augen

// Blinzeln
constexpr std::uint32_t kBlinkCloseMs  = 90;
constexpr std::uint32_t kBlinkOpenMs   = 130;
constexpr std::uint32_t kBlinkMinGapMs = 2500;
constexpr std::uint32_t kBlinkMaxGapMs = 6000;

// Blickwechsel-Intervall
constexpr std::uint32_t kGazeMinGapMs = 1400;
constexpr std::uint32_t kGazeMaxGapMs = 3800;
constexpr int           kGazeCenterPercent = 35;  // Wahrscheinlichkeit "Mitte"

// "Atmen"
constexpr float kBreathAmpPx = 2.5f;
constexpr float kBreathHz    = 0.31f;  // ~1 Atemzug alle ~3,2 s
constexpr float kTwoPi       = 6.28318530718f;

inline float clampf(float v, float lo, float hi) {
  return v < lo ? lo : (v > hi ? hi : v);
}

inline std::int16_t iround(float v) {
  return static_cast<std::int16_t>(v < 0 ? v - 0.5f : v + 0.5f);
}

}  // namespace

void Face::begin(std::int16_t screenW, std::int16_t screenH) {
  screenW_ = screenW;
  screenH_ = screenH;

  randomSeed(micros());

  // Vollbild-Canvas im PSRAM -> flicker-freies Rendern des ganzen Gesichts.
  // (320x240x16bit ~= 150 KB; passt bequem ins PSRAM des CoreS3.)
  canvas_.setColorDepth(16);
  canvas_.setPsram(true);
  ready_ = (canvas_.createSprite(screenW_, screenH_) != nullptr);
  if (!ready_) {
    Serial.println("[Face] FEHLER: Canvas konnte nicht angelegt werden (RAM?).");
  }

  const std::uint32_t now = millis();
  scheduleNextBlink(now);
  scheduleNextGaze(now);
}

void Face::scheduleNextBlink(std::uint32_t nowMs) {
  nextBlinkAt_ = nowMs + random(kBlinkMinGapMs, kBlinkMaxGapMs);
}

void Face::scheduleNextGaze(std::uint32_t nowMs) {
  nextGazeChangeAt_ = nowMs + random(kGazeMinGapMs, kGazeMaxGapMs);
  // Meist leichte Auslenkung, gelegentlich Blick zurueck zur Mitte.
  if (random(100) < kGazeCenterPercent) {
    gazeTargetX_ = 0.0f;
    gazeTargetY_ = 0.0f;
  } else {
    gazeTargetX_ = static_cast<float>(random(-kMaxGazeX, kMaxGazeX + 1));
    gazeTargetY_ = static_cast<float>(random(-kMaxGazeY, kMaxGazeY + 1));
  }
}

void Face::blinkNow() {
  if (!blinking_) {
    blinking_ = true;
    blinkStartedAt_ = millis();
  }
}

void Face::lookAt(std::int16_t x, std::int16_t y) {
  const float cx = screenW_ * 0.5f;
  const float cy = screenH_ * 0.5f;
  gazeTargetX_ = clampf((x - cx) / cx * kMaxGazeX, -kMaxGazeX, kMaxGazeX);
  gazeTargetY_ = clampf((y - cy) / cy * kMaxGazeY, -kMaxGazeY, kMaxGazeY);
  // Blick kurz halten, bevor die Idle-Logik wieder uebernimmt.
  nextGazeChangeAt_ = millis() + 1500;
}

void Face::update(std::uint32_t nowMs) {
  // --- Blinzeln ---
  if (blinking_) {
    const std::uint32_t t = nowMs - blinkStartedAt_;
    if (t < kBlinkCloseMs) {
      eyeOpen_ = 1.0f - static_cast<float>(t) / kBlinkCloseMs;  // 1 -> 0
    } else if (t < kBlinkCloseMs + kBlinkOpenMs) {
      eyeOpen_ = static_cast<float>(t - kBlinkCloseMs) / kBlinkOpenMs;  // 0 -> 1
    } else {
      eyeOpen_ = 1.0f;
      blinking_ = false;
      scheduleNextBlink(nowMs);
    }
  } else {
    eyeOpen_ = 1.0f;
    if (nowMs >= nextBlinkAt_) blinkNow();
  }

  // --- Blickrichtung: Ziel neu wuerfeln + sanft nachfuehren ---
  if (nowMs >= nextGazeChangeAt_) scheduleNextGaze(nowMs);
  gazeX_ += (gazeTargetX_ - gazeX_) * kGazeSmooth;
  gazeY_ += (gazeTargetY_ - gazeY_) * kGazeSmooth;

  // --- "Atmen": leichtes vertikales Wippen ---
  bobY_ = sinf(static_cast<float>(nowMs) * 0.001f * kBreathHz * kTwoPi) *
          kBreathAmpPx;
}

void Face::drawEye(std::int16_t cx, std::int16_t cy, float openAmount,
                   std::int16_t gazeX, std::int16_t gazeY) {
  const std::int16_t h =
      static_cast<std::int16_t>(kMinEyeH + (kEyeH - kMinEyeH) * openAmount);
  std::int16_t r = kEyeCornerR;
  if (r > h / 2) r = h / 2;
  if (r > kEyeW / 2) r = kEyeW / 2;

  // Augapfel (leuchtendes Cyan).
  canvas_.fillRoundRect(cx - kEyeW / 2, cy - h / 2, kEyeW, h, r,
                        config::kColorAccent);

  // Pupille + Glanzpunkt nur bei ausreichend geoeffnetem Auge.
  if (openAmount > 0.55f) {
    const std::int16_t px = cx + gazeX;
    const std::int16_t py = cy + gazeY;
    canvas_.fillCircle(px, py, kPupilR, config::kColorBackground);  // Pupille
    canvas_.fillCircle(px - 5, py - 5, 3, config::kColorText);      // Glanz
  }
}

void Face::render() {
  if (!ready_) return;

  canvas_.fillScreen(config::kColorBackground);

  const std::int16_t cx = screenW_ / 2;
  const std::int16_t eyeY = static_cast<std::int16_t>(kEyeCenterY + bobY_);
  const std::int16_t gx = iround(gazeX_);
  const std::int16_t gy = iround(gazeY_);

  drawEye(cx - kEyeGap / 2, eyeY, eyeOpen_, gx, gy);
  drawEye(cx + kEyeGap / 2, eyeY, eyeOpen_, gx, gy);

  // Mund (in Sprint 1 noch statisch - Mimik kommt mit den Emotionen).
  const std::int16_t my = static_cast<std::int16_t>(kMouthY + bobY_);
  canvas_.fillRoundRect(cx - kMouthW / 2, my, kMouthW, kMouthH, kMouthCornerR,
                        config::kColorAccent);

  // Alles in einem Rutsch auf das Display schieben (flicker-frei).
  canvas_.pushSprite(&M5.Display, 0, 0);
}

}  // namespace pc
