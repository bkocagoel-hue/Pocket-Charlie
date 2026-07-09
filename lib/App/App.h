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

#include "Dice.h"
#include "Display.h"
#include "Face.h"
#include "FocusCard.h"
#include "Input.h"
#include "InputContext.h"
#include "Menu.h"
#include "NetworkManager.h"
#include "OnlineClient.h"
#include "Persona.h"
#include "Productivity.h"
#include "Sound.h"

namespace pc {

class App {
 public:
  void setup();
  void loop();

 private:
  // Wertet die Eingaben dieses Frames aus und loest Reaktionen aus.
  void handleInput();
  // Sprint 7: Button-Dispatch (A/C/B), aus loop() ausgelagert - eigene
  // Methode mit klarer Prioritaet: offener Pocketindex besitzt A/C/B
  // exklusiv (kein Fallthrough in die normale Screen-Navigation). Bleibt
  // bewusst screen-bewusst (if/else auf menu_.current()), damit spaetere
  // screen-spezifische Tastenbelegung (Stopwatch/Timer/Pomodoro/Settings)
  // hier andocken kann, ohne die Struktur nochmal umbauen zu muessen.
  void handleButtons(std::uint32_t nowMs);
  // Zeichnet die Text-Screens (Clock/Mood/Info) - nur bei Aenderung.
  void renderScreen(std::uint32_t nowMs);
  // Sprint 7 (Pocketindex - Rolodex Notebook): Vollbild-Kartenindex - nur
  // bei Aenderung (Oeffnen/Schliessen, Kartenwechsel, Pulse-Feedback).
  void renderPocketindex();
  // Einzelne Text-Widgets (je genau ein Widget pro Screen).
  void renderClockWidget(std::uint32_t nowMs);
  void renderMoodWidget();
  void renderOnlineWidget();
  void renderProductivityWidget();
  void renderSettingsWidget();
  void renderInfoWidget();
  void renderDiceWidget();
  void renderFocusCardWidget();

  Display display_;  // Screen-Grundfunktionen / Boot-Screen
  Input   input_;    // Touch + Buttons
  InputContext interaction_;  // Sprint 3: Eingaben -> Intents (read-only)
  Face    face_;     // Charlies animiertes Gesicht
  Persona persona_;  // Stimmungs-/Emotionszustand (Sprint 2)
  NetworkManager network_;  // Sprint 4: optionales WLAN (local-first)
  OnlineClient   online_;   // Sprint 4: Bridge-Ping (Task auf Core 0)
  Productivity   prod_;     // Sprint 5: Stopwatch/Countdown/Pomodoro
  Sound          sound_;    // Sprint 5: dezentes Timer-Audio (abschaltbar)
  Dice           dice_;     // Sprint 7: erste Pocketindex-Mini-App (d6/d20/Muenze)
  FocusCard      card_;     // Sprint 7: zweite Pocketindex-Mini-App (Focus/Break/Reset-Prompts)

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

  // Sprint 7, Fix (Screen-eigene Tasten): kurzzeitiger "lap N"-Text anstatt
  // des generischen "ok", wenn auf dem Productivity-Screen im Stopwatch-
  // Modus eine Lap-Marke gesetzt wird - nutzt denselben flashActive_-Timer.
  char lapFlashBuf_[16] = {0};

  // Sprint 7, E1: eigenes, kurzes Tap-Feedback fuer das Menue-Icon -
  // bewusst getrennt von flashActive_ (das ueberschreibt sonst die Sub-Zeile
  // jedes Widgets mit "ok", obwohl der Icon-Tap gar keine Screen-Aktion ist).
  bool menuIconFlash_ = false;
  std::uint32_t menuIconFlashUntil_ = 0;

  // Sprint 7 (Pocketindex - Rolodex Notebook): eigener Redraw-Trigger fuer
  // den Pocketindex, getrennt von screenRedraw_ (der gehoert den Widget-
  // Screens; beide Pfade schliessen sich gegenseitig aus, siehe loop()).
  bool pocketRedraw_ = false;
  // Kurzer Highlight-Puls auf dem Kartentitel bei Oeffnen/Wechsel/Auswahl -
  // dasselbe Timer-Flag-Muster wie menuIconFlash_ (ein Redraw beim Setzen,
  // ein Redraw beim Ablaufen, kein Extra-Draw pro Frame -> kein Flackern).
  bool pocketPulse_ = false;
  std::uint32_t pocketPulseUntil_ = 0;

  // Sprint 6, E3: "provider ai:status" fuer den Online-Screen (z. B.
  // "ollama ai:on"), aus OnlineClient::providerName()/aiStatusName() gebaut.
  char aiSubBuf_[24] = {0};

  // Sprint 4: Online-Screen zeichnet neu, wenn sich WiFi- ODER Bridge-Status
  // aendert (beide kombiniert in einem Wert, siehe renderScreen()).
  int lastNetState_ = -1;

  // Sprint 5: Productivity-Screen zeichnet sekuendlich neu (wie Clock).
  std::uint32_t lastProdSec_ = 0;

  // Sprint 4 E4B: Online-Ereignisse -> kleine emotionale Momente.
  // Flanken-Erkennung fuer Bridge-/Thought-Ausgang + Fehlerzaehler (Sad).
  int prevBridge_ = -1;
  int prevThought_ = -1;
  std::uint8_t prevThoughtSeq_ = 0;
  std::uint8_t onlineFails_ = 0;

  // Spaetere Subsysteme reihen sich hier ein, z. B.:
  //   WifiService wifi_;   // Netzwerk (Sprint 0/3)
};

}  // namespace pc
