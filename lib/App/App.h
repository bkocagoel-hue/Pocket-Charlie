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
#include "InputContext.h"
#include "Menu.h"
#include "Persona.h"

namespace pc {

class App {
 public:
  void setup();
  void loop();

 private:
  // Wertet die Eingaben dieses Frames aus und loest Reaktionen aus.
  void handleInput();
  // Zeichnet die Text-Screens (Clock/Mood/Info) - nur bei Aenderung.
  void renderScreen(std::uint32_t nowMs);

  Display display_;  // Screen-Grundfunktionen / Boot-Screen
  Input   input_;    // Touch + Buttons
  InputContext interaction_;  // Sprint 3: Eingaben -> Intents (read-only)
  Face    face_;     // Charlies animiertes Gesicht
  Persona persona_;  // Stimmungs-/Emotionszustand (Sprint 2)

  // Zeitpunkt des letzten gerenderten Frames (fuer feste Bildrate).
  std::uint32_t lastFrameMs_ = 0;

  // Sprint 3: Microcopy-Steuerung
  Emotion       prevEmotion_ = Emotion::Neutral;
  std::uint32_t lastSayMs_ = 0;
  std::uint32_t nextIdlePhraseAt_ = 0;

  // Sprint 3: Button-Menuefuehrung / Text-Screens
  Menu menu_;
  int  lastRenderedScreen_ = -1;    // zuletzt gezeichneter Text-Screen (-1 = Face)
  bool screenRedraw_ = false;
  std::uint32_t lastUptimeSec_ = 0;
  bool flashActive_ = false;
  std::uint32_t screenFlashUntil_ = 0;
  char uptimeBuf_[12] = {0};

  // Spaetere Subsysteme reihen sich hier ein, z. B.:
  //   WifiService wifi_;   // Netzwerk (Sprint 0/3)
};

}  // namespace pc
