#include "OnlineClient.h"

#include <Arduino.h>
#include <HTTPClient.h>

#include <cstdio>
#include <cstring>

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

// Minimales "JSON-Parsing": extrahiert den Wert der "text"-Property.
// Bewusst KEINE JSON-Bibliothek - die Bridge gehoert uns, das Format ist
// stabil und die Saetze enthalten keine Escapes/Anfuehrungszeichen.
bool extractText(const char* json, char* out, std::size_t outSize) {
  const char* p = strstr(json, "\"text\"");
  if (p == nullptr) return false;
  p = strchr(p + 6, ':');
  if (p == nullptr) return false;
  p = strchr(p, '"');
  if (p == nullptr) return false;
  ++p;
  std::size_t i = 0;
  while (*p != '\0' && *p != '"' && i + 1 < outSize) {
    out[i++] = *p++;  // laengere Texte werden hart gekuerzt (kein Crash)
  }
  out[i] = '\0';
  return i > 0;
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
  if (state() == BridgeState::Disabled || requestRunning()) return;
  // Ein neuer Health-Check macht ein altes Thought-Ergebnis hinfaellig.
  if (thoughtState() != ThoughtState::None) setThoughtState(ThoughtState::None);
  setState(BridgeState::Checking);
  kind_.store(0, std::memory_order_relaxed);
  xTaskNotifyGive(static_cast<TaskHandle_t>(task_));
}

void OnlineClient::requestThought() {
  if (task_ == nullptr) return;
  if (state() != BridgeState::Ok || requestRunning()) return;
  setThoughtState(ThoughtState::Fetching);  // ab jetzt liest niemand thought_
  kind_.store(1, std::memory_order_relaxed);
  xTaskNotifyGive(static_cast<TaskHandle_t>(task_));
}

void OnlineClient::reset() {
  // Nur abgeschlossene Ergebnisse verwerfen; einen laufenden Request nicht
  // anfassen (der Task schreibt sein Ergebnis gleich selbst).
  const BridgeState s = state();
  if (s == BridgeState::Ok || s == BridgeState::Down) {
    setState(BridgeState::Idle);
  }
  const ThoughtState ts = thoughtState();
  if (ts == ThoughtState::Ok || ts == ThoughtState::Failed) {
    setThoughtState(ThoughtState::None);
  }
}

void OnlineClient::taskEntry(void* self) {
  static_cast<OnlineClient*>(self)->taskLoop();
}

void OnlineClient::taskLoop() {
  for (;;) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);  // schlaeft bis request...()
    if (kind_.load(std::memory_order_relaxed) == 1) {
      doThought();
    } else {
      doPing();
    }
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

void OnlineClient::doThought() {
  char url[96];
  std::snprintf(url, sizeof(url), "%s/thought", secrets::kBridgeUrl);

  bool ok = false;
  HTTPClient http;
  http.setConnectTimeout(kHttpTimeoutMs);
  http.setTimeout(kHttpTimeoutMs);
  if (http.begin(url)) {
    const int code = http.GET();
    if (code == HTTP_CODE_OK) {
      // Antwort ist winzig (< 100 B); Text extrahieren und hart kuerzen.
      const String payload = http.getString();
      ok = extractText(payload.c_str(), thought_, sizeof(thought_));
      if (ok) {
        Serial.printf("[Bridge] /thought -> ok: \"%s\"\n", thought_);
      } else {
        Serial.println("[Bridge] /thought -> Antwort ohne \"text\".");
      }
    } else {
      Serial.printf("[Bridge] /thought -> down (%d: %s)\n", code,
                    code < 0 ? HTTPClient::errorToString(code).c_str()
                             : "HTTP-Fehler");
    }
    http.end();
  } else {
    Serial.println("[Bridge] /thought -> down (URL ungueltig).");
  }

  if (ok) {
    thoughtSeq_.fetch_add(1, std::memory_order_relaxed);
    setThoughtState(ThoughtState::Ok);
  } else {
    // Ehrlich bleiben: Thought scheiterte -> Bridge gilt als down; der
    // naechste BtnB macht wieder einen /health-Ping.
    setThoughtState(ThoughtState::Failed);
    setState(BridgeState::Down);
  }
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
