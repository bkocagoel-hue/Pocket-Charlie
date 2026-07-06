#include "OnlineClient.h"

#include <Arduino.h>
#include <HTTPClient.h>

#include <cstdio>

// PcConfig.h verankert lib/PcConfig im Include-Pfad dieses Moduls, damit
// __has_include("PcSecrets.h") die (gitignorte) Secrets-Datei findet -
// gleiche Technik wie im NetworkManager.
#include "PcConfig.h"

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
constexpr std::uint32_t kHttpTimeoutMs = 2000;  // kurz: kein UI-Gefuehl von Haengen
constexpr std::uint32_t kTaskStackBytes = 8192;
constexpr UBaseType_t   kTaskPriority   = 1;    // niedrig; Rendering hat Vorrang
constexpr BaseType_t    kTaskCore       = 0;    // Arduino-Loop laeuft auf Core 1

bool hasBridgeUrl() {
  return secrets::kBridgeUrl != nullptr && secrets::kBridgeUrl[0] != '\0';
}
}  // namespace

void OnlineClient::begin() {
  if (!hasBridgeUrl()) {
    setState(BridgeState::Disabled);
    Serial.println("[Bridge] keine Bridge-URL in PcSecrets.h -> deaktiviert.");
    return;
  }
  setState(BridgeState::Idle);
  TaskHandle_t handle = nullptr;
  const BaseType_t okCreated =
      xTaskCreatePinnedToCore(&OnlineClient::taskEntry, "bridge",
                              kTaskStackBytes, this, kTaskPriority, &handle,
                              kTaskCore);
  if (okCreated == pdPASS) {
    task_ = handle;
    Serial.printf("[Bridge] bereit (%s, Ping via BtnB im Online-Screen).\n",
                  secrets::kBridgeUrl);
  } else {
    task_ = nullptr;
    setState(BridgeState::Disabled);
    Serial.println("[Bridge] FEHLER: Task konnte nicht erstellt werden.");
  }
}

void OnlineClient::requestPing() {
  if (task_ == nullptr) return;
  const BridgeState s = state();
  if (s == BridgeState::Disabled || s == BridgeState::Checking) return;
  setState(BridgeState::Checking);
  xTaskNotifyGive(static_cast<TaskHandle_t>(task_));
}

void OnlineClient::reset() {
  // Nur ein abgeschlossenes Ergebnis verwerfen; einen laufenden Ping nicht
  // anfassen (der Task schreibt sein Ergebnis gleich selbst).
  const BridgeState s = state();
  if (s == BridgeState::Ok || s == BridgeState::Down) {
    setState(BridgeState::Idle);
  }
}

void OnlineClient::taskEntry(void* self) {
  static_cast<OnlineClient*>(self)->taskLoop();
}

void OnlineClient::taskLoop() {
  for (;;) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);  // schlaeft bis requestPing()
    doPing();
  }
}

void OnlineClient::doPing() {
  char url[96];
  std::snprintf(url, sizeof(url), "%s/health", secrets::kBridgeUrl);

  bool ok = false;
  HTTPClient http;
  http.setConnectTimeout(kHttpTimeoutMs);
  http.setTimeout(kHttpTimeoutMs);
  if (http.begin(url)) {
    const int code = http.GET();
    ok = (code == HTTP_CODE_OK);
    if (ok) {
      Serial.println("[Bridge] /health -> ok (200)");
    } else {
      // code < 0 = Verbindungsfehler/Timeout, sonst HTTP-Fehlercode.
      Serial.printf("[Bridge] /health -> down (%d: %s)\n", code,
                    code < 0 ? HTTPClient::errorToString(code).c_str()
                             : "HTTP-Fehler");
    }
    http.end();
  } else {
    Serial.println("[Bridge] /health -> down (URL ungueltig).");
  }

  setState(ok ? BridgeState::Ok : BridgeState::Down);
}

const char* OnlineClient::stateName() const {
  switch (state()) {
    case BridgeState::Disabled: return "no url";
    case BridgeState::Idle:     return "ready";
    case BridgeState::Checking: return "checking";
    case BridgeState::Ok:       return "ok";
    case BridgeState::Down:     return "down";
    default:                    return "?";
  }
}

}  // namespace pc
