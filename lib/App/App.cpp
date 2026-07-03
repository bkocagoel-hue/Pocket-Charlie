#include "App.h"

#include <Arduino.h>
#include <M5Unified.h>

#include "PcConfig.h"

namespace pc {

void App::setup() {
  // 1) M5Unified initialisieren. EIN Aufruf konfiguriert Display, Power
  //    (AXP2101), Touch und I2C fuer den CoreS3.
  auto cfg = M5.config();
  M5.begin(cfg);

  // 2) Serielle Ausgabe fuer Entwicklungs-Logs (via USB).
  Serial.begin(115200);
  Serial.printf("[%s] Boot v%s\n", config::kAppName, config::kAppVersion);

  // 3) Subsysteme hochfahren und ersten Screen zeichnen.
  display_.begin();
  display_.showGreeting();

  Serial.println("[App] setup() abgeschlossen.");
}

void App::loop() {
  // M5.update() liest Buttons, Touch etc. ein. Wir rufen es schon jetzt auf,
  // damit Input in Sprint 0/1 sofort zur Verfuegung steht.
  M5.update();

  // Aktuell ist der Screen statisch -> keine weitere Arbeit pro Tick.
  // Ab Sprint 1 kommt hier die Animations- und Zustandslogik hinein.

  delay(config::kLoopIntervalMs);
}

}  // namespace pc
