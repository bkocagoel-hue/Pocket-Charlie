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

 private:
  void clear();
};

}  // namespace pc
