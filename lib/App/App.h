#pragma once
// ============================================================================
//  App - zentrale Anwendungslogik ("Composition Root" / Orchestrator)
//
//  Verantwortung:
//  - Besitzt und initialisiert die Subsysteme (aktuell nur: Display).
//  - Steuert den Lebenszyklus: setup() einmalig, loop() periodisch.
//  - Ist der EINE Ort, an dem spaeter Input, Services (WLAN/KI) und
//    Persona zusammengefuehrt werden.
//
//  Warum eine App-Klasse statt "alles in main.cpp"?
//  - main.cpp bleibt ein duenner Einstiegspunkt (Arduino-Konvention).
//  - Die eigentliche Logik ist gekapselt, testbar und waechst kontrolliert.
// ============================================================================

#include "Display.h"

namespace pc {

class App {
 public:
  // Einmalige Initialisierung: Hardware (M5) + alle Subsysteme.
  void setup();

  // Ein "Tick" der Hauptschleife. Wird von loop() in main.cpp aufgerufen.
  void loop();

 private:
  Display display_;  // Subsystem: Bildschirmausgabe
  // Spaeter hier ergaenzen, z. B.:
  //   Input   input_;     // Touch / Buttons
  //   WifiService wifi_;  // Netzwerk
  //   Persona persona_;   // Charakter / Zustand
};

}  // namespace pc
