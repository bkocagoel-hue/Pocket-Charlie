#pragma once
// ============================================================================
//  App - zentrale Anwendungslogik ("Composition Root" / Orchestrator)
//
//  Verantwortung:
//  - Besitzt und initialisiert die Subsysteme (Display, Input, Face).
//  - Steuert den Lebenszyklus: setup() einmalig, loop() periodisch.
//  - Verdrahtet Eingaben mit Reaktionen (Input -> Face).
//
//  main.cpp bleibt ein duenner Einstiegspunkt; die eigentliche Logik ist hier
//  gekapselt und waechst kontrolliert.
// ============================================================================

#include <cstdint>

#include "Display.h"
#include "Face.h"
#include "Input.h"

namespace pc {

class App {
 public:
  void setup();
  void loop();

 private:
  // Wertet die Eingaben dieses Frames aus und loest Reaktionen aus.
  void handleInput();

  Display display_;  // Screen-Grundfunktionen / Boot-Screen
  Input   input_;    // Touch + Buttons
  Face    face_;     // Charlies animiertes Gesicht

  // Zeitpunkt des letzten gerenderten Frames (fuer feste Bildrate).
  std::uint32_t lastFrameMs_ = 0;

  // Spaetere Subsysteme reihen sich hier ein, z. B.:
  //   WifiService wifi_;   // Netzwerk (Sprint 0/3)
  //   Persona     persona_;// Charakter/Zustand (Sprint 1-2)
};

}  // namespace pc
