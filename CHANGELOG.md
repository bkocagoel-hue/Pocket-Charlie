# Changelog

Alle nennenswerten Änderungen an Pocket Charlie werden hier dokumentiert.

Das Format orientiert sich an [Keep a Changelog](https://keepachangelog.com/de/1.1.0/),
das Projekt folgt [Semantic Versioning](https://semver.org/lang/de/).

---

## [0.5.0] – 2026-07-06 – Éclair ⚡ · Sprint 4: Online Widgets v1

> Erster öffentlicher Release nach **Cheesecake v0.3.0**. **Donut (Sprint 3) war
> ein interner Meilenstein und ist in dieser Version enthalten** — es gab
> bewusst **kein** separates `v0.4.0`-Release.
> **Guardrail-Wechsel, aber local-first:** WLAN + lokale Bridge sind jetzt
> möglich; ohne beides läuft Charlie unverändert lokal. **Keine API-Keys in der
> Firmware, keine echte KI** — `/thought` ist bewusst statisch/mock.

### Added — Online (Sprint 4)
- **Optionale WLAN-Schicht** (`lib/Network/NetworkManager`) — non-blocking FSM
  (`off/trying/online/offline`), 15-s-Timeout, max. 3 Auto-Retries mit Backoff,
  manueller Retry per BtnB. Secrets-Handling: `PcSecrets.example.h` (Vorlage im
  Repo) + gitignorte `PcSecrets.h`; ohne Secrets rein lokaler Modus.
- **Online-Screen** — Navigation jetzt Face → Clock → Mood → **Online** → Info;
  zeigt WiFi-Status, Bridge-Status und Online-Thought; `BtnB` = Retry/Ping/Thought.
- **Lokale Bridge** (`backend/pocket-charlie-bridge/`, Python-Stdlib, 0 Deps):
  `GET /health`, `GET /thought` (kurze statische Sätze); README mit Setup.
- **OnlineClient** (`lib/Online/`) — HTTP im FreeRTOS-Task auf Core 0, 2-s-
  Timeout, kein UI-Freeze; charmante Fallbacks (`offline / still me`).

### Added — Persönlichkeit & Ausdruck (Sprint 4)
- **Emotion Expansion v2** — Curious/Confused/Excited/Sad/WakingUp
  freigeschaltet (Namen, Trigger, Microcopy). Online-Ereignisse als kurze
  emotionale Momente: verbindet → Excited, Fehler → Confused, Fehlerserie →
  dezent Sad, Thought → Happy; sanftes Aufwachen (Sleeping → WakingUp).
- **Expression Pack v1** — Expression-Varianten je Emotion (Happy×3,
  Thoughtful/Annoyed/Curious/Tired×2), Onset-Akzent beim Emotionswechsel,
  seltene Neutral-Micro-Expressions; Sleeping bleibt bewusst ruhig.

### Added — enthaltener interner Meilenstein: Donut (Sprint 3)
- **`InputContext`** (`lib/Interaction/`) – klassifiziert den Eingabe-Snapshot zu
  lokalen Intents (SingleTap/RapidTap, BtnA/B/C, PWR).
- **Button-Menüführung + lokale Widgets** – Screens Face · Clock/Uptime · Mood ·
  Info (umlaufend); `BtnA`/`BtnC` navigieren, `BtnB` = kontextuelle Aktion.
  Touch bleibt emotionale Interaktion.
- **Mood light** – dezente, langfristige Grundstimmung (High/Neutral/Low) mit
  Hysterese; färbt die Idle-Microcopy.
- **Microcopy / Gedankenblasen** – sehr kurze, seltene lokale Sprüche.
- **Augenbrauen-Expression** – kleine, weiche, emotionsabhängige Pixel-Augenbrauen
  (weich interpoliert, kein Flackern), die Emotionen lesbarer machen.

### Changed
- Firmware-Version auf `0.5.0` (Release-Stand); Codename `Eclair` (Display-ASCII).
- Schnurrbart bleibt deaktiviert / **optionales späteres Feature / Easter Egg /
  Skin** (`config::kEnableMoustache`, Default `false`; Architektur bleibt erhalten).

### Verified
- Sprint-4-Einheiten E2 (WLAN), E3 (Bridge `/health`), E4A (Thought), E4B
  (Emotionen), E4C (Expressions) einzeln auf Hardware verifiziert; **voller
  Éclair-Gesamtdurchlauf bestanden** (M5Stack CoreS3, 2026-07-06): lokaler +
  Online-Modus, Fallback/Recovery, 11/11 Emotionen, Expression Pack, kein
  Freeze/Reboot/Flackern. Donut-Funktionen (Sprint 3) ebenfalls verifiziert.
  Details: [docs/TEST_CHECKLIST.md](docs/TEST_CHECKLIST.md).

### Out of Scope (bewusst)
- Keine echte KI, kein Wetter, kein NTP, kein Audio, keine Cloud/Accounts,
  keine API-Keys/Secrets im Repo.

---

## [0.3.0] – 2026-07-06 – Cheesecake 🍰 · Sprint 2: Lokale Persönlichkeit

### Added
- **Emotion Engine v1** – `Persona`-Modul wählt lokal (ohne Netz/KI) die Emotion:
  Neutral, Happy, Tired, Sleeping, Thoughtful, Annoyed (weitere im Enum
  vorbereitet). Datengetriebenes, weich interpoliertes Face-Rendering.
- **Lokale Trigger:** Boot/Touch → Happy, schnelles Dauer-Tippen → Annoyed,
  Inaktivität → Tired → Sleeping, BtnB → Thoughtful; sauberes Aufwachen bei Input.
- **Zustands-Indikatoren:** animiertes `zZz` im Schlaf, `?` bei Thoughtful.
- Emotionswechsel-Log auf Serial (nur bei tatsächlichem Wechsel).

### Changed
- **Visuelle Identität:** violette Augen mit **weißen Pupillen**; echtes
  Lid-Schließen (geschlossene Augen als klare Linie); Annoyed mit sichtbaren
  Pupillen. Firmware-Version auf `0.3.0`.
- Schnurrbart **vorerst deaktiviert** (`kEnableMoustache = false`); Zeichen-
  funktion/Architektur bleiben für ein späteres Pixel-Sprite erhalten.

### Verified
- Auf echter Hardware (M5Stack CoreS3): Tired (~20 s), Sleeping (~40 s, kein
  Blinzeln, `zZz`), Annoyed, Thoughtful, sauberes Aufwachen — bestätigt. Kein
  Flackern/Freeze/Reboot. Details: [docs/TEST_CHECKLIST.md](docs/TEST_CHECKLIST.md).

### Out of Scope (bewusst)
- Kein WLAN, keine KI, kein Audio, kein Menüsystem.

---

## [0.2.0] – 2026-07-04 – Brownie 🍫 · Sprint 1: Charlie lebt visuell

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
- Firmware-Version auf `0.2.0` gesetzt (Release-Stand).

### Fixed
- Buttons A/B/C auf dem CoreS3 aktiviert (`M5.setTouchButtonHeight()` in
  `setup()`). Ohne dieses Touch-Band wären A/B/C inaktiv (Default-Höhe 0).
  Hintergrund: [docs/LESSONS_LEARNED.md](docs/LESSONS_LEARNED.md) #7.

### Verified
- Auf echter Hardware (M5Stack CoreS3, 2026-07-04): Touch, Blick folgt
  Touchposition, Blinzeln, Buttons A/B/C — bestätigt. Keine Freezes, keine
  Reboots, kein Flackern. Details: [docs/TEST_CHECKLIST.md](docs/TEST_CHECKLIST.md).

### Notes
- **Grundpersönlichkeit v0.1** ist definiert und dokumentiert
  ([docs/PERSONALITY.md](docs/PERSONALITY.md)); die Idle-Animation ist bereits
  charakter-parametrisiert. Ein Emotions-System ist als nächster Architektur-
  Schritt skizziert, aber **nicht** Teil dieses Releases.

---

## [0.1.0] – 2026-07-03 – Apple Pie 🥧 · Sprint 0: Hardware-Basis

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

[0.5.0]: https://github.com/bkocagoel-hue/Pocket-Charlie/compare/v0.3.0...v0.5.0
[0.3.0]: https://github.com/bkocagoel-hue/Pocket-Charlie/compare/v0.2.0...v0.3.0
[0.2.0]: https://github.com/bkocagoel-hue/Pocket-Charlie/compare/v0.1.0...v0.2.0
[0.1.0]: https://github.com/bkocagoel-hue/Pocket-Charlie/releases/tag/v0.1.0
