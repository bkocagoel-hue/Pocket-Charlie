#include "App.h"

#include <Arduino.h>
#include <M5Unified.h>
#include <cstdio>
#include <cstring>

#include "PcConfig.h"
#include "Phrases.h"

namespace pc {
namespace {
constexpr std::uint32_t kSayMs    = 1600;  // Anzeigedauer einer Textblase
constexpr std::uint32_t kSayGapMs = 3000;  // Mindestabstand zwischen Spruechen

// Sprint 4 E4B: Dauer der kleinen emotionalen Online-Momente (transient).
constexpr std::uint32_t kExcitedMs = 2500;  // WLAN verbunden
constexpr std::uint32_t kHappyNetMs = 1600;  // Thought erfolgreich
constexpr std::uint32_t kConfusedMs = 2200;  // Bridge down / Timeout
constexpr std::uint32_t kSadMs      = 2500;  // wiederholter Online-Fehler
constexpr std::uint32_t kCuriousMs  = 3000;  // BtnB auf Text-Screens
}  // namespace

void App::setup() {
  // 1) M5Unified initialisieren (Display, Power/AXP2101, Touch, Buttons, I2C).
  auto cfg = M5.config();
  M5.begin(cfg);

  // CoreS3 hat keine physischen A/B/C-Tasten: M5Unified bildet sie ueber ein
  // Touch-Band am unteren Rand ab (Default-Hoehe 0 => inaktiv). Hier aktivieren.
  M5.setTouchButtonHeight(config::kTouchButtonHeight);

  // 2) Serielle Logs (via USB) fuer Entwicklung/Debugging.
  Serial.begin(115200);
  Serial.printf("[%s] Boot v%s\n", config::kAppName, config::kAppVersion);

  // 3) Subsysteme hochfahren.
  display_.begin();
  display_.showBootScreen();

  input_.begin();
  interaction_.begin();
  face_.begin(M5.Display.width(), M5.Display.height());
  persona_.begin();
  menu_.begin();
  network_.begin();  // Sprint 4: non-blocking; verbindet (falls Secrets) im
                     // Hintergrund waehrend des Boot-Splash. Ohne WLAN laeuft
                     // Charlie unveraendert lokal (local-first).
  online_.begin();   // Sprint 4 E3: Bridge-Ping-Task (Core 0); ohne Bridge-URL
                     // deaktiviert. Pings nur manuell via BtnB.
  prod_.begin();     // Sprint 5: Fokus-Werkzeuge (rein lokal)
  sound_.begin();    // Sprint 5: dezentes Timer-Audio (stumm-sicher)
  nextIdlePhraseAt_ = millis() + 30000;  // erste Idle-Microcopy fruehestens ~30 s

  // 4) Boot-Splash kurz stehen lassen, dann uebernimmt die Loop das Gesicht.
  //    (Blockierendes delay ist hier bewusst ok: einmalig, in setup().)
  delay(config::kBootScreenMs);

  Serial.println("[App] setup() abgeschlossen - Charlie ist wach.");
}

void App::loop() {
  // Nicht-blockierende Game-Loop:
  //  - Hardware/Eingaben jeden Durchlauf pollen (maximale Reaktionsschnelle).
  //  - Animation nur mit fester Bildrate rendern (~30 FPS).
  M5.update();       // Buttons/Touch/... aktualisieren
  input_.update();   // Eingabe-Snapshot ziehen
  handleInput();     // Reaktionen ausloesen

  const std::uint32_t now = millis();

  // Sprint 4: WLAN-Status fortschreiben (non-blocking, nur Polling).
  network_.update(now);
  if (network_.justDisconnected()) {
    online_.reset();  // altes Bridge-Ergebnis ist ohne WLAN nicht mehr ehrlich
  }

  // Sprint 4 E4B: Online-Ereignisse als kleine emotionale Momente uebersetzen.
  // Einweg (Persona haengt NIE vom Netz ab); Fehler sind kurze Momente, keine
  // kaputte Dauerstimmung - danach faellt alles automatisch auf Neutral zurueck.
  if (network_.justConnected()) {
    persona_.poke(Emotion::Excited, kExcitedMs);
  }
  {
    const int bs = static_cast<int>(online_.state());
    const int ts = static_cast<int>(online_.thoughtState());
    const bool pingFailed =
        (bs == static_cast<int>(BridgeState::Down) &&
         prevBridge_ == static_cast<int>(BridgeState::Checking));
    const bool thoughtFailed =
        (ts == static_cast<int>(ThoughtState::Failed) &&
         prevThought_ == static_cast<int>(ThoughtState::Fetching));
    if (pingFailed || thoughtFailed) {
      ++onlineFails_;
      persona_.poke(onlineFails_ >= 2 ? Emotion::Sad : Emotion::Confused,
                    onlineFails_ >= 2 ? kSadMs : kConfusedMs);
    }
    const std::uint8_t seq = online_.thoughtSeq();
    if (seq != prevThoughtSeq_) {  // neuer Thought angekommen
      prevThoughtSeq_ = seq;
      onlineFails_ = 0;
      persona_.poke(Emotion::Happy, kHappyNetMs);
    }
    if (bs == static_cast<int>(BridgeState::Ok) &&
        prevBridge_ == static_cast<int>(BridgeState::Checking)) {
      onlineFails_ = 0;  // Health ok -> Fehlerserie beendet
    }
    prevBridge_ = bs;
    prevThought_ = ts;
  }

  // Sprint 3: Eingaben zu Intents klassifizieren (read-only) und loggen.
  // Aendert (noch) kein Verhalten - Persona/Face bleiben zustaendig.
  interaction_.update(now, input_);
  if (interaction_.rapidTap()) {
    Serial.println("[Intent] RapidTap");
  } else if (interaction_.doubleTap()) {
    Serial.println("[Intent] DoubleTap");
  } else if (interaction_.singleTap()) {
    Serial.println("[Intent] SingleTap");
  }
  if (interaction_.btnA()) Serial.println("[Intent] BtnA");
  if (interaction_.btnB()) Serial.println("[Intent] BtnB");
  if (interaction_.btnC()) Serial.println("[Intent] BtnC");
  if (interaction_.pwr())  Serial.println("[Intent] PWR");

  // Sprint 5: Productivity-Zeitlogik fortschreiben (non-blocking).
  prod_.update(now);

  handleButtons(now);

  // Sprint 5: Productivity-Ereignisse in kurze emotionale Momente, dezente
  // Toene und Charlie-Microcopy uebersetzen (nach der Bedienung abholen,
  // damit Button-Aktionen sofort wirken). Alles transient, keine Dauerstimmung.
  const ProdEvent pe = prod_.takeEvent();
  if (pe != ProdEvent::None) {
    if (menu_.current() == Screen::Productivity) screenRedraw_ = true;
    const char* phrase = nullptr;
    switch (pe) {
      case ProdEvent::Started:
        persona_.poke(Emotion::Thoughtful, 2500);
        phrase = phrases::kFocusStart[random(phrases::kFocusStartN)];
        break;
      case ProdEvent::Resumed:
        persona_.poke(Emotion::Curious, 1500);
        break;
      case ProdEvent::Paused:  // bewusst ruhig: kein Poke, nur ein Satz
        phrase = phrases::kProdPause[random(phrases::kProdPauseN)];
        break;
      case ProdEvent::Reset:
        persona_.poke(Emotion::Confused, 1500);
        phrase = phrases::kProdReset[random(phrases::kProdResetN)];
        break;
      case ProdEvent::ModeChanged:
        persona_.poke(Emotion::Curious, 1500);
        break;
      case ProdEvent::CountdownDone:
        persona_.poke(Emotion::Happy, 2500);  // Timer gelandet
        sound_.playTimerDone();
        phrase = phrases::kProdDone[random(phrases::kProdDoneN)];
        break;
      case ProdEvent::FocusDone:
        persona_.poke(Emotion::Happy, 2000);  // Break beginnt
        sound_.playFocusDone();
        phrase = phrases::kProdPause[random(phrases::kProdPauseN)];
        break;
      case ProdEvent::BreakDone:
        persona_.poke(Emotion::Excited, 2500);  // Durchgang geschafft
        sound_.playBreakDone();
        phrase = phrases::kProdDone[random(phrases::kProdDoneN)];
        break;
      default:
        break;
    }
    if (phrase != nullptr && now - lastSayMs_ >= kSayGapMs) {
      face_.say(phrase, kSayMs);
      lastSayMs_ = now;  // blockt die generische Microcopy des Emotionswechsels
    }
  }

  persona_.update(now, input_);  // jede Runde: Eingabe-Flanken nicht verpassen

  // Sprint 3: dezente Microcopy je nach Zustand (rate-limitiert).
  const Emotion emo = persona_.current();
  const bool sayAllowed = (now - lastSayMs_ >= kSayGapMs);
  if (emo != prevEmotion_) {
    // Emotionswechsel -> passende, seltene Microcopy (E4B: neue Emotionen).
    const char* phrase = nullptr;
    switch (emo) {
      case Emotion::Happy:
        phrase = phrases::kGreet[random(phrases::kGreetN)]; break;
      case Emotion::Annoyed:
        phrase = phrases::kGrumble[random(phrases::kGrumbleN)]; break;
      case Emotion::Curious:
        phrase = phrases::kCurious[random(phrases::kCuriousN)]; break;
      case Emotion::Confused:
        phrase = phrases::kConfused[random(phrases::kConfusedN)]; break;
      case Emotion::Excited:
        phrase = phrases::kExcited[random(phrases::kExcitedN)]; break;
      case Emotion::Sad:
        phrase = phrases::kSad[random(phrases::kSadN)]; break;
      case Emotion::WakingUp:
        phrase = phrases::kWakeUp[random(phrases::kWakeUpN)]; break;
      default:
        break;  // Neutral/Tired/Sleeping/Thoughtful: bewusst wortlos
    }
    if (sayAllowed && phrase != nullptr) {
      face_.say(phrase, kSayMs);
      lastSayMs_ = now;
    }
    prevEmotion_ = emo;
  } else if (emo == Emotion::Neutral && now >= nextIdlePhraseAt_ && sayAllowed) {
    const int mood = persona_.moodLevel();  // Mood light faerbt die Idle-Sprueche
    const char* phrase =
        (mood > 0) ? phrases::kIdleHigh[random(phrases::kIdleHighN)]
      : (mood < 0) ? phrases::kIdleLow[random(phrases::kIdleLowN)]
                   : phrases::kIdle[random(phrases::kIdleN)];
    face_.say(phrase, kSayMs);
    lastSayMs_ = now;
    nextIdlePhraseAt_ = now + random(25000, 45000);
  }

  if (now - lastFrameMs_ >= config::kFrameIntervalMs) {
    lastFrameMs_ = now;
    if (menu_.pocketOpen()) {
      // Sprint 7 (Pocketindex - Rolodex Notebook): offener Pocketindex hat
      // Vorrang vor Face/Widget-Screens - tickende Screens (Clock/
      // Productivity/Online) duerfen ihn nicht ueberzeichnen, da
      // renderScreen() hier bewusst nicht laeuft.
      renderPocketindex();
    } else if (menu_.current() == Screen::Face) {
      face_.setEmotion(persona_.current());
      face_.update(now);
      face_.render();
      // Sprint 7, E1 (gefixt): KEIN zusaetzlicher Icon-Draw hier - ein
      // separater M5.Display-Schreibzugriff jeden Frame (~30x/s) direkt nach
      // Faces eigenem Canvas-Push verursachte sichtbares Flackern. Face
      // bleibt bewusst ohne Icon (und ohne Face.cpp-Aenderung).
      lastRenderedScreen_ = -1;  // Display zeigt jetzt das Gesicht
    } else {
      renderScreen(now);
    }
  }

  delay(1);  // an FreeRTOS zurueckgeben (kein Busy-Wait, Watchdog zufrieden)
}

void App::handleInput() {
  // Sprint 7, E1 Debug: rohe Touch-Koordinaten bei jedem Antippen loggen,
  // solange die Icon-Hitbox getestet/kalibriert wird.
  if (input_.wasPressed()) {
    Serial.printf("[Touch] x=%d y=%d\n", input_.touchX(), input_.touchY());
  }

  // Menue-Icon-Zone oben rechts - bewusst auf JEDEM Screen als Trefferzone
  // aktiv, auch auf Face (das Icon wird dort weiterhin nicht gezeichnet,
  // siehe loop()/E1-Begruendung; die Zone bleibt trotzdem ein "Hot Corner",
  // damit der Pocketindex auch vom Face-Screen aus erreichbar ist). Oeffnet
  // bzw. schliesst per Toggle; ein Tap hier darf NICHT zusaetzlich Faces
  // lookAt()/blinkNow() ausloesen. Eigenes, kurzes Tap-Feedback
  // (menuIconFlash_) statt flashActive_ - sonst wuerde ein Icon-Tap die
  // Sub-Zeile jedes Widgets ueberschreiben, obwohl gar keine Screen-Aktion
  // passiert ist.
  if (input_.wasPressed() &&
      Display::isMenuIconZone(input_.touchX(), input_.touchY())) {
    const bool wasOpen = menu_.pocketOpen();
    menuIconFlash_ = true;
    menuIconFlashUntil_ = millis() + 250;
    menu_.togglePocketindex();
    screenRedraw_ = true;   // falls wir zu einem Widget-Screen zurueckkehren
    pocketRedraw_ = true;   // falls der Pocketindex gerade geoeffnet wurde
    if (wasOpen) {
      sound_.playPocketClose();
    } else {
      sound_.playPocketOpen();
      pocketPulse_ = true;  // kurzer Highlight-Puls auf dem Kartentitel beim Oeffnen
      pocketPulseUntil_ = millis() + 150;
    }
    Serial.printf("[Pocketindex] icon tap x=%d y=%d\n", input_.touchX(),
                  input_.touchY());
    return;
  }

  // Sprint 7, Fix: bei offenem Pocketindex bleibt Touch auf die Icon-Zone
  // (oben behandelt) beschraenkt; ein generischer Tap schliesst den
  // Pocketindex (Sprint-Vorgabe: "optional Touch ausserhalb schliesst") -
  // ABER NICHT, wenn dieser Touch im unteren A/B/C-Band liegt. Wichtig:
  // diese Ausnahme rein GEOMETRISCH pruefen (Display::isButtonBandZone),
  // NICHT ueber input_.btnAPressed()/btnBPressed()/btnCPressed() - deren
  // (debounced) Button-Objekte melden den Druck oft ein paar Frames
  // SPAETER als der rohe Touch. Mit der Flag-Pruefung schloss der erste
  // Touch-Frame eines A/C-Drucks den Pocketindex bereits, BEVOR
  // handleButtons() (siehe loop()) den Tastendruck ueberhaupt sehen
  // konnte - Tasten wirkten tot, man landete sofort wieder im vorherigen
  // Screen. Die geometrische Pruefung ist unabhaengig von jedem
  // Debounce-Timing. Pocketindex-eigene A/C/B-Navigation lebt bewusst in
  // handleButtons(), nicht hier.
  if (menu_.pocketOpen()) {
    const bool isButtonZoneTouch = Display::isButtonBandZone(input_.touchY());
    if (input_.wasPressed() && !isButtonZoneTouch) {
      menu_.closePocketindex();
      screenRedraw_ = true;
      sound_.playPocketClose();
    }
    return;
  }

  // Touch bleibt emotionale Interaktion: Charlie schaut zum Punkt + blinzelt.
  // (Buttons steuern die Menuefuehrung - siehe handleButtons().)
  if (input_.wasPressed()) {
    face_.lookAt(input_.touchX(), input_.touchY());
    face_.blinkNow();
  }
}

void App::handleButtons(std::uint32_t nowMs) {
  // Sprint 7, Fix (Stabilisierung): BtnB HALTEN oeffnet global den
  // Pocketindex, wenn er gerade geschlossen ist - der garantierte
  // Hardware-Fallback, denn A/C navigieren ja nirgends mehr zwischen
  // Screens. Funktioniert von jedem Screen aus (kein if/else auf
  // menu_.current() noetig - absichtlich vor jeder Screen-Verzweigung).
  //
  // Warum das nicht mit den Screen-B-Aktionen kollidiert: Input.h
  // garantiert bereits (M5Unified-Vertrag, siehe input_.btnBClicked()/
  // btnBHeld()-Doku), dass "Clicked" (kurz) und "Held" (lang) sich
  // GEGENSEITIG ausschliessen - "Ein Halten loest KEIN Clicked aus".
  // Deshalb nutzen ab hier ALLE Screen-B-Aktionen (unten) btnBClicked()
  // statt der reinen Druck-Flanke - ein langer Druck feuert dadurch NIE
  // zusaetzlich eine Screen- oder Pocketindex-Auswahl-Aktion.
  if (!menu_.pocketOpen() && input_.btnBHeld()) {
    menu_.openPocketindex();
    pocketRedraw_ = true;
    pocketPulse_ = true;
    pocketPulseUntil_ = nowMs + 150;
    sound_.playPocketOpen();  // bestehender Ton, keine neue Sound-Architektur
    Serial.println("[Pocketindex] opened via long-press B");
    return;  // dieser Druck ist verbraucht - keine Screen-/Karten-Aktion mehr
  }

  // Sprint 7, Fix: klare Prioritaet - offener Pocketindex besitzt A/C/B
  // exklusiv fuer diesen Frame (Karten-Navigation: A=vorherige, C=naechste
  // Karte). Die normale Screen-Bedienung laeuft NUR im else-Zweig, also nie
  // im selben Frame wie die Pocketindex-Navigation.
  if (menu_.pocketOpen()) {
    if (interaction_.btnA()) {
      menu_.pocketPrev();
      pocketRedraw_ = true;
      pocketPulse_ = true;  // kurzer Kartenwechsel-Puls
      pocketPulseUntil_ = nowMs + 150;
      sound_.playPocketMove();
    }
    if (interaction_.btnC()) {
      menu_.pocketNext();
      pocketRedraw_ = true;
      pocketPulse_ = true;
      pocketPulseUntil_ = nowMs + 150;
      sound_.playPocketMove();
    }
    if (input_.btnBClicked()) {
      // Der Pocketindex schliesst hier sofort - ein Titel-Puls waere nie
      // sichtbar (naechster Frame zeigt schon den Zielscreen). Die
      // "Select"-Rueckmeldung ist deshalb bewusst nur akustisch.
      sound_.playPocketSelect();
      menu_.selectPocketCard();
      screenRedraw_ = true;  // Zielscreen soll beim Schliessen neu zeichnen
      Serial.printf("[Pocketindex] open %s\n", menu_.name());
    }
    return;
  }

  // Sprint 7, Fix ("Claude unleashed", stabilisiert): A/B/C wechseln
  // NIRGENDS mehr global den Screen - das ist ausschliesslich Aufgabe des
  // Pocketindex (Menue-Icon-Tap oder BtnB-Halten, siehe oben). Innerhalb
  // eines Screens sind A/B/C screen-eigene Funktionen; welcher Screen aktiv
  // ist, entscheidet per if/else auf menu_.current().
  if (menu_.current() == Screen::Productivity) {
    // Ready (nichts laeuft): A/C waehlen das Werkzeug - ersetzt das
    // fruehere "BtnB halten" bei Ready (das ist jetzt der globale
    // Pocketindex-Fallback). Running/Paused/Done: A/C haben ihre eigene,
    // direkte Funktion je Modus (siehe Productivity::aHint()/cHint()).
    if (prod_.status() == ProdStatus::Ready) {
      if (interaction_.btnA()) {
        prod_.cycleModePrev();
        screenRedraw_ = true;
      }
      if (interaction_.btnC()) {
        prod_.cycleModeNext();
        screenRedraw_ = true;
      }
    } else {
      switch (prod_.mode()) {
        case ProdMode::Stopwatch:
          if (interaction_.btnA()) {
            prod_.resetStopwatch(nowMs);
            screenRedraw_ = true;
          }
          if (interaction_.btnC()) {
            prod_.markLap();
            std::snprintf(lapFlashBuf_, sizeof(lapFlashBuf_), "lap %u",
                          static_cast<unsigned>(prod_.lapCount()));
            flashActive_ = true;
            screenFlashUntil_ = nowMs + 1000;
            screenRedraw_ = true;
            sound_.playAdjustTick();
          }
          break;
        case ProdMode::Countdown:
        case ProdMode::Pomodoro:
          if (interaction_.btnA()) {
            prod_.adjustTarget(-60L * 1000L);  // -1 Minute
            screenRedraw_ = true;
            sound_.playAdjustTick();
          }
          if (interaction_.btnC()) {
            prod_.adjustTarget(60L * 1000L);  // +1 Minute
            screenRedraw_ = true;
            sound_.playAdjustTick();
          }
          break;
      }
    }
    // BtnB kurz = start/pause/weiter (bzw. Done bestaetigen -> bereit).
    // BtnB halten ist oben bereits global abgehandelt (Pocketindex).
    if (input_.btnBClicked()) {
      prod_.primaryAction(nowMs);
      screenRedraw_ = true;
    }
  } else if (menu_.current() == Screen::Face) {
    // A/C: Charlie schaut kurz nach links/rechts und blinzelt - dieselbe
    // oeffentliche Face-API wie beim Touch-Blick (kein Face.cpp-Zugriff).
    if (interaction_.btnA()) {
      face_.lookAt(0, M5.Display.height() / 2);
      face_.blinkNow();
    }
    if (interaction_.btnC()) {
      face_.lookAt(M5.Display.width(), M5.Display.height() / 2);
      face_.blinkNow();
    }
    if (input_.btnBClicked()) {
      persona_.pokeThoughtful();  // Face-Aktion: kurz nachdenklich (unveraendert)
    }
  } else if (menu_.current() == Screen::Mood) {
    // A/C: Charlies Stimmung direkt anstupsen - passt thematisch genau zum
    // Mood-Screen. Kurze, transiente Pokes (dieselbe API wie die Online-
    // Emotionsmomente), keine Dauerstimmung.
    if (interaction_.btnA()) {
      persona_.poke(Emotion::Sad, kSadMs);
      screenRedraw_ = true;
    }
    if (interaction_.btnC()) {
      persona_.poke(Emotion::Excited, kExcitedMs);
      screenRedraw_ = true;
    }
    if (input_.btnBClicked()) {
      persona_.poke(Emotion::Curious, kCuriousMs);
      flashActive_ = true;
      screenFlashUntil_ = nowMs + 1000;
      screenRedraw_ = true;
    }
  } else if (input_.btnBClicked()) {
    // Clock/Online/Settings/Info: A/C bewusst (noch) ohne eigene Funktion -
    // BtnB-Verhalten inhaltlich unveraendert gegenueber vor Sprint 7, nur
    // jetzt konsequent ueber btnBClicked() statt der reinen Druck-Flanke.
    if (menu_.current() == Screen::Online && network_.online()) {
      // Bridge ok -> Thought holen; sonst (unbekannt/down) erst /health pingen.
      // "checking"/"waiting..." sind die sichtbare Rueckmeldung.
      if (online_.state() == BridgeState::Ok) {
        online_.requestThought();
      } else {
        online_.requestPing();
      }
      screenRedraw_ = true;
    } else if (menu_.current() == Screen::Settings) {
      // Sprint 5: BtnB = Sound umschalten; beim Einschalten kurze Hoerprobe.
      sound_.setEnabled(!sound_.enabled());
      if (sound_.enabled()) sound_.playTimerDone();
      screenRedraw_ = true;
    } else {
      if (menu_.current() == Screen::Online) {
        network_.retry();  // WLAN offline: BtnB = Verbindungs-Retry
      } else {
        // E4B: BtnB auf Clock/Info -> kurzer Curious-Moment (sichtbar z. B.
        // beim Rueckwechsel zum Face).
        persona_.poke(Emotion::Curious, kCuriousMs);
      }
      flashActive_ = true;  // kurze "ok"-Rueckmeldung
      screenFlashUntil_ = nowMs + 1000;
      screenRedraw_ = true;
    }
  }
}

void App::renderScreen(std::uint32_t nowMs) {
  const Screen s = menu_.current();

  // Clock aktualisiert sich sekundenweise.
  if (s == Screen::Clock) {
    const std::uint32_t sec = nowMs / 1000;
    if (sec != lastUptimeSec_) {
      lastUptimeSec_ = sec;
      screenRedraw_ = true;
    }
  }
  // Productivity aktualisiert sich sekuendlich (laufende Timer).
  if (s == Screen::Productivity) {
    const std::uint32_t sec = prod_.displaySeconds();
    if (sec != lastProdSec_) {
      lastProdSec_ = sec;
      screenRedraw_ = true;
    }
  }
  // Online-Screen aktualisiert sich bei WiFi-/Bridge-/Thought-Statuswechsel
  // oder neuem Thought-Text (alles kombiniert in einem Vergleichswert).
  if (s == Screen::Online) {
    const int ns = ((static_cast<int>(network_.state()) * 8 +
                     static_cast<int>(online_.state())) * 8 +
                    static_cast<int>(online_.thoughtState())) * 256 +
                   online_.thoughtSeq();
    if (ns != lastNetState_) {
      lastNetState_ = ns;
      screenRedraw_ = true;
    }
  }
  // Flash-Ende -> einmal neu zeichnen (Rueckmeldung entfernen).
  if (flashActive_ && nowMs >= screenFlashUntil_) {
    flashActive_ = false;
    screenRedraw_ = true;
  }
  // Menue-Icon-Tap-Feedback-Ende -> ebenfalls einmal neu zeichnen.
  if (menuIconFlash_ && nowMs >= menuIconFlashUntil_) {
    menuIconFlash_ = false;
    screenRedraw_ = true;
  }
  // Screen gewechselt (oder kam vom Face) -> neu zeichnen.
  if (static_cast<int>(s) != lastRenderedScreen_) screenRedraw_ = true;

  if (!screenRedraw_) return;
  screenRedraw_ = false;
  lastRenderedScreen_ = static_cast<int>(s);

  // Je Screen genau ein Text-Widget (Face wird nicht hier gezeichnet).
  switch (s) {
    case Screen::Clock:        renderClockWidget(nowMs);   break;
    case Screen::Mood:         renderMoodWidget();         break;
    case Screen::Online:       renderOnlineWidget();       break;
    case Screen::Productivity: renderProductivityWidget(); break;
    case Screen::Settings:     renderSettingsWidget();     break;
    case Screen::Info:         renderInfoWidget();         break;
    default:                   break;
  }
  // Sprint 7, E1 (gefixt): Menue-Icon oben rechts auf allen Widget-Screens -
  // Teil desselben Redraw-Aufrufs wie showScreen()/drawNavBar() (kein
  // separater Extra-Draw pro Frame -> kein Flackern). "menuIconFlash_"
  // laesst es kurz aufblitzen (Tap-Feedback).
  display_.drawMenuIcon(menuIconFlash_);
}

void App::renderPocketindex() {
  const std::uint32_t nowMs = millis();

  // Menue-Icon-Tap-Feedback-Ende -> einmal neu zeichnen (eigener Trigger,
  // getrennt vom Widget-Pfad in renderScreen() - siehe App.h).
  if (menuIconFlash_ && nowMs >= menuIconFlashUntil_) {
    menuIconFlash_ = false;
    pocketRedraw_ = true;
  }
  // Karten-Puls-Ende (Oeffnen/Wechsel) -> ebenfalls einmal neu zeichnen.
  if (pocketPulse_ && nowMs >= pocketPulseUntil_) {
    pocketPulse_ = false;
    pocketRedraw_ = true;
  }

  if (!pocketRedraw_) return;
  pocketRedraw_ = false;
  lastRenderedScreen_ = -2;  // kein Screen-Index (-1 = Face) -> erzwingt
                             // beim Verlassen des Pocketindex einen Redraw.

  const int idx = menu_.pocketIndex();
  int lineCount = 0;
  const char* const* lines = Menu::cardLines(idx, &lineCount);
  display_.showPocketindex(Menu::nameAt(idx), lines, lineCount, idx,
                           Menu::count(), pocketPulse_);
  // Konsistent mit den Widget-Screens: gleiche Stelle, gleicher Aufruf.
  display_.drawMenuIcon(menuIconFlash_);
}

void App::renderClockWidget(std::uint32_t nowMs) {
  // Uptime HH:MM:SS - bewusst KEINE echte Uhrzeit (kein RTC/NTP in Sprint 3).
  const std::uint32_t t = nowMs / 1000;
  snprintf(uptimeBuf_, sizeof(uptimeBuf_), "%02u:%02u:%02u",
           static_cast<unsigned>(t / 3600),
           static_cast<unsigned>((t / 60) % 60),
           static_cast<unsigned>(t % 60));
  display_.showScreen("uptime", uptimeBuf_, flashActive_ ? "ok" : "");
  // A/C bewusst ohne eigene Funktion auf Clock (noch keine passende Idee) -
  // leere Hinweise zeichnen keine Pfeile mehr vor. BtnB loest im else-Zweig
  // von handleButtons() einen kurzen "Curious"-Moment aus - ehrlich benannt.
  display_.drawNavBar(menu_.index(), Menu::count(), "", "B: hey", "");
}

void App::renderMoodWidget() {
  display_.showScreen("mood", persona_.moodName(),
                      flashActive_ ? "ok" : persona_.stateName());
  display_.drawNavBar(menu_.index(), Menu::count(), "A: down", "B: hey", "C: up");
}

void App::renderOnlineWidget() {
  // Offline ist ein normaler Zustand: kurze, ruhige Anzeige statt
  // Fehlermeldungs-Wand. Ohne WLAN -> WiFi-Sicht (BtnB = retry); mit WLAN ->
  // Bridge-/Thought-Sicht. Die BtnB-Hinweise leben in der NavBar (Sprint 5),
  // die Sub-Zeile traegt nur noch Status (IP, "still me", ...).
  const char* title = "wifi";
  const char* mainText = network_.stateName();
  const char* sub = "";
  const char* action = "";

  if (!network_.online()) {
    switch (network_.state()) {
      case NetState::Offline:  action = "B: retry";  break;
      case NetState::Disabled: sub = "no secrets";   break;
      default:                 break;  // Connecting: nur "trying"
    }
  } else {
    // Sprint 6, E3: knappe Provider-/AI-Sichtbarkeit, z. B. "ollama ai:on".
    // Neutrale Defaults ("unknown"/"fallback"), solange nichts Genaueres
    // bekannt ist - siehe OnlineClient::providerName()/aiStatusName().
    std::snprintf(aiSubBuf_, sizeof(aiSubBuf_), "%s ai:%s",
                  online_.providerName(), online_.aiStatusName());

    const ThoughtState ts = online_.thoughtState();
    if (ts == ThoughtState::Fetching) {
      title = "online";
      mainText = "waiting...";
    } else if (online_.state() == BridgeState::Checking) {
      title = "bridge";
      mainText = "checking";
    } else if (ts == ThoughtState::Ok) {
      title = "online";
      mainText = online_.thoughtText();
      // Sprint 6, E5: hier zaehlt die TATSAECHLICHE Quelle dieses einen
      // Thoughts (kann bei bridge-seitigem Fallback vom Konfigurationsstatus
      // abweichen), nicht der generelle "provider ai:status" von /health.
      // Aeltere Bridge ohne "source"-Feld -> weiterhin Konfigurationsstatus.
      const char* thoughtSrc = online_.thoughtSourceName();
      if (std::strcmp(thoughtSrc, "unknown") == 0) {
        // aiSubBuf_ enthaelt bereits den generellen Status (oben berechnet).
      } else if (std::strcmp(thoughtSrc, "ollama") == 0) {
        std::snprintf(aiSubBuf_, sizeof(aiSubBuf_), "ollama ai:on");
      } else if (std::strcmp(online_.providerName(), "ollama") == 0) {
        std::snprintf(aiSubBuf_, sizeof(aiSubBuf_), "mock fallback");
      } else {
        std::snprintf(aiSubBuf_, sizeof(aiSubBuf_), "mock ai:off");
      }
      sub = aiSubBuf_;
      action = "B: again";
    } else if (ts == ThoughtState::Failed) {
      // Charmanter Fallback statt Fehlermeldung; BtnB pingt danach /health.
      title = "online";
      mainText = "offline";
      sub = "still me";
      action = "B: ping";
    } else {
      title = "bridge";
      mainText = online_.stateName();
      sub = aiSubBuf_;
      switch (online_.state()) {
        case BridgeState::Idle: action = "B: ping";    break;
        case BridgeState::Ok:   action = "B: thought"; break;
        case BridgeState::Down: action = "B: ping";    break;
        default:                break;  // no url
      }
    }
  }

  display_.showScreen(title, mainText, flashActive_ ? "ok" : sub);
  // A/C bewusst ohne eigene Funktion auf Online (noch keine passende Idee,
  // die nicht mit BtnBs bestehender "smarter" Aktion ueberladen wirkt).
  display_.drawNavBar(menu_.index(), Menu::count(), "", action, "");
}

void App::renderProductivityWidget() {
  // Ruhig und klar: Modus als Titel, Zeit gross, Status klein darunter,
  // Bedien-Hinweise in der NavBar - keine Smartwatch-Ueberladung.
  char timeBuf[12];
  char sub[16];
  prod_.timeText(timeBuf, sizeof(timeBuf));
  prod_.subText(sub, sizeof(sub));
  // Sprint 7, Fix: Lap-Marke (Stopwatch, BtnC) zeigt kurz "lap N" statt der
  // sonst dauerhaft sichtbaren Status-Zeile - nutzt denselben
  // flashActive_-Timer wie die generische "ok"-Rueckmeldung anderswo.
  display_.showScreen(prod_.modeName(), timeBuf,
                      flashActive_ ? lapFlashBuf_ : sub);
  display_.drawNavBar(menu_.index(), Menu::count(), prod_.aHint(),
                      prod_.actionHint(), prod_.cHint());
}

void App::renderSettingsWidget() {
  // Bewusst schlicht: eine Einstellung, ein Toggle. Pomodoro 25/5 bleibt
  // Konstante; Persistenz ist bewusst noch nicht dran (Runtime-Setting).
  display_.showScreen("settings", sound_.enabled() ? "sound on" : "sound off",
                      "");
  // A/C bewusst ohne eigene Funktion auf Settings (nur eine Option bisher).
  display_.drawNavBar(menu_.index(), Menu::count(), "", "B: toggle", "");
}

void App::renderInfoWidget() {
  display_.showScreen(config::kAppName, config::kAppCodename,
                      flashActive_ ? "ok" : config::kAppVersion);
  // A/C bewusst ohne eigene Funktion auf Info. BtnB loest im else-Zweig von
  // handleButtons() einen kurzen "Curious"-Moment aus - ehrlich benannt.
  display_.drawNavBar(menu_.index(), Menu::count(), "", "B: hey", "");
}

}  // namespace pc
