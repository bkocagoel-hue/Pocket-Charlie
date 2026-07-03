# Changelog

Alle nennenswerten Änderungen an Pocket Charlie werden hier dokumentiert.

Das Format orientiert sich an [Keep a Changelog](https://keepachangelog.com/de/1.1.0/),
das Projekt folgt [Semantic Versioning](https://semver.org/lang/de/).

---

## [Unreleased] – Sprint 1: Charlie lebt visuell

### Added
- **`Input`-Modul** – kapselt Touchscreen und Buttons (BtnA/B/C, PWR) als
  sauberen Per-Frame-Snapshot.
- **`Face`-Modul** – Charlies animiertes Gesicht (Augen + Mund), flicker-frei
  gerendert über einen Off-Screen-Puffer (`M5Canvas` im PSRAM).
- **Idle-Animation** – zufälliges Blinzeln, sanfte Augenbewegung (Gaze) und
  leichtes „Atmen" (vertikales Wippen). → Charlie wirkt „lebendig".
- **Touch-/Button-Reaktionen** – Touch: Blick zum Berührungspunkt + Blinzeln;
  Buttons A/B/C: Blick links / Mitte / rechts (drei unterscheidbare Reaktionen).
- **Boot-Screen** mit Name + Bootnachricht (ersetzt den Hello-World-Screen).
- **Nicht-blockierende Game-Loop** mit fester Bildrate (~30 FPS).

### Changed
- `Display`-Modul: `showGreeting()` → `showBootScreen()` (screen-weite
  Grundfunktionen; das Gesicht rendert jetzt das `Face`-Modul).
- Firmware-Version im Build auf `0.2.0-dev` angehoben.

### Fixed
- Buttons A/B/C auf dem CoreS3 aktiviert (`M5.setTouchButtonHeight()` in
  `setup()`). Ohne dieses Touch-Band wären A/B/C inaktiv (Default-Höhe 0).
  Hintergrund: [docs/LESSONS_LEARNED.md](docs/LESSONS_LEARNED.md) #7.

### Notes
- Blinzelrate, Blick-Energie usw. sind bewusst als Parameter (Konstanten in
  `Face.cpp`) ausgelegt und werden an die **Grundpersönlichkeit** (Notion)
  angepasst, sobald diese definiert ist.

---

## [0.1.0] – 2026-07-03 – Sprint 0: Hardware-Basis

Erster Meilenstein: lauffähige Firmware auf echter Hardware.

### Added
- PlatformIO-Projekt für den **M5Stack CoreS3** (Arduino-Framework).
- Integration von **M5Unified 0.2.17 / M5GFX 0.2.24** (Platform espressif32 7.0.1).
- Modulare Architektur: dünner `main.cpp`-Einstiegspunkt + `App`-Orchestrator
  + `Display`-Modul + `PcConfig`-Modul (alle als `lib/`-Module).
- **„Hello Pocket Charlie"** wird zuverlässig auf dem Display angezeigt.
- README mit Architektur- und Build-Dokumentation.

### Fixed
- Header-Namenskollision: eigenes `Config.h` kollidierte auf Windows
  (case-insensitiv) mit Framework-Headern → umbenannt zu `PcConfig.h`.
- `include/` ist für `lib/`-Module unsichtbar → gemeinsame Konfiguration als
  eigenes `lib/PcConfig/`-Modul organisiert.

### Verified
- Erfolgreich kompiliert **und auf Hardware geflasht**; Display zeigt Text,
  Version 0.1.0 veröffentlicht.
- Lessons Learned: siehe [docs/LESSONS_LEARNED.md](docs/LESSONS_LEARNED.md).

---

_Vergleichslinks (Unreleased/0.1.0) werden ergänzt, sobald das Repository
online (z. B. auf GitHub) verfügbar ist._
