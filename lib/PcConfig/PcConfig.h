#pragma once
// ============================================================================
//  PcConfig.h - zentrale, projektweite Konstanten (header-only Modul)
//
//  Warum liegt das in lib/PcConfig/ und nicht in include/?
//  In PlatformIO ist das projektweite include/-Verzeichnis nur fuer Dateien
//  in src/ sichtbar - NICHT fuer eigenstaendige lib/-Module. Da unser gesamter
//  Code als lib/-Modul organisiert ist, ist auch die gemeinsame Konfiguration
//  ein eigenes (header-only) Modul. So findet jeder Baustein sie zuverlaessig.
//
//  Warum "PcConfig.h" und nicht "Config.h"?
//  Das ESP32-Framework bringt selbst mehrere Header namens "config.h" mit
//  (mbedtls, Bluetooth ...). Auf case-insensitiven Dateisystemen (Windows)
//  wuerde ein generisches #include "Config.h" versehentlich einen dieser
//  Framework-Header einziehen. Ein projekt-spezifisches Praefix (Pc = Pocket
//  Charlie) verhindert solche Namenskollisionen zuverlaessig.
//
//  Bewusste Design-Entscheidung:
//  Diese Datei haengt NICHT von M5Unified/M5GFX ab. Dadurch bleibt die
//  Konfiguration von der konkreten Display-Library entkoppelt (saubere
//  Schichtung). Farben liegen daher als rohe RGB565-Werte vor.
// ============================================================================

#include <cstdint>

namespace pc {
namespace config {

// --- Projekt-Metadaten ---------------------------------------------------
constexpr const char* kAppName    = "Pocket Charlie";
constexpr const char* kAppVersion = "0.1.0";

// --- Hello-World-Screen (Sprint 0) ---------------------------------------
constexpr const char* kGreeting = "Hello Pocket Charlie";
constexpr const char* kSubtitle = "Sprint 0 - Hardware-Basis";

// --- Farben im RGB565-Format ---------------------------------------------
// M5GFX nutzt 16-Bit-Farben: 5 Bit Rot, 6 Bit Gruen, 5 Bit Blau.
// 0x0000 = Schwarz, 0xFFFF = Weiss, 0x07FF = Cyan.
constexpr std::uint16_t kColorBackground = 0x0000;  // Schwarz
constexpr std::uint16_t kColorText       = 0xFFFF;  // Weiss
constexpr std::uint16_t kColorAccent     = 0x07FF;  // Cyan (Charlie-Akzent)

// --- Loop-Timing ---------------------------------------------------------
// Wie lange der Haupt-Loop pro Durchlauf "schlaeft". Bewusst konservativ.
// Fuer Animationen (Sprint 1) erhoehen wir die Taktrate spaeter gezielt.
constexpr std::uint32_t kLoopIntervalMs = 50;  // ~20 Ticks/Sekunde

}  // namespace config
}  // namespace pc
