#include "BehaviorEngine.h"

#include <Arduino.h>

#include "Face.h"
#include "Phrases.h"

namespace pc {
namespace {
constexpr std::uint32_t kSayMs    = 1600;  // Anzeigedauer einer Textblase
constexpr std::uint32_t kSayGapMs = 3000;  // Mindestabstand zwischen Spruechen

// Sprint 4 E4B: Dauer der kleinen emotionalen Online-Momente (transient).
constexpr std::uint32_t kExcitedMs  = 2500;  // WLAN verbunden
constexpr std::uint32_t kHappyNetMs = 1600;  // Thought erfolgreich
constexpr std::uint32_t kConfusedMs = 2200;  // Bridge down / Timeout
constexpr std::uint32_t kSadMs      = 2500;  // wiederholter Online-Fehler
}  // namespace

void BehaviorEngine::begin(Persona& persona, Face& face) {
  persona_ = &persona;
  face_ = &face;
  prevBridge_ = -1;
  prevThought_ = -1;
  prevThoughtSeq_ = 0;
  onlineFails_ = 0;
  prevEmotion_ = Emotion::Neutral;
  lastSayMs_ = 0;
  // Sprint 8, Einheit 2 Fix: 9s statt vormals 30s. Persona wird nach 20s
  // Inaktivitaet (kIdleTiredMs, siehe Persona.cpp) Tired, danach greift die
  // "current == Emotion::Neutral"-Bedingung unten nicht mehr - eine erste
  // Pruefung bei 30s kam faktisch nie zum Zug. 9s liegt sicher innerhalb des
  // Neutral-Fensters.
  nextIdlePhraseAt_ = millis() + 9000;
}

void BehaviorEngine::onSystemEvent(std::uint32_t nowMs, BridgeState bridge,
                                   ThoughtState thought,
                                   std::uint8_t thoughtSeq,
                                   bool netJustConnected) {
  (void)nowMs;
  // Sprint 4 E4B: Online-Ereignisse als kleine emotionale Momente
  // uebersetzen. Einweg (Persona haengt NIE vom Netz ab); Fehler sind kurze
  // Momente, keine kaputte Dauerstimmung - danach faellt alles automatisch
  // auf Neutral zurueck.
  if (netJustConnected) {
    persona_->poke(Emotion::Excited, kExcitedMs);
  }

  const int bs = static_cast<int>(bridge);
  const int ts = static_cast<int>(thought);
  const bool pingFailed =
      (bs == static_cast<int>(BridgeState::Down) &&
       prevBridge_ == static_cast<int>(BridgeState::Checking));
  const bool thoughtFailed =
      (ts == static_cast<int>(ThoughtState::Failed) &&
       prevThought_ == static_cast<int>(ThoughtState::Fetching));
  if (pingFailed || thoughtFailed) {
    ++onlineFails_;
    persona_->poke(onlineFails_ >= 2 ? Emotion::Sad : Emotion::Confused,
                  onlineFails_ >= 2 ? kSadMs : kConfusedMs);
  }
  if (thoughtSeq != prevThoughtSeq_) {  // neuer Thought angekommen
    prevThoughtSeq_ = thoughtSeq;
    onlineFails_ = 0;
    persona_->poke(Emotion::Happy, kHappyNetMs);
  }
  if (bs == static_cast<int>(BridgeState::Ok) &&
      prevBridge_ == static_cast<int>(BridgeState::Checking)) {
    onlineFails_ = 0;  // Health ok -> Fehlerserie beendet
  }
  prevBridge_ = bs;
  prevThought_ = ts;
}

void BehaviorEngine::onProdEvent(std::uint32_t nowMs, ProdEvent event,
                                 bool focusRunning) {
  (void)focusRunning;  // Sprint 8, Einheit 2: vorbereitet, noch ungenutzt.

  // Sprint 5: Productivity-Ereignisse in kurze emotionale Momente und
  // Charlie-Microcopy uebersetzen. Alles transient, keine Dauerstimmung.
  const char* phrase = nullptr;
  switch (event) {
    case ProdEvent::Started:
      persona_->poke(Emotion::Thoughtful, 2500);
      phrase = phrases::kFocusStart[random(phrases::kFocusStartN)];
      break;
    case ProdEvent::Resumed:
      persona_->poke(Emotion::Curious, 1500);
      break;
    case ProdEvent::Paused:  // bewusst ruhig: kein Poke, nur ein Satz
      phrase = phrases::kProdPause[random(phrases::kProdPauseN)];
      break;
    case ProdEvent::Reset:
      persona_->poke(Emotion::Confused, 1500);
      phrase = phrases::kProdReset[random(phrases::kProdResetN)];
      break;
    case ProdEvent::ModeChanged:
      persona_->poke(Emotion::Curious, 1500);
      break;
    case ProdEvent::CountdownDone:
      persona_->poke(Emotion::Happy, 2500);  // Timer gelandet
      phrase = phrases::kProdDone[random(phrases::kProdDoneN)];
      break;
    case ProdEvent::FocusDone:
      persona_->poke(Emotion::Happy, 2000);  // Break beginnt
      phrase = phrases::kProdPause[random(phrases::kProdPauseN)];
      break;
    case ProdEvent::BreakDone:
      persona_->poke(Emotion::Excited, 2500);  // Durchgang geschafft
      phrase = phrases::kProdDone[random(phrases::kProdDoneN)];
      break;
    default:
      break;
  }
  if (phrase != nullptr && nowMs - lastSayMs_ >= kSayGapMs) {
    face_->say(phrase, kSayMs);
    lastSayMs_ = nowMs;  // blockt die generische Microcopy des Emotionswechsels
  }
}

void BehaviorEngine::onEmotionTick(std::uint32_t nowMs, Emotion current,
                                   int moodLevel) {
  // Sprint 3: dezente Microcopy je nach Zustand (rate-limitiert).
  const bool sayAllowed = (nowMs - lastSayMs_ >= kSayGapMs);
  if (current != prevEmotion_) {
    // Emotionswechsel -> passende, seltene Microcopy (E4B: neue Emotionen).
    const char* phrase = nullptr;
    switch (current) {
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
      face_->say(phrase, kSayMs);
      lastSayMs_ = nowMs;
    }
    prevEmotion_ = current;
  } else if (current == Emotion::Neutral && nowMs >= nextIdlePhraseAt_ &&
            sayAllowed) {
    // Mood light faerbt die Idle-Sprueche.
    const char* phrase =
        (moodLevel > 0) ? phrases::kIdleHigh[random(phrases::kIdleHighN)]
      : (moodLevel < 0) ? phrases::kIdleLow[random(phrases::kIdleLowN)]
                        : phrases::kIdle[random(phrases::kIdleN)];
    face_->say(phrase, kSayMs);
    lastSayMs_ = nowMs;
    // Sprint 8, Einheit 2 Fix: 9-15s statt vormals 25-45s (gleicher Grund wie
    // in begin() - muss innerhalb des 20s-Neutral-Fensters bleiben, siehe
    // Persona.cpp kIdleTiredMs). Kein Dauergeplapper: sobald Tired/Sleeping
    // einsetzt, sperrt "current == Emotion::Neutral" oben ganz normal weiter.
    nextIdlePhraseAt_ = nowMs + random(9000, 15000);
  }
}

}  // namespace pc
