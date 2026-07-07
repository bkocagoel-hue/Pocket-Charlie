#include "Sound.h"

#include <Arduino.h>
#include <M5Unified.h>

namespace pc {
namespace {
constexpr std::uint8_t kVolume = 72;  // moderat (0..255) - dezent, nicht laut
}  // namespace

void Sound::begin() {
  available_ = M5.Speaker.isEnabled();
  if (available_) {
    M5.Speaker.setVolume(kVolume);
    Serial.println("[Sound] Speaker bereit (dezent).");
  } else {
    Serial.println("[Sound] kein Speaker verfuegbar -> stumm, alles laeuft.");
  }
}

void Sound::tone(float freqHz, std::uint32_t durMs) {
  if (!enabled_ || !available_) return;
  M5.Speaker.tone(freqHz, durMs);  // non-blocking, kurzer Einzelton
}

void Sound::playTimerDone() { tone(880.0f, 160); }   // A5, freundlich
void Sound::playFocusDone() { tone(988.0f, 160); }   // H5, "Pause verdient"
void Sound::playBreakDone() { tone(1319.0f, 200); }  // E6, kleiner Abschluss

}  // namespace pc
