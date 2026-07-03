// ============================================================================
//  main.cpp - Einstiegspunkt (Arduino-Framework)
//
//  Bewusst minimal gehalten: Wir instanziieren die App EINMAL und delegieren
//  setup()/loop() an sie. So bleibt die Arduino-Konvention (setup/loop)
//  erhalten, waehrend die eigentliche Logik sauber in Modulen (lib/) liegt.
// ============================================================================

#include "App.h"

// Globale App-Instanz. Lebt fuer die gesamte Laufzeit des Geraets
// (statisch angelegt -> keine dynamische Speicherverwaltung noetig).
static pc::App app;

void setup() {
  app.setup();
}

void loop() {
  app.loop();
}
