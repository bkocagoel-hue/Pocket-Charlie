#pragma once
// ============================================================================
//  Display - Kapselung screen-weiter Grundfunktionen (M5.Display / M5GFX)
//
//  Verantwortung (Single Responsibility Principle):
//  - Grundeinstellungen des Panels (Rotation, Helligkeit).
//  - Screen-weite, statische Ausgaben wie der Boot-Screen.
//
//  Bewusst NICHT hier:
//  - Charlies animiertes Gesicht -> das ist Aufgabe des Face-Moduls, das mit
//    einem eigenen Off-Screen-Puffer (Canvas) flicker-frei rendert.
// ============================================================================

#include <cstdint>

namespace pc {

class Display {
 public:
  // Panel-Grundeinstellungen. Voraussetzung: M5.begin() lief bereits.
  void begin();

  // Kurzer Start-Bildschirm mit Name + Version (Sprint-1-Bootscreen).
  void showBootScreen();

  // Generischer Text-Screen (Sprint 3): Titel klein + Hauptinfo gross + Sub.
  void showScreen(const char* title, const char* mainText, const char* sub);

  // Navigations-Hinweise (Sprint 5): dezente Leiste am unteren Rand, direkt
  // ueber den A/B/C-Touch-Zonen. Punktreihe = Screens (aktueller violett),
  // "<" / ">" = BtnA/BtnC, Mitte = optionale BtnB-Aktion (z. B. "B: start").
  // Nach showScreen() aufrufen; das Face zeichnet bewusst keine Leiste.
  void drawNavBar(int index, int count, const char* action);

  // Sprint 7, E1 (gefixt): dezentes Menue-Icon oben RECHTS (Touch-
  // Einstiegspunkt fuer den kommenden Launcher, Einheit 3). Nur auf den
  // Widget-Screens aufrufen (Teil von showScreen()s Redraw-Zyklus) - NICHT
  // zusaetzlich pro Frame auf dem Face-Screen, das verursachte Flackern.
  // "highlight" zeigt kurzes Tap-Feedback (heller statt dezent).
  void drawMenuIcon(bool highlight = false);

  // Trefferzone oben rechts (dieselbe Breite wie beim Zeichnen - eine
  // Quelle der Wahrheit, kein Zahlendrift). Nutzt die tatsaechliche
  // Panel-Breite (M5.Display.width()), kein hardcodiertes 320.
  static bool isMenuIconZone(std::int16_t x, std::int16_t y);

 private:
  void clear();

  static constexpr std::int32_t kMenuIconZonePx = 48;
};

}  // namespace pc
