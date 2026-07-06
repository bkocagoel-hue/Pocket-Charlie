#include "NetworkManager.h"

#include <Arduino.h>
#include <WiFi.h>

// PcConfig.h wird hier (auch) eingebunden, damit lib/PcConfig sicher im
// Include-Pfad dieses Moduls liegt -> __has_include("PcSecrets.h") unten
// findet die (gitignorte) Secrets-Datei zuverlaessig.
#include "PcConfig.h"

// Echte Zugangsdaten liegen in lib/PcConfig/PcSecrets.h (gitignored, Vorlage:
// PcSecrets.example.h). Ohne die Datei baut die Firmware trotzdem - WLAN
// bleibt dann aus und Charlie laeuft rein lokal (local-first).
#if __has_include("PcSecrets.h")
#include "PcSecrets.h"
#else
namespace pc {
namespace secrets {
constexpr const char* kWifiSsid  = "";
constexpr const char* kWifiPass  = "";
constexpr const char* kBridgeUrl = "";
}  // namespace secrets
}  // namespace pc
#endif

namespace pc {
namespace {
// Timing (leicht tunbar): kurzer Timeout je Versuch, wenige Auto-Retries mit
// Backoff - danach ausschliesslich manueller retry() (keine Endlosschleife).
constexpr std::uint32_t kConnectTimeoutMs = 15000;
constexpr std::uint8_t  kMaxAttempts      = 3;
constexpr std::uint32_t kRetryDelayMs[]   = {8000, 30000};  // vor Versuch 2/3

bool hasCredentials() {
  return secrets::kWifiSsid != nullptr && secrets::kWifiSsid[0] != '\0';
}
}  // namespace

void NetworkManager::begin() {
  if (!hasCredentials()) {
    state_ = NetState::Disabled;
    Serial.println("[Net] keine PcSecrets.h/SSID -> WLAN aus (lokaler Modus).");
    return;
  }
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(false);  // Retries steuern wir selbst (begrenzt)
  attempt_ = 0;
  startAttempt(millis());
}

void NetworkManager::startAttempt(std::uint32_t nowMs) {
  ++attempt_;
  attemptStartedAt_ = nowMs;
  nextRetryAt_ = 0;
  state_ = NetState::Connecting;
  WiFi.disconnect();
  WiFi.begin(secrets::kWifiSsid, secrets::kWifiPass);
  Serial.printf("[Net] verbinde (Versuch %u/%u) ...\n",
                static_cast<unsigned>(attempt_),
                static_cast<unsigned>(kMaxAttempts));
}

void NetworkManager::enterOnline() {
  state_ = NetState::Online;
  justConnected_ = true;
  snprintf(ip_, sizeof(ip_), "%s", WiFi.localIP().toString().c_str());
  Serial.printf("[Net] online, IP %s\n", ip_);
}

void NetworkManager::update(std::uint32_t nowMs) {
  justConnected_ = false;
  justDisconnected_ = false;

  if (state_ == NetState::Disabled) return;

  const bool connected = (WiFi.status() == WL_CONNECTED);

  switch (state_) {
    case NetState::Connecting:
      if (connected) {
        enterOnline();
      } else if (nowMs - attemptStartedAt_ >= kConnectTimeoutMs) {
        state_ = NetState::Offline;
        if (attempt_ < kMaxAttempts) {
          nextRetryAt_ = nowMs + kRetryDelayMs[attempt_ - 1];
          Serial.printf("[Net] Timeout - Auto-Retry in %us.\n",
                        static_cast<unsigned>(kRetryDelayMs[attempt_ - 1] / 1000));
        } else {
          nextRetryAt_ = 0;
          Serial.println(
              "[Net] offline (Auto-Versuche aufgebraucht; BtnB = retry).");
        }
      }
      break;

    case NetState::Online:
      if (!connected) {
        state_ = NetState::Offline;
        justDisconnected_ = true;
        ip_[0] = '\0';
        attempt_ = 0;  // Verbindung war da -> neuer Zyklus lohnt sich
        nextRetryAt_ = nowMs + kRetryDelayMs[0];
        Serial.println("[Net] Verbindung verloren - Auto-Retry folgt.");
      }
      break;

    case NetState::Offline:
      if (connected) {
        enterOnline();  // Stack hat sich doch noch verbunden
      } else if (nextRetryAt_ != 0 && nowMs >= nextRetryAt_) {
        startAttempt(nowMs);
      }
      break;

    default:
      break;
  }
}

void NetworkManager::retry() {
  if (state_ == NetState::Offline) {
    attempt_ = 0;  // manueller Anstoss startet den vollen Zyklus neu
    startAttempt(millis());
  }
}

const char* NetworkManager::stateName() const {
  switch (state_) {
    case NetState::Disabled:   return "off";
    case NetState::Connecting: return "trying";
    case NetState::Online:     return "online";
    case NetState::Offline:    return "offline";
    default:                   return "?";
  }
}

}  // namespace pc
