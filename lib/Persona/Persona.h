#pragma once
// ============================================================================
//  Persona - haelt Charlies aktuelle Grundstimmung (Emotion)
//
//  Sprint 2 (Backlog-Item 3): minimal. Persona haelt nur den Zustand und gibt
//  vorerst ausschliesslich Neutral zurueck - noch KEINE Trigger. Die Regeln
//  (Inaktivitaet -> Tired, Touch -> Happy, BtnB-Hold -> Thoughtful, Piesacken
//  -> Annoyed) kommen in spaeteren Schritten.
// ============================================================================

#include <cstdint>

#include "Emotion.h"

namespace pc {

class Persona {
 public:
  void begin();

  // Schreibt den Stimmungszustand fort (now = millis()).
  // Item 3: noch ohne Trigger -> bleibt Neutral.
  void update(std::uint32_t nowMs);

  // Aktuelle Grundstimmung.
  Emotion current() const { return current_; }

 private:
  Emotion current_ = Emotion::Neutral;
};

}  // namespace pc
