#pragma once
// ============================================================================
//  NetworkManager - optionale WLAN-Anbindung (Sprint 4, non-blocking)
//
//  Kleine Zustands-FSM: Disabled (keine Secrets) -> Connecting -> Online /
//  Offline. Der Verbindungsaufbau laeuft asynchron im WiFi-Stack; update()
//  pollt nur den Status - kein delay(), kein Blockieren der 30-FPS-Loop.
//  Begrenzte Auto-Retries mit Backoff (KEINE Endlosschleife); danach nur noch
//  manuell per retry() (BtnB im Online-Screen).
//
//  local-first: Fehlt WLAN oder faellt es aus, laeuft Charlie unveraendert
//  lokal weiter. Offline ist ein normaler Zustand, kein Fehler.
//  Bewusst KEIN WiFi-Include im Header (nur POD) - wie beim Input-Modul.
// ============================================================================

#include <cstdint>

namespace pc {

enum class NetState : std::uint8_t { Disabled = 0, Connecting, Online, Offline };

class NetworkManager {
 public:
  // Startet (falls Secrets vorhanden) den ersten Verbindungsversuch - kehrt
  // sofort zurueck, der Stack verbindet im Hintergrund.
  void begin();

  // Einmal pro Loop-Runde aufrufen; pollt Status, Timeout und Retries.
  void update(std::uint32_t nowMs);

  NetState state() const { return state_; }
  const char* stateName() const;  // "off" / "trying" / "online" / "offline"
  bool online() const { return state_ == NetState::Online; }

  // IP als Text (nur gueltig wenn online, sonst leer) - fuer den Online-Screen.
  const char* ip() const { return ip_; }

  // Einmal-Flanken (genau ein update()-Frame true) fuer Persona-Reaktionen (E4).
  bool justConnected() const { return justConnected_; }
  bool justDisconnected() const { return justDisconnected_; }

  // Manueller Neuversuch (BtnB im Online-Screen); startet den Zyklus neu.
  void retry();

 private:
  void startAttempt(std::uint32_t nowMs);
  void enterOnline();

  NetState state_ = NetState::Disabled;
  std::uint32_t attemptStartedAt_ = 0;
  std::uint32_t nextRetryAt_ = 0;  // 0 = kein Auto-Retry geplant
  std::uint8_t attempt_ = 0;       // Versuchszaehler innerhalb eines Zyklus
  bool justConnected_ = false;
  bool justDisconnected_ = false;
  char ip_[16] = {0};
};

}  // namespace pc
