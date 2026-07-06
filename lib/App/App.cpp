#include "App.h"

#include <Arduino.h>
#include <M5Unified.h>

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
    face_.setEmotion(persona_.current());
    face_.update(now);
    face_.render();
  }

  delay(1);  // an FreeRTOS zurueckgeben (kein Busy-Wait, Watchdog zufrieden)
}

void App::handleInput() {
  // --- Touch: Charlie schaut zum Beruehrungspunkt und blinzelt ---
  if (input_.wasPressed()) {
    Serial.printf("[Input] Touch @ (%d, %d)\n", input_.touchX(),
                  input_.touchY());
    face_.lookAt(input_.touchX(), input_.touchY());
    face_.blinkNow();
  }
  if (input_.wasReleased()) {
    Serial.println("[Input] Touch losgelassen");
  }

  // --- Buttons (CoreS3: A/B/C = Touch-Zonen unten, PWR = Power-Taste) ---
  // Drei unterschiedliche Reaktionen -> deckt "mind. 3 Touch-Aktionen" ab.
  if (input_.btnAPressed()) {
    Serial.println("[Input] Button A -> Blick nach links");
    face_.lookAt(0, M5.Display.height() / 2);
  }
  if (input_.btnBPressed()) {
    Serial.println("[Input] Button B -> Blick zur Mitte + blinzeln");
    face_.lookAt(M5.Display.width() / 2, M5.Display.height() / 2);
    face_.blinkNow();
  }
  if (input_.btnCPressed()) {
    Serial.println("[Input] Button C -> Blick nach rechts");
    face_.lookAt(M5.Display.width(), M5.Display.height() / 2);
  }
  if (input_.btnPwrPressed()) {
    Serial.println("[Input] Button PWR (kurz)");
  }
}

}  // namespace pc
