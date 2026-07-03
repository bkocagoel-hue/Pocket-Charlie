#pragma once
// ============================================================================
//  Face - Charlies animiertes Gesicht (Augen + Mund) mit Idle-Animation
//
//  Verantwortung:
//  - Haelt den Animationszustand (Blinzeln, Blickrichtung, "Atmen").
//  - Rendert das Gesicht flicker-frei ueber einen Off-Screen-Puffer (M5Canvas
//    im PSRAM): erst komplett in den Puffer zeichnen, dann in EINEM Rutsch auf
//    das Display schieben. So gibt es kein Flackern durch Teil-Neuzeichnungen.
//
//  Trennung update()/render():
//  - update(now): schreibt NUR den Zustand fort (Logik, zeitbasiert).
//  - render():    zeichnet den aktuellen Zustand (keine Logik).
//  Das haelt die Animation deterministisch und spaeter testbar.
//
//  Charakter-Parametrisierung:
//  Blinzelrate, Blick-Energie usw. liegen als Konstanten in Face.cpp. Sobald
//  die "Grundpersoenlichkeit" (Notion) definiert ist, justieren wir dort die
//  Werte - ohne strukturelle Aenderungen.
// ============================================================================

#include <cstdint>

#include <M5Unified.h>  // fuer M5Canvas

namespace pc {

class Face {
 public:
  // Legt den Canvas an (Bildschirmgroesse). Voraussetzung: M5.begin() lief.
  void begin(std::int16_t screenW, std::int16_t screenH);

  // Animationszustand fortschreiben (now = millis()).
  void update(std::uint32_t nowMs);

  // Aktuellen Zustand auf das Display bringen.
  void render();

  // --- Interaktion (Sprint 1) ---
  void lookAt(std::int16_t x, std::int16_t y);  // Blick zu einem Punkt lenken
  void blinkNow();                              // sofortiges Blinzeln ausloesen

 private:
  void scheduleNextBlink(std::uint32_t nowMs);
  void scheduleNextGaze(std::uint32_t nowMs);
  void drawEye(std::int16_t cx, std::int16_t cy, float openAmount,
               std::int16_t gazeX, std::int16_t gazeY);

  M5Canvas canvas_;  // Off-Screen-Puffer (PSRAM)
  std::int16_t screenW_ = 0;
  std::int16_t screenH_ = 0;
  bool ready_ = false;

  // Blinzeln
  bool blinking_ = false;
  std::uint32_t blinkStartedAt_ = 0;
  std::uint32_t nextBlinkAt_ = 0;
  float eyeOpen_ = 1.0f;  // 0 = geschlossen, 1 = offen

  // Blickrichtung (Ziel + sanft nachgefuehrter Ist-Wert)
  float gazeX_ = 0.0f, gazeY_ = 0.0f;
  float gazeTargetX_ = 0.0f, gazeTargetY_ = 0.0f;
  std::uint32_t nextGazeChangeAt_ = 0;

  // "Atmen" (leichtes vertikales Wippen)
  float bobY_ = 0.0f;
};

}  // namespace pc
