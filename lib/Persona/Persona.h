#pragma once
// ============================================================================
//  Persona - haelt Charlies aktuelle Grundstimmung (Emotion) und waehlt sie
//  aus einfachen, lokalen Regeln (Zeit + Eingabe). Keine KI, kein Netz.
//
//  Trigger (Sprint 2): Boot -> Happy; Touch -> Happy; schnelles Dauer-Tippen
//  -> Annoyed; Inaktivitaet -> Tired; BtnB -> Thoughtful. Alles nicht-blockierend
//  ueber millis(), mit automatischem Rueckfall auf Neutral.
//
//  TODO (spaeter): laengerfristige "Mood"-Ebene ueber die kurzfristige Emotion
//  legen (ruhig/gelangweilt/neugierig ...). Die Architektur ist dafuer offen -
//  Mood wuerde hier den Neutral-Grundzustand ersetzen. Siehe SPRINT_2_PLAN.md.
// ============================================================================

#include <cstdint>

#include "Emotion.h"
#include "Input.h"

namespace pc {

class Persona {
 public:
  void begin();

  // now = millis(); input = aktueller Eingabe-Snapshot (fuer lokale Trigger).
  // Jede Loop-Runde aufrufen, damit keine Eingabe-Flanke verpasst wird.
  void update(std::uint32_t nowMs, const Input& input);

  Emotion current() const { return current_; }

  // Mood light (Sprint 3): grobe Grundstimmung -1 (low) / 0 / +1 (high).
  int moodLevel() const { return moodLevel_; }

 private:
  void setTransient(Emotion e, std::uint32_t nowMs, std::uint32_t durMs);

  Emotion current_ = Emotion::Neutral;
  Emotion lastLogged_ = Emotion::Neutral;  // fuer Change-Log (keine Log-Flut)
  bool started_ = false;
  std::uint32_t transientUntil_ = 0;  // 0 = kein transienter Zustand aktiv
  std::uint32_t lastActivityMs_ = 0;

  // Rapid-Tap-Erkennung (-> Annoyed)
  std::uint16_t tapCount_ = 0;
  std::uint32_t tapWindowStart_ = 0;

  // Mood light: laengerfristige, dezente Grundstimmung
  float mood_ = 0.0f;                // [-1..+1]
  int   moodLevel_ = 0;              // abgeleitet, mit Hysterese
  std::uint32_t lastMoodTickMs_ = 0;
};

}  // namespace pc
