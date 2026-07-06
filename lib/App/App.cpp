#include "App.h"

#include <Arduino.h>
#include <M5Unified.h>
#include <cstdio>

#include "PcConfig.h"
#include "Phrases.h"

namespace pc {
namespace {
constexpr std::uint32_t kSayMs    = 1600;  // Anzeigedauer einer Textblase
constexpr std::uint32_t kSayGapMs = 3000;  // Mindestabstand zwischen Spruechen
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
  if (interaction_.btnB()) {
    if (menu_.current() == Screen::Face) {
      persona_.pokeThoughtful();  // Face-Aktion: kurz nachdenklich
    } else {
      if (menu_.current() == Screen::Online) {
        network_.retry();  // Online-Aktion: manueller Verbindungs-Refresh
      }
      flashActive_ = true;  // andere Screens: kurze "ok"-Rueckmeldung
      screenFlashUntil_ = now + 1000;
      screenRedraw_ = true;
    }
  }

  persona_.update(now, input_);  // jede Runde: Eingabe-Flanken nicht verpassen

  // Sprint 3: dezente Microcopy je nach Zustand (rate-limitiert).
  const Emotion emo = persona_.current();
  const bool sayAllowed = (now - lastSayMs_ >= kSayGapMs);
  if (emo != prevEmotion_) {
    if (sayAllowed && emo == Emotion::Happy) {
      face_.say(phrases::kGreet[random(phrases::kGreetN)], kSayMs);
      lastSayMs_ = now;
    } else if (sayAllowed && emo == Emotion::Annoyed) {
      face_.say(phrases::kGrumble[random(phrases::kGrumbleN)], kSayMs);
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
      lastRenderedScreen_ = -1;  // Display zeigt jetzt das Gesicht
    } else {
      renderScreen(now);
    }
  }

  delay(1);  // an FreeRTOS zurueckgeben (kein Busy-Wait, Watchdog zufrieden)
}

void App::handleInput() {
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
  // Online-Screen aktualisiert sich bei Netz-Statuswechsel.
  if (s == Screen::Online) {
    const int ns = static_cast<int>(network_.state());
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
  // Screen gewechselt (oder kam vom Face) -> neu zeichnen.
  if (static_cast<int>(s) != lastRenderedScreen_) screenRedraw_ = true;

  if (!screenRedraw_) return;
  screenRedraw_ = false;
  lastRenderedScreen_ = static_cast<int>(s);

  // Je Screen genau ein Text-Widget (Face wird nicht hier gezeichnet).
  switch (s) {
    case Screen::Clock:  renderClockWidget(nowMs); break;
    case Screen::Mood:   renderMoodWidget();       break;
    case Screen::Online: renderOnlineWidget();     break;
    case Screen::Info:   renderInfoWidget();       break;
    default:             break;
  }
}

void App::renderClockWidget(std::uint32_t nowMs) {
  // Uptime HH:MM:SS - bewusst KEINE echte Uhrzeit (kein RTC/NTP in Sprint 3).
  const std::uint32_t t = nowMs / 1000;
  snprintf(uptimeBuf_, sizeof(uptimeBuf_), "%02u:%02u:%02u",
           static_cast<unsigned>(t / 3600),
           static_cast<unsigned>((t / 60) % 60),
           static_cast<unsigned>(t % 60));
  display_.showScreen("uptime", uptimeBuf_, flashActive_ ? "ok" : "");
}

void App::renderMoodWidget() {
  display_.showScreen("mood", persona_.moodName(),
                      flashActive_ ? "ok" : persona_.stateName());
}

void App::renderOnlineWidget() {
  // WiFi-Status (E2); Bridge/Thought folgen in E3/E4. Offline ist ein normaler
  // Zustand: kurze, ruhige Anzeige statt Fehlermeldungs-Wand. BtnB = retry.
  const char* sub = "";
  switch (network_.state()) {
    case NetState::Online:   sub = network_.ip(); break;
    case NetState::Offline:  sub = "B: retry";    break;
    case NetState::Disabled: sub = "no secrets";  break;
    default:                 break;  // Connecting: keine Sub-Zeile
  }
  display_.showScreen("wifi", network_.stateName(),
                      flashActive_ ? "ok" : sub);
}

void App::renderInfoWidget() {
  display_.showScreen(config::kAppName, config::kAppCodename,
                      flashActive_ ? "ok" : config::kAppVersion);
}

}  // namespace pc
