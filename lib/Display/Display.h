#pragma once
// ============================================================================
//  Display - Kapselung der Bildschirmausgabe (M5.Display / M5GFX)
//
//  Verantwortung (Single Responsibility Principle):
//  - Kennt das Display und weiss, WIE gezeichnet wird.
//  - Bietet der App semantische Methoden (z. B. showGreeting()), statt
//    ueberall im Code direkte M5GFX-Aufrufe zu verstreuen.
//
//  Bewusst NICHT Aufgabe dieser Klasse:
//  - App-Logik, Zustaende, Timing. Das Display "malt nur" - es entscheidet
//    nicht, WANN oder WARUM etwas gezeichnet wird.
// ============================================================================

namespace pc {

class Display {
 public:
  // Display-spezifische Grundeinstellungen (Rotation, Text-Defaults).
  // Voraussetzung: M5.begin() wurde bereits aufgerufen (siehe App::setup()).
  void begin();

  // Zeichnet den statischen Hello-World-Screen aus Sprint 0.
  void showGreeting();

 private:
  // Fuellt den Bildschirm mit der Hintergrundfarbe.
  void clear();
};

}  // namespace pc
