#pragma once
// ============================================================================
//  Face - Charlies animiertes Gesicht (Augen + Augenbrauen + Mund;
//  Schnurrbart optional/geparkt)
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
  void drawEyebrows(std::int16_t cx, std::int16_t eyeY, float lift, float tilt,
                    float asym, float vis);

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
  // Expression Pack v1 (Sprint 4 E4C): Variante wird beim Betreten einer
  // Emotion gewaehlt; Onset = kurzer Brauen-Akzent beim Wechsel; Micro =
  // seltene kleine Idle-Regung in Neutral. Alles rein visuell, keine States.
  std::uint8_t variant_ = 0;
  std::uint32_t onsetUntil_ = 0;
  std::uint32_t nextMicroAt_ = 0;
  std::uint32_t microUntil_ = 0;
  std::uint8_t microKind_ = 0;
  float sEyeOpen_ = 1.0f;
  float sMouthCurve_ = 0.0f;
  float sMustache_ = 0.0f;
  float sBlinkMul_ = 1.0f;
  std::uint8_t gazeMode_ = 0;

  // Augenbrauen (Sprint 3, Einheit 6): weich interpolierte Ausdrucksebene
  float sBrowLift_ = 0.0f;  // + angehoben / - gesenkt
  float sBrowTilt_ = 0.0f;  // + innere Enden tiefer (streng) / - hoeher (weich)
  float sBrowAsym_ = 0.0f;  // eine Braue hoeher (nachdenklich/verwirrt)
  float sBrowVis_ = 1.0f;   // Sichtbarkeit 0..1 (Sleeping -> 0, ruhiges Ausblenden)

  // Microcopy / Gedankenblase (Sprint 3)
  const char* sayText_ = nullptr;
  std::uint32_t sayUntil_ = 0;
};

}  // namespace pc
