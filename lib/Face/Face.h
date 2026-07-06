#pragma once
// ============================================================================
//  Face - Charlies animiertes Gesicht (Augen + Schnurrbart + Mund)
//
//  Sprint 2: emotionsabhaengiges, datengetriebenes Rendering. Eine Emotion
//  liefert einen Parametersatz (EmotionStyle, in Face.cpp), der weich
//  interpoliert wird (sanfte Uebergaenge). update() schreibt nur Zustand fort,
//  render() zeichnet - beides ueber millis(), ohne delay().
// ============================================================================

#include <cstdint>

#include <M5Unified.h>  // fuer M5Canvas

#include "Emotion.h"

namespace pc {

class Face {
 public:
  void begin(std::int16_t screenW, std::int16_t screenH);
  void update(std::uint32_t nowMs);
  void render();

  // --- Interaktion (Sprint 1) ---
  void lookAt(std::int16_t x, std::int16_t y);
  void blinkNow();

  // --- Emotion (Sprint 2) ---
  void setEmotion(Emotion e);  // Ziel-Emotion; Stil wird sanft nachgezogen

  // --- Microcopy / Gedankenblase (Sprint 3) ---
  void say(const char* text, std::uint32_t durMs);  // kurze Textblase anzeigen

 private:
  void scheduleNextBlink(std::uint32_t nowMs);
  void scheduleNextGaze(std::uint32_t nowMs);
  void drawEye(std::int16_t cx, std::int16_t cy, float openAmount,
               std::int16_t gazeX, std::int16_t gazeY);
  void drawMouth(std::int16_t cx, std::int16_t baseY, float curve);
  void drawMustache(std::int16_t cx, std::int16_t baseY, float lift);

  M5Canvas canvas_;
  std::int16_t screenW_ = 0;
  std::int16_t screenH_ = 0;
  bool ready_ = false;

  // Blinzeln
  bool blinking_ = false;
  std::uint32_t blinkStartedAt_ = 0;
  std::uint32_t nextBlinkAt_ = 0;
  float eyeOpen_ = 1.0f;

  // Blick (Gaze)
  float gazeX_ = 0.0f, gazeY_ = 0.0f;
  float gazeTargetX_ = 0.0f, gazeTargetY_ = 0.0f;
  std::uint32_t nextGazeChangeAt_ = 0;

  // Atmen
  float bobY_ = 0.0f;

  // Emotion + weich interpolierter Stil
  Emotion emotion_ = Emotion::Neutral;
  float sEyeOpen_ = 1.0f;
  float sMouthCurve_ = 0.0f;
  float sMustache_ = 0.0f;
  float sBlinkMul_ = 1.0f;
  std::uint8_t gazeMode_ = 0;

  // Microcopy / Gedankenblase (Sprint 3)
  const char* sayText_ = nullptr;
  std::uint32_t sayUntil_ = 0;
};

}  // namespace pc
