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
//  (mbedtls, Bluetooth ...). Ein projekt-spezifisches Praefix (Pc = Pocket
//  Charlie) verhindert Namenskollisionen auf case-insensitiven Dateisystemen.
//
//  Bewusste Design-Entscheidung:
//  Diese Datei haengt NICHT von M5Unified/M5GFX ab -> die Konfiguration bleibt
//  von der konkreten Display-Library entkoppelt. Farben liegen als rohe
//  RGB565-Werte vor.
// ============================================================================

#include <cstdint>

namespace pc {
namespace config {

// --- Projekt-Metadaten ---------------------------------------------------
constexpr const char* kAppName    = "Pocket Charlie";
constexpr const char* kAppVersion = "0.2.0";  // Sprint 1 Release (Brownie)

// --- Boot-Screen (Sprint 1) ----------------------------------------------
constexpr const char*    kBootHint     = "Charlie wacht auf ...";
constexpr std::uint32_t  kBootScreenMs = 1200;  // Splash-Dauer vor dem Gesicht

// --- Display -------------------------------------------------------------
constexpr std::uint8_t kBrightness = 128;  // 0..255

// --- Eingabe: Touch-Buttons (CoreS3) -------------------------------------
// Der CoreS3 hat keine physischen A/B/C-Tasten. M5Unified bildet sie ueber ein
// Touch-Band am unteren Bildschirmrand ab, dessen Hoehe standardmaessig 0 ist
// (=> A/B/C inaktiv). Wir setzen eine Bandhoehe, damit A/B/C nutzbar sind.
constexpr std::uint16_t kTouchButtonHeight = 40;  // Pixel am unteren Rand

// --- Farben im RGB565-Format ---------------------------------------------
// M5GFX nutzt 16-Bit-Farben: 5 Bit Rot, 6 Bit Gruen, 5 Bit Blau.
constexpr std::uint16_t kColorBackground = 0x0000;  // Schwarz
constexpr std::uint16_t kColorText       = 0xFFFF;  // Weiss
constexpr std::uint16_t kColorAccent     = 0x07FF;  // Cyan (Charlie-Akzent)

// --- Game-Loop / Animation -----------------------------------------------
// Ziel-Bildrate der nicht-blockierenden Hauptschleife.
constexpr std::uint32_t kFrameIntervalMs = 33;  // ~30 FPS

}  // namespace config
}  // namespace pc
