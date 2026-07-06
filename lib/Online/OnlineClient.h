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
//  im Serial-Log). Kommunikation Loop <-> Task ueber ein einzelnes atomares
//  Status-Byte; keine geteilten Puffer noetig.
//
//  local-first: keine API-Keys, keine KI, keine Cloud - nur die lokale Bridge.
// ============================================================================

#include <atomic>
#include <cstdint>

namespace pc {

enum class BridgeState : std::uint8_t { Disabled = 0, Idle, Checking, Ok, Down };

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
  // Ignoriert, wenn Disabled oder bereits ein Ping laeuft.
  void requestPing();

  // Ergebnis verwerfen (z. B. nach WLAN-Verlust) -> zurueck auf Idle.
  void reset();

 private:
  static void taskEntry(void* self);
  void taskLoop();
  void doPing();
  void setState(BridgeState s) {
    state_.store(static_cast<std::uint8_t>(s), std::memory_order_relaxed);
  }

  std::atomic<std::uint8_t> state_{0};  // BridgeState::Disabled
  void* task_ = nullptr;                // TaskHandle_t (Header bleibt POD-only)
};

}  // namespace pc
