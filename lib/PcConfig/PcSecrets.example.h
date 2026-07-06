#pragma once
// ============================================================================
//  PcSecrets.example.h - VORLAGE fuer lokale Zugangsdaten (Sprint 4)
//
//  Setup:
//    1) Diese Datei kopieren nach:  lib/PcConfig/PcSecrets.h
//    2) In der Kopie echte Werte eintragen.
//
//  WICHTIG:
//  - PcSecrets.h ist gitignored und darf NIEMALS committed werden.
//  - Diese Vorlage (PcSecrets.example.h) enthaelt nur leere Platzhalter.
//  - Ohne PcSecrets.h baut die Firmware trotzdem: WLAN bleibt dann aus und
//    Charlie laeuft rein lokal (local-first, Offline = normaler Zustand).
//  - KEINE API-Keys hier - die Firmware spricht nur die lokale Bridge an.
//
//  Warum lib/PcConfig/ statt include/? Das include/-Verzeichnis ist fuer
//  lib/-Module unsichtbar (PlatformIO-Eigenheit, siehe README/LESSONS_LEARNED).
// ============================================================================

namespace pc {
namespace secrets {

constexpr const char* kWifiSsid  = "";  // z. B. "MeinHeimnetz"
constexpr const char* kWifiPass  = "";  // z. B. "sehr-geheim"

// Basis-URL der lokalen Bridge (Sprint 4, Einheit 3),
// z. B. "http://192.168.1.20:8787" - Rechner im selben Netz, kein Internet-Host.
constexpr const char* kBridgeUrl = "";

}  // namespace secrets
}  // namespace pc
