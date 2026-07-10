#pragma once
// ============================================================================
//  BehaviorEngine - zentralisiert die Prioritaet zwischen konkurrierenden
//  Emotion-/Microcopy-Ausloesern (Sprint 8, Awake, Einheit 2).
//
//  Vorher lag diese Logik direkt in App::loop() verstreut (Bridge-/Thought-
//  Flankenerkennung, Productivity-Ereignisse, Emotionswechsel-Microcopy,
//  Idle-Phrasen) - jede Quelle rief unabhaengig persona_.poke()/face_.say()
//  auf. Da Persona::poke() nur einen Single-Slot-Puffer hat (letzter Aufruf
//  vor dem naechsten persona_.update() gewinnt, siehe Persona.h/.cpp),
//  entschied bisher zufaellig die Code-Reihenfolge in App::loop(), welche
//  Quelle "gewinnt", wenn mehrere im selben Frame poken wollten.
//
//  Diese Klasse ist bewusst reiner Verhaltens-Umzug (Sprint 8, Einheit 2):
//  gleiche Phrasen, gleiche Emotionen, gleiche Cooldowns wie vorher - nur
//  die Zustaendigkeit wandert von App.cpp hierher. Kein neues Verhalten.
//
//  Drei Methoden statt eines einzigen update() - notwendig, weil
//  Persona::poke() erst bei persona_.update() wirksam wird: Systemereignisse
//  und Productivity-Ereignisse (Prioritaet 1/2) muessen VOR persona_.update()
//  poken, Emotionswechsel-Microcopy/Idle-Phrasen (Prioritaet 3/4) lesen
//  persona_.current() erst DANACH. App::loop() ruft entsprechend an exakt
//  denselben zwei Stellen wie zuvor auf.
//
//  Prioritaet (hoechste zuerst):
//   1. onSystemEvent()  - Bridge/Thought-Fehler, WLAN verbunden
//   2. onProdEvent()    - Focus-/Productivity-Ereignisse
//   3. onEmotionTick()  - Emotionswechsel-Microcopy
//   4. onEmotionTick()  - Idle-Phrasen (niedrigste Prioritaet, wird von 1-3
//                         verdraengt, da Persona nur einen Poke pro Frame
//                         vorhaelt)
// ============================================================================

#include <cstdint>

#include "OnlineClient.h"  // BridgeState, ThoughtState
#include "Persona.h"       // Emotion, Persona
#include "Productivity.h"  // ProdEvent

namespace pc {

class Face;  // Vorwaertsdeklaration reicht (nur Face::say() wird genutzt).

class BehaviorEngine {
 public:
  void begin(Persona& persona, Face& face);

  // Prioritaet 1: kritische Systemereignisse. Unveraendert gegenueber der
  // fruehesten App::loop()-Logik (Sprint 4 E4B) - jeden Frame aufrufen.
  void onSystemEvent(std::uint32_t nowMs, BridgeState bridge,
                     ThoughtState thought, std::uint8_t thoughtSeq,
                     bool netJustConnected);

  // Prioritaet 2: Focus-/Productivity-Ereignisse (Sprint 5). "focusRunning"
  // ist Sprint 8 Einheit 2 vorbereitet (spaetere Care-Nudge-Unterdrueckung
  // waehrend eines laufenden Focus-Sprints) - wird in dieser Einheit noch
  // NICHT ausgewertet, kein Verhalten haengt davon ab.
  void onProdEvent(std::uint32_t nowMs, ProdEvent event, bool focusRunning);

  // Prioritaet 3/4: Emotionswechsel-Microcopy + Idle-Phrasen. Nach
  // persona.update() aufrufen (liest den dann aktuellen Zustand).
  void onEmotionTick(std::uint32_t nowMs, Emotion current, int moodLevel);

 private:
  Persona* persona_ = nullptr;
  Face* face_ = nullptr;

  // Aus App.h uebernommen (Sprint 4 E4B): Bridge-/Thought-Flankenerkennung
  // + Fehlerzaehler.
  int prevBridge_ = -1;
  int prevThought_ = -1;
  std::uint8_t prevThoughtSeq_ = 0;
  std::uint8_t onlineFails_ = 0;

  // Aus App.h uebernommen (Sprint 3): Microcopy-Steuerung.
  Emotion prevEmotion_ = Emotion::Neutral;
  std::uint32_t lastSayMs_ = 0;
  std::uint32_t nextIdlePhraseAt_ = 0;
};

}  // namespace pc
