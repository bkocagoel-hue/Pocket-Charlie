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
  mood_ = 0.0f;
  moodLevel_ = 0;
  lastMoodTickMs_ = 0;
  hasPending_ = false;
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

    // Trigger nach Prioritaet: externe Aktion > Annoyed > Happy (Touch).
    // (BtnB wird jetzt von der Menuefuehrung behandelt -> pokeThoughtful().)
    if (hasPending_) {
      setTransient(pending_, nowMs, pendingDur_);
      hasPending_ = false;
    } else if (rapid) {
      setTransient(Emotion::Annoyed, nowMs, kAnnoyedMs);
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

    // --- Mood light: dezente, laengerfristige Grundstimmung ---
    if (touched && !rapid) mood_ += 0.06f;  // freundliche Interaktion hebt
    if (rapid)             mood_ -= 0.18f;  // Piesacken senkt
    if (nowMs - lastMoodTickMs_ >= 1000) {  // langsamer Zerfall Richtung 0
      lastMoodTickMs_ = nowMs;
      if (mood_ > 0.02f)       mood_ -= 0.02f;
      else if (mood_ < -0.02f) mood_ += 0.02f;
      else                     mood_ = 0.0f;
    }
    if (mood_ > 1.0f) mood_ = 1.0f;
    if (mood_ < -1.0f) mood_ = -1.0f;
    int lvl = moodLevel_;  // Hysterese gegen Flackern
    if (mood_ > 0.40f) lvl = 1;
    else if (mood_ < -0.40f) lvl = -1;
    else if (mood_ > -0.20f && mood_ < 0.20f) lvl = 0;
    if (lvl != moodLevel_) {
      moodLevel_ = lvl;
      Serial.printf("[Persona] mood: %s\n",
                    lvl > 0 ? "High" : (lvl < 0 ? "Low" : "Neutral"));
    }
  }

  // Emotionswechsel loggen (nur bei tatsaechlicher Aenderung -> keine Flut).
  if (current_ != lastLogged_) {
    Serial.printf("[Persona] -> %s\n", emotionName(current_));
    lastLogged_ = current_;
  }
}

void Persona::pokeThoughtful() {
  pending_ = Emotion::Thoughtful;
  pendingDur_ = kThoughtfulMs;
  hasPending_ = true;
}

const char* Persona::stateName() const { return emotionName(current_); }

const char* Persona::moodName() const {
  return moodLevel_ > 0 ? "high" : (moodLevel_ < 0 ? "low" : "neutral");
}

}  // namespace pc
