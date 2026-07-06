#include "Persona.h"

#include <Arduino.h>

namespace pc {
namespace {
// Timing / Schwellen (leicht tunbar).
constexpr std::uint32_t kBootHappyMs   = 1500;
constexpr std::uint32_t kTouchHappyMs  = 1200;
constexpr std::uint32_t kAnnoyedMs     = 1800;
constexpr std::uint32_t kThoughtfulMs  = 2800;   // BtnB kurz -> Thoughtful
constexpr std::uint32_t kIdleTiredMs   = 20000;  // Inaktivitaet -> Tired
constexpr std::uint32_t kIdleSleepingMs = 40000;  // Inaktivitaet -> Sleeping

// Rapid-Tap -> Annoyed (bewusst nicht zu empfindlich).
constexpr std::uint16_t kAnnoyTaps     = 4;
constexpr std::uint32_t kAnnoyWindowMs = 1800;

const char* emotionName(Emotion e) {
  switch (e) {
    case Emotion::Neutral:    return "Neutral";
    case Emotion::Happy:      return "Happy";
    case Emotion::Tired:      return "Tired";
    case Emotion::Thoughtful: return "Thoughtful";
    case Emotion::Annoyed:    return "Annoyed";
    case Emotion::Sleeping:   return "Sleeping";
    default:                  return "?";
  }
}
}  // namespace

void Persona::begin() {
  current_ = Emotion::Neutral;
  lastLogged_ = Emotion::Neutral;
  started_ = false;
  transientUntil_ = 0;
  lastActivityMs_ = 0;
  tapCount_ = 0;
  tapWindowStart_ = 0;
}

void Persona::setTransient(Emotion e, std::uint32_t nowMs, std::uint32_t durMs) {
  current_ = e;
  transientUntil_ = nowMs + durMs;
}

void Persona::update(std::uint32_t nowMs, const Input& input) {
  if (!started_) {
    // Erster Tick nach dem Boot-Splash: kurzer Begruessungs-Happy.
    started_ = true;
    lastActivityMs_ = nowMs;
    setTransient(Emotion::Happy, nowMs, kBootHappyMs);
  } else {
    const bool touched = input.wasPressed();
    const bool anyButton = input.btnAPressed() || input.btnBPressed() ||
                           input.btnCPressed() || input.btnPwrPressed();
    if (touched || anyButton) lastActivityMs_ = nowMs;  // Aktivitaet -> wach

    // Rapid-Tap zaehlen (nur echte Touches).
    bool rapid = false;
    if (touched) {
      if (nowMs - tapWindowStart_ > kAnnoyWindowMs) {
        tapWindowStart_ = nowMs;
        tapCount_ = 1;
      } else {
        ++tapCount_;
      }
      if (tapCount_ >= kAnnoyTaps) {
        rapid = true;
        tapCount_ = 0;
      }
    }

    // Trigger nach Prioritaet: Annoyed > Thoughtful (BtnB) > Happy (Touch).
    if (rapid) {
      setTransient(Emotion::Annoyed, nowMs, kAnnoyedMs);
    } else if (input.btnBPressed()) {
      setTransient(Emotion::Thoughtful, nowMs, kThoughtfulMs);
    } else if (touched) {
      setTransient(Emotion::Happy, nowMs, kTouchHappyMs);
    }

    // Transienten Zustand auslaufen lassen -> Neutral.
    if (transientUntil_ != 0 && nowMs >= transientUntil_) {
      transientUntil_ = 0;
      current_ = Emotion::Neutral;
    }

    // Ohne aktiven transienten Zustand: Inaktivitaet -> Tired -> Sleeping,
    // bei Aktivitaet sauber aufwachen.
    if (transientUntil_ == 0) {
      const std::uint32_t idle = nowMs - lastActivityMs_;
      if (idle >= kIdleSleepingMs) {
        current_ = Emotion::Sleeping;
      } else if (idle >= kIdleTiredMs) {
        current_ = Emotion::Tired;
      } else if (current_ == Emotion::Tired || current_ == Emotion::Sleeping) {
        current_ = Emotion::Neutral;  // aufgewacht
      }
    }
  }

  // Emotionswechsel loggen (nur bei tatsaechlicher Aenderung -> keine Flut).
  if (current_ != lastLogged_) {
    Serial.printf("[Persona] -> %s\n", emotionName(current_));
    lastLogged_ = current_;
  }
}

}  // namespace pc
