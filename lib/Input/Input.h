#pragma once
// ============================================================================
//  Input - Kapselung aller Eingaben (Touchscreen + Buttons)
//
//  Verantwortung:
//  - Liest den Hardware-Zustand (von M5.update() aktualisiert) EINMAL pro
//    Frame aus und stellt ihn als sauberen "Snapshot" bereit.
//  - Der Rest der App spricht nur mit diesem Modul, nie direkt mit M5.Touch /
//    M5.BtnX -> die konkrete Hardware bleibt austauschbar gekapselt.
//
//  Bewusst KEIN M5-Include im Header: Input.h haengt nur von POD-Typen ab.
//  Die M5-Abhaengigkeit lebt ausschliesslich in Input.cpp.
//
//  CoreS3-Besonderheit: A/B/C sind keine physischen Tasten, sondern drei
//  Touch-Zonen am unteren Bildschirmrand (von M5Unified bereitgestellt).
//  PWR ist der physische Power-Button.
// ============================================================================

#include <cstdint>

namespace pc {

class Input {
 public:
  void begin();

  // Einmal pro Frame aufrufen - NACH M5.update().
  void update();

  // --- Touch ---
  bool isTouched() const   { return isTouched_; }
  bool wasPressed() const  { return wasPressed_; }   // Beruehrung begann
  bool wasReleased() const { return wasReleased_; }  // Beruehrung endete
  std::int16_t touchX() const { return touchX_; }
  std::int16_t touchY() const { return touchY_; }

  // --- Buttons (steigende Flanke: genau im Frame des Drucks true) ---
  bool btnAPressed() const   { return btnA_; }
  bool btnBPressed() const   { return btnB_; }
  bool btnCPressed() const   { return btnC_; }
  bool btnPwrPressed() const { return btnPwr_; }

  // --- BtnB kurz vs. lang (Sprint 5, Productivity-Bedienung) ---
  // Clicked = kurz gedrueckt und losgelassen; Held = Halte-Schwelle erreicht
  // (feuert genau einmal). Ein Halten loest KEIN Clicked aus.
  bool btnBClicked() const { return btnBClicked_; }
  bool btnBHeld() const    { return btnBHeld_; }

 private:
  bool isTouched_   = false;
  bool wasPressed_  = false;
  bool wasReleased_ = false;
  std::int16_t touchX_ = 0;
  std::int16_t touchY_ = 0;

  bool btnA_   = false;
  bool btnB_   = false;
  bool btnC_   = false;
  bool btnPwr_ = false;
  bool btnBClicked_ = false;
  bool btnBHeld_    = false;
};

}  // namespace pc
