#include "Input.h"

#include <M5Unified.h>

namespace pc {

void Input::begin() {
  // Touch und Buttons werden bereits von M5.begin() konfiguriert.
  // Hier ist aktuell nichts weiter zu tun - die Methode existiert als
  // sauberer Erweiterungspunkt (z. B. spaeter Kalibrierung/Empfindlichkeit).
}

void Input::update() {
  // Touch: ersten Kontaktpunkt auslesen (Multi-Touch ignorieren wir bewusst).
  const auto touch = M5.Touch.getDetail();
  isTouched_   = touch.isPressed();
  wasPressed_  = touch.wasPressed();
  wasReleased_ = touch.wasReleased();
  if (isTouched_ || wasPressed_) {
    touchX_ = touch.x;
    touchY_ = touch.y;
  }

  // Buttons: steigende Flanke abgreifen.
  btnA_   = M5.BtnA.wasPressed();
  btnB_   = M5.BtnB.wasPressed();
  btnC_   = M5.BtnC.wasPressed();
  // Power-Button nur als kurzen Klick werten (langes Halten = Hardware-Off).
  btnPwr_ = M5.BtnPWR.wasClicked();
}

}  // namespace pc
