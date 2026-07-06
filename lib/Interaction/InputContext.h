#pragma once
// ============================================================================
//  InputContext - kleine lokale Interaktionsschicht (Sprint 3)
//
//  Klassifiziert den rohen Input-Snapshot zu semantischen "Intents"
//  (SingleTap / DoubleTap / RapidTap / Buttons) und haelt einen einfachen
//  Aktivitaets-/Idle-Kontext. Bewusst klein und read-only: dieses Modul
//  entscheidet NICHTS, es liefert nur Kontext. Persona/Face bleiben zustaendig.
// ============================================================================

#include <cstdint>

#include "Input.h"

namespace pc {

class InputContext {
 public:
  void begin();

  // Einmal pro Loop-Runde aufrufen (nach input.update()).
  void update(std::uint32_t nowMs, const Input& input);

  // --- klassifizierte Intents dieses Frames ---
  bool singleTap() const { return singleTap_; }  // Beruehrung begann
  bool doubleTap() const { return doubleTap_; }  // zwei Taps kurz hintereinander
  bool rapidTap()  const { return rapidTap_; }   // viele Taps in kurzer Zeit

  bool btnA() const { return btnA_; }
  bool btnB() const { return btnB_; }
  bool btnC() const { return btnC_; }
  bool pwr()  const { return pwr_; }

  // Zeit seit der letzten Eingabe (ms).
  std::uint32_t idleMs() const { return idleMs_; }

 private:
  bool singleTap_ = false;
  bool doubleTap_ = false;
  bool rapidTap_  = false;
  bool btnA_ = false, btnB_ = false, btnC_ = false, pwr_ = false;

  std::uint32_t lastTapMs_        = 0;
  std::uint32_t rapidWindowStart_ = 0;
  std::uint32_t lastActivityMs_   = 0;
  std::uint32_t idleMs_           = 0;
  std::uint16_t rapidCount_       = 0;
};

}  // namespace pc
