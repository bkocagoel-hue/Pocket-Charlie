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

  // Sprint 3: Button-Menuefuehrung. BtnA/BtnC navigieren, BtnB = Screen-Aktion.
  if (interaction_.btnA()) {
    menu_.prev();
    screenRedraw_ = true;
    Serial.printf("[Menu] screen: %s\n", menu_.name());
  }
  if (interaction_.btnC()) {
    menu_.next();
    screenRedraw_ = true;
    Serial.printf("[Menu] screen: %s\n", menu_.name());
  }
  if (menu_.current() == Screen::Productivity) {
    // Sprint 5: BtnB kurz = start/pause/weiter, BtnB halten = reset/modus.
    // Bewusst Klick (Loslassen) statt Druck-Flanke, damit Halten nicht
    // zusaetzlich eine Kurz-Aktion ausloest.
    if (input_.btnBClicked()) {
      prod_.primaryAction(now);
      screenRedraw_ = true;
    }
    if (input_.btnBHeld()) {
      prod_.secondaryAction(now);
      screenRedraw_ = true;
    }
  } else if (interaction_.btnB()) {
    if (menu_.current() == Screen::Face) {
      persona_.pokeThoughtful();  // Face-Aktion: kurz nachdenklich
    } else if (menu_.current() == Screen::Online && network_.online()) {
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
        // E4B: BtnB auf Clock/Mood/Info -> kurzer Curious-Moment (sichtbar
        // z. B. als Emotion im Mood-Screen oder beim Rueckwechsel zum Face).
        persona_.poke(Emotion::Curious, kCuriousMs);
      }
      flashActive_ = true;  // kurze "ok"-Rueckmeldung
      screenFlashUntil_ = now + 1000;
      screenRedraw_ = true;
    }
  }

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
    if (menu_.current() == Screen::Face) {
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

  // Menue-Icon-Zone oben rechts - nur auf den Widget-Screens aktiv (Face
  // zeigt das Icon (noch) nicht, siehe loop()/renderScreen(), daher bleibt
  // hier bewusst auch keine unsichtbare Trefferzone). Noch kein echter
  // Launcher (folgt in Einheit 3); ein Tap hier darf NICHT zusaetzlich
  // Faces lookAt()/blinkNow() ausloesen. Eigenes, kurzes Tap-Feedback
  // (menuIconFlash_) statt flashActive_ - sonst wuerde ein Icon-Tap die
  // Sub-Zeile jedes Widgets ueberschreiben, obwohl gar keine Screen-Aktion
  // passiert ist.
  if (input_.wasPressed() && menu_.current() != Screen::Face &&
      Display::isMenuIconZone(input_.touchX(), input_.touchY())) {
    menuIconFlash_ = true;
    menuIconFlashUntil_ = millis() + 250;
    screenRedraw_ = true;
    Serial.printf("[Menu] icon tap x=%d y=%d\n", input_.touchX(),
                  input_.touchY());
    return;
  }

  // Touch bleibt emotionale Interaktion: Charlie schaut zum Punkt + blinzelt.
  // (Buttons steuern die Menuefuehrung - siehe loop().)
  if (input_.wasPressed()) {
    face_.lookAt(input_.touchX(), input_.touchY());
    face_.blinkNow();
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
  // laesst es kurz aufblitzen (Tap-Feedback), noch kein Launcher-Inhalt.
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
  display_.drawNavBar(menu_.index(), Menu::count(), "");
}

void App::renderMoodWidget() {
  display_.showScreen("mood", persona_.moodName(),
                      flashActive_ ? "ok" : persona_.stateName());
  display_.drawNavBar(menu_.index(), Menu::count(), "");
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
  display_.drawNavBar(menu_.index(), Menu::count(), action);
}

void App::renderProductivityWidget() {
  // Ruhig und klar: Modus als Titel, Zeit gross, Status klein darunter,
  // Bedien-Hinweise in der NavBar - keine Smartwatch-Ueberladung.
  char timeBuf[12];
  char sub[16];
  prod_.timeText(timeBuf, sizeof(timeBuf));
  prod_.subText(sub, sizeof(sub));
  display_.showScreen(prod_.modeName(), timeBuf, sub);
  display_.drawNavBar(menu_.index(), Menu::count(), prod_.actionHint());
}

void App::renderSettingsWidget() {
  // Bewusst schlicht: eine Einstellung, ein Toggle. Pomodoro 25/5 bleibt
  // Konstante; Persistenz ist bewusst noch nicht dran (Runtime-Setting).
  display_.showScreen("settings", sound_.enabled() ? "sound on" : "sound off",
                      "");
  display_.drawNavBar(menu_.index(), Menu::count(), "B: toggle");
}

void App::renderInfoWidget() {
  display_.showScreen(config::kAppName, config::kAppCodename,
                      flashActive_ ? "ok" : config::kAppVersion);
  display_.drawNavBar(menu_.index(), Menu::count(), "");
}

}  // namespace pc
