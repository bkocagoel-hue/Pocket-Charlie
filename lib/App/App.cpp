#include "App.h"

#include <Arduino.h>
#include <M5Unified.h>
#include <cstdio>
#include <cstring>

#include "PcConfig.h"

namespace pc {
namespace {
// Sprint 8, Einheit 2: kSayMs/kSayGapMs/kExcitedMs/kHappyNetMs/kConfusedMs/
// kSadMs sind in BehaviorEngine.cpp umgezogen (dort verwendet). kCuriousMs
// bleibt hier - direkte User-Input-Reaktion (System-BtnB), keine
// Behavior-Engine-Zustaendigkeit.
constexpr std::uint32_t kCuriousMs  = 3000;  // BtnB auf Text-Screens

// Sprint 8 (Awake, Einheit 1): Anzahl interner System-Unterseiten
// (0 = Status/Uptime, 1 = Settings/Sound, 2 = Info/Version).
constexpr int kSystemPageCount = 3;
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
  behavior_.begin(persona_, face_);  // Sprint 8, Einheit 2
  menu_.begin();
  network_.begin();  // Sprint 4: non-blocking; verbindet (falls Secrets) im
                     // Hintergrund waehrend des Boot-Splash. Ohne WLAN laeuft
                     // Charlie unveraendert lokal (local-first).
  online_.begin();   // Sprint 4 E3: Bridge-Ping-Task (Core 0); ohne Bridge-URL
                     // deaktiviert. Pings nur manuell via BtnB.
  prod_.begin();     // Sprint 5: Fokus-Werkzeuge (rein lokal)
  sound_.begin();    // Sprint 5: dezentes Timer-Audio (stumm-sicher)
  dice_.begin();     // Sprint 7: Dice/Coin (rein lokal, kein Zeitverhalten)
  card_.begin();     // Sprint 7: Focus Card (rein lokal, kein Zeitverhalten)
  beat_.begin();     // Sprint 7: Beatbox (rein lokal, kein Zeitverhalten)
  eightBall_.begin();// Sprint 7: Magic 8-Ball (rein lokal, kein Zeitverhalten)

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

  // Sprint 8, Einheit 2: Prioritaet 1 (kritische Systemereignisse) - reiner
  // Verhaltens-Umzug aus dem vorherigen Inline-Block, siehe BehaviorEngine.
  behavior_.onSystemEvent(now, online_.state(), online_.thoughtState(),
                         online_.thoughtSeq(), network_.justConnected());

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

  // Sprint 5 + Sprint 8 Einheit 2: Productivity-Ereignisse (Prioritaet 2) -
  // Sound bleibt hier (Behavior Engine kennt Sound bewusst nicht), Emotion/
  // Phrase-Auswahl ist reiner Verhaltens-Umzug in BehaviorEngine.
  const ProdEvent pe = prod_.takeEvent();
  if (pe != ProdEvent::None) {
    if (menu_.current() == Screen::Focus) screenRedraw_ = true;
    switch (pe) {
      case ProdEvent::CountdownDone: sound_.playTimerDone();  break;
      case ProdEvent::FocusDone:     sound_.playFocusDone();  break;
      case ProdEvent::BreakDone:     sound_.playBreakDone();  break;
      default: break;
    }
    const bool focusRunning = (prod_.status() == ProdStatus::Running);
    behavior_.onProdEvent(now, pe, focusRunning);
  }

  persona_.update(now, input_);  // jede Runde: Eingabe-Flanken nicht verpassen

  // Sprint 8, Einheit 2: Prioritaet 3/4 (Emotionswechsel-Microcopy, Idle-
  // Phrasen) - liest den durch obige Pokes bereits aktualisierten Zustand.
  behavior_.onEmotionTick(now, persona_.current(), persona_.moodLevel());

  if (now - lastFrameMs_ >= config::kFrameIntervalMs) {
    lastFrameMs_ = now;
    if (menu_.pocketOpen()) {
      // Sprint 7 (Pocketindex - Rolodex Notebook): offener Pocketindex hat
      // Vorrang vor Face/Widget-Screens - tickende Screens (Clock/
      // Productivity/Online) duerfen ihn nicht ueberzeichnen, da
      // renderScreen() hier bewusst nicht laeuft.
      renderPocketindex();
    } else if (menu_.current() == Screen::Home) {
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

  // Sprint 8 (Awake, Einheit 1): Beatbox ist aus der Hauptnavigation
  // geparkt (siehe Menu::Screen) - die Touch-Zonen-Sonderbehandlung entfaellt
  // hier entsprechend. Beatbox.* und Display::beatboxZoneAt()/
  // showBeatboxGrid() bleiben unveraendert im Code erhalten.

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
  if (menu_.current() == Screen::Focus) {
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
  } else if (menu_.current() == Screen::Home) {
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
  } else if (menu_.current() == Screen::Think) {
    // A/C bewusst ohne eigene Funktion (wie zuvor auf Online) - BtnB-
    // Verhalten inhaltlich unveraendert gegenueber vor Sprint 8, nur jetzt
    // ein eigener Hauptmodus statt eines Pocketindex-Screens unter vielen.
    if (input_.btnBClicked()) {
      if (network_.online()) {
        // Bridge ok -> Thought holen; sonst (unbekannt/down) erst /health
        // pingen. "checking"/"waiting..." sind die sichtbare Rueckmeldung.
        if (online_.state() == BridgeState::Ok) {
          online_.requestThought();
        } else {
          online_.requestPing();
        }
        screenRedraw_ = true;
      } else {
        network_.retry();  // WLAN offline: BtnB = Verbindungs-Retry
        flashActive_ = true;
        screenFlashUntil_ = nowMs + 1000;
        screenRedraw_ = true;
      }
    }
  } else if (menu_.current() == Screen::System) {
    // Sprint 8 (Awake, Einheit 1): System buendelt Status(Uptime)/Settings/
    // Info als EIN Hauptmodus - A/C wechseln nur die interne Unterseite
    // (systemPage_), dasselbe Muster wie Productivity/Focus im Ready-
    // Zustand. Kein zweiter Pocketindex-artiger Mechanismus noetig.
    if (interaction_.btnA()) {
      systemPage_ = (systemPage_ + kSystemPageCount - 1) % kSystemPageCount;
      screenRedraw_ = true;
    }
    if (interaction_.btnC()) {
      systemPage_ = (systemPage_ + 1) % kSystemPageCount;
      screenRedraw_ = true;
    }
    if (input_.btnBClicked()) {
      if (systemPage_ == 1) {
        // Settings-Seite: Sound umschalten; beim Einschalten kurze
        // Hoerprobe - inhaltlich unveraendert gegenueber vor Sprint 8.
        sound_.setEnabled(!sound_.enabled());
        if (sound_.enabled()) sound_.playTimerDone();
      } else {
        // Status(Uptime)/Info-Seite: kurzer Curious-Moment - inhaltlich
        // unveraendert gegenueber dem frueheren Clock/Info-Verhalten.
        persona_.poke(Emotion::Curious, kCuriousMs);
        flashActive_ = true;
        screenFlashUntil_ = nowMs + 1000;
      }
      screenRedraw_ = true;
    }
  } else if (menu_.current() == Screen::Care) {
    // Sprint 8 (Awake, Einheit 1): Care ist bewusst ein Stub - keine
    // Reminder-Logik, keine Aktion auf A/B/C (folgt in einer spaeteren
    // Einheit, siehe Behavior-Engine-Vorschlag).
  }
}

void App::renderScreen(std::uint32_t nowMs) {
  const Screen s = menu_.current();

  // System-Status-Unterseite (Uptime) aktualisiert sich sekundenweise.
  if (s == Screen::System && systemPage_ == 0) {
    const std::uint32_t sec = nowMs / 1000;
    if (sec != lastUptimeSec_) {
      lastUptimeSec_ = sec;
      screenRedraw_ = true;
    }
  }
  // Focus aktualisiert sich sekuendlich (laufende Timer).
  if (s == Screen::Focus) {
    const std::uint32_t sec = prod_.displaySeconds();
    if (sec != lastProdSec_) {
      lastProdSec_ = sec;
      screenRedraw_ = true;
    }
  }
  // Think aktualisiert sich bei WiFi-/Bridge-/Thought-Statuswechsel oder
  // neuem Thought-Text (alles kombiniert in einem Vergleichswert).
  if (s == Screen::Think) {
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
  // Beatbox-Zonen-Tap-Feedback-Ende -> einmal neu zeichnen (Highlight weg).
  if (beatFlashZone_ != -1 && nowMs >= beatFlashUntil_) {
    beatFlashZone_ = -1;
    screenRedraw_ = true;
  }
  // Screen gewechselt (oder kam vom Face) -> neu zeichnen.
  if (static_cast<int>(s) != lastRenderedScreen_) screenRedraw_ = true;

  if (!screenRedraw_) return;
  screenRedraw_ = false;
  lastRenderedScreen_ = static_cast<int>(s);

  // Je Modus genau ein Text-Widget (Home wird nicht hier gezeichnet, siehe
  // loop()). Dice/FocusCard/Beatbox/EightBall sind aus der Navigation
  // geparkt (siehe Menu::Screen) - ihre render*Widget()-Funktionen bleiben
  // unten im Code stehen, werden hier aber bewusst nicht mehr angesprungen.
  switch (s) {
    case Screen::Think:  renderOnlineWidget();       break;
    case Screen::Focus:  renderProductivityWidget(); break;
    case Screen::System: renderSystemWidget(nowMs);  break;
    case Screen::Care:   renderCareWidget();         break;
    default:              break;
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
  // Sprint 8 (Awake, Einheit 1): A/C wechseln jetzt die System-Unterseite
  // (Status/Settings/Info, siehe App::handleButtons()) statt ohne Funktion
  // zu sein - Hinweise muessen das ehrlich zeigen. BtnB loest weiterhin
  // einen kurzen "Curious"-Moment aus (unveraendert).
  display_.drawNavBar(menu_.index(), Menu::count(), "A: prev", "B: hey", "C: next");
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
  // Sprint 8 (Awake, Einheit 1): A/C wechseln die System-Unterseite (siehe
  // renderClockWidget()) - "B: toggle" bleibt die einzige Aktion hier.
  display_.drawNavBar(menu_.index(), Menu::count(), "A: prev", "B: toggle", "C: next");
}

void App::renderInfoWidget() {
  display_.showScreen(config::kAppName, config::kAppCodename,
                      flashActive_ ? "ok" : config::kAppVersion);
  // Sprint 8 (Awake, Einheit 1): A/C wechseln die System-Unterseite (siehe
  // renderClockWidget()). BtnB loest weiterhin einen kurzen "Curious"-
  // Moment aus (unveraendert).
  display_.drawNavBar(menu_.index(), Menu::count(), "A: prev", "B: hey", "C: next");
}

void App::renderSystemWidget(std::uint32_t nowMs) {
  // Sprint 8 (Awake, Einheit 1): System ist EIN Hauptmodus mit drei
  // Unterseiten (Status/Uptime, Settings/Sound, Info/Version) statt drei
  // eigener Pocketindex-Karten. Wiederverwendet die bestehenden, unveraen-
  // derten Widget-Funktionen 1:1 - nur der Aufruf ist jetzt bedingt.
  switch (systemPage_) {
    case 0:  renderClockWidget(nowMs);  break;
    case 1:  renderSettingsWidget();    break;
    default: renderInfoWidget();        break;
  }
}

void App::renderCareWidget() {
  // Sprint 8 (Awake, Einheit 1): bewusster Platzhalter - keine Reminder-
  // Logik, keine neuen Aktionen. A/B/C bleiben ehrlich leer.
  display_.showScreen("care", "steht bereit.", "bald mehr hier.");
  display_.drawNavBar(menu_.index(), Menu::count(), "", "", "");
}

void App::renderDiceWidget() {
  // Sprint 7: erste Pocketindex-Mini-App. A/C wechseln den Modus (kein
  // Reroll, rein informativ bis zum naechsten B), B wuerfelt/wirft.
  char result[8];
  dice_.resultText(result, sizeof(result));
  const char* sub = dice_.hasRolled() ? "" : "tap B to roll";
  display_.showScreen(dice_.modeName(), result, sub);
  display_.drawNavBar(menu_.index(), Menu::count(), "A: prev mode", "B: roll",
                      "C: next mode");
}

void App::renderFocusCardWidget() {
  // Sprint 7: zweite Pocketindex-Mini-App. A/C wechseln die Kategorie (kein
  // neuer Zug, rein informativ bis zum naechsten B), B zieht eine Karte.
  // showScreen()s eingebautes Text-Fitting uebernimmt die laengeren Prompts
  // sicher (wie schon bei Online-Thoughts) - kein eigenes Kuerzen noetig.
  const char* text = card_.hasDrawn() ? card_.cardText() : "";
  const char* sub = card_.hasDrawn() ? "" : "tap B for a card";
  display_.showScreen(card_.categoryName(), text, sub);
  display_.drawNavBar(menu_.index(), Menu::count(), "A: prev cat", "B: draw",
                      "C: next cat");
}

void App::renderBeatboxWidget() {
  // Sprint 7 (Touch-Zonen): dritte Pocketindex-Mini-App. Vier Zonen zeigen
  // alle Sounds gleichzeitig (kein A/C-Zyklus mehr); ein Tap trifft direkt.
  // Reihenfolge muss zu Display::beatboxZoneAt() passen (0=oben-links,
  // 1=oben-rechts, 2=unten-links, 3=unten-rechts) - dieselbe wie DrumKit.
  static const char* const kLabels[4] = {"KICK", "SNARE", "HIHAT", "CLAP"};
  display_.showBeatboxGrid(kLabels, beatFlashZone_);
  // Nur die Positions-Punktreihe (wie jeder andere Widget-Screen) - keine
  // A/B/C-Hints, die Tasten sind hier bewusst inert (siehe handleButtons()).
  display_.drawNavBar(menu_.index(), Menu::count(), "", "", "");
}

void App::renderEightBallWidget() {
  // Sprint 7: vierte Pocketindex-Mini-App. A/C bewusst ohne eigene Funktion
  // (wie Beatbox) - B zieht eine neue (absichtlich alberne) Antwort.
  const char* text = eightBall_.hasAsked() ? eightBall_.answerText() : "";
  const char* sub = eightBall_.hasAsked() ? "" : "tap B to ask";
  display_.showScreen("8-ball", text, sub);
  display_.drawNavBar(menu_.index(), Menu::count(), "", "B: ask", "");
}

}  // namespace pc
