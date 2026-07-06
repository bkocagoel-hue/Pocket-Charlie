#pragma once
// ============================================================================
//  OnlineClient - Backend-Bridge-Anbindung (Sprint 4, E3: /health-Ping)
//
//  Prueft auf Anforderung (BtnB im Online-Screen), ob die lokale Bridge
//  erreichbar ist. Der HTTP-Request laeuft in einem kleinen FreeRTOS-Task auf
//  Core 0 (die Arduino-Loop rendert auf Core 1) -> die 30-FPS-Loop friert nie
//  ein, egal ob die Bridge antwortet, haengt oder aus ist.
//
//  Zustaende: Disabled (keine Bridge-URL in PcSecrets.h) / Idle (bereit, noch
//  nicht geprueft) / Checking / Ok / Down (down, Timeout oder Fehler - Details
//  im Serial-Log). Dazu (E4A) ein kurzer "Thought"-Text via GET /thought.
//
//  Nebenlaeufigkeit (bewusst simpel, keine Locks noetig):
//  - Es laeuft immer hoechstens EIN Request; Status als atomare Bytes.
//  - Der Task schreibt den Thought-Puffer NUR waehrend ThoughtState::Fetching;
//    die Loop liest ihn NUR bei ThoughtState::Ok -> kein gleichzeitiger
//    Zugriff, keine String-Lebensdauer-Probleme (fester char-Puffer).
//
//  local-first: keine API-Keys, keine KI, keine Cloud - nur die lokale Bridge.
// ============================================================================

#include <atomic>
#include <cstdint>

namespace pc {

enum class BridgeState : std::uint8_t { Disabled = 0, Idle, Checking, Ok, Down };
enum class ThoughtState : std::uint8_t { None = 0, Fetching, Ok, Failed };

class OnlineClient {
 public:
  // Liest die Bridge-URL (PcSecrets.h) und startet - falls konfiguriert -
  // den Hintergrund-Task. Ohne URL bleibt der Client Disabled (kein Task).
  void begin();

  BridgeState state() const {
    return static_cast<BridgeState>(state_.load(std::memory_order_relaxed));
  }
  const char* stateName() const;  // "no url"/"ready"/"checking"/"ok"/"down"

  // /health-Ping im Hintergrund anstossen (BtnB im Online-Screen).
  // Ignoriert, wenn Disabled oder bereits ein Request laeuft.
  void requestPing();

  // --- Thought (E4A) ---
  // /thought im Hintergrund holen; nur sinnvoll, wenn die Bridge Ok ist.
  void requestThought();
  ThoughtState thoughtState() const {
    return static_cast<ThoughtState>(
        thoughtState_.load(std::memory_order_acquire));
  }
  // Nur gueltig, wenn thoughtState() == Ok (siehe Nebenlaeufigkeits-Regel oben).
  const char* thoughtText() const { return thought_; }
  // Zaehlt erfolgreiche Fetches hoch -> Redraw-Trigger bei neuem Text.
  std::uint8_t thoughtSeq() const {
    return thoughtSeq_.load(std::memory_order_relaxed);
  }

  // Ergebnisse verwerfen (z. B. nach WLAN-Verlust) -> zurueck auf Idle/None.
  void reset();

 private:
  static void taskEntry(void* self);
  void taskLoop();
  void doPing();
  void doThought();
  void setState(BridgeState s) {
    state_.store(static_cast<std::uint8_t>(s), std::memory_order_relaxed);
  }
  void setThoughtState(ThoughtState s) {
    thoughtState_.store(static_cast<std::uint8_t>(s),
                        std::memory_order_release);
  }
  bool requestRunning() const {
    return state() == BridgeState::Checking ||
           thoughtState() == ThoughtState::Fetching;
  }

  std::atomic<std::uint8_t> state_{0};         // BridgeState::Disabled
  std::atomic<std::uint8_t> thoughtState_{0};  // ThoughtState::None
  std::atomic<std::uint8_t> thoughtSeq_{0};
  std::atomic<std::uint8_t> kind_{0};  // naechster Request: 0=health, 1=thought
  char thought_[27] = {0};  // max. 26 Zeichen (passt auf den Screen) + NUL
  void* task_ = nullptr;    // TaskHandle_t (Header bleibt POD-only)
};

}  // namespace pc
