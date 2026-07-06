# 🤖 Pocket Charlie

Ein kleiner, charmanter **Desktop-Begleiter** auf Basis des **M5Stack CoreS3**.
Fokus: Persönlichkeit, Display, Interaktion — später KI-Anbindung.
(Inspiriert von StackChan, aber kein Nachbau.)

**Charakter:** Charlies Grundpersönlichkeit ist in
[docs/PERSONALITY.md](docs/PERSONALITY.md) beschrieben — Leitsatz:
*„Ein Blick sagt mehr als ein Absatz."*

**Status:**
- ✅ **Sprint 0 – Hardware-Basis** · **v0.1.0 – Apple Pie 🥧** („Hello Pocket Charlie").
- ✅ **Sprint 1 – Charlie lebt visuell** · **v0.2.0 – Brownie 🍫** (Gesicht, Idle, Touch/Buttons).
- ✅ **Sprint 2 – Lokale Persönlichkeit** · **v0.3.0 – Cheesecake 🍰** (Emotion Engine v1:
  Neutral/Happy/Tired/Sleeping/Thoughtful/Annoyed — auf Hardware verifiziert).
- ➡️ Als Nächstes: **Sprint 3 – Charlie denkt** (KI-Backend).

> **MVP-Regel:** Kein Feature wird umgesetzt, bevor Display, Touch, WLAN und
> eine einfache Charlie-Gesichtsanzeige stabil funktionieren.

---

## Hardware & Toolchain

| Thema        | Wahl                                   | Warum                                        |
|--------------|----------------------------------------|----------------------------------------------|
| Board        | M5Stack CoreS3 (ESP32-S3, 320×240)     | Zielgerät des Projekts                       |
| Build-System | **PlatformIO**                         | Reproduzierbar, Library-Management, CI-fähig |
| Framework    | Arduino (auf ESP-IDF / FreeRTOS)       | Schnelle Iteration, beste M5-Unterstützung   |
| HAL / Grafik | **M5Unified** 0.2.17 (+ M5GFX 0.2.24)  | Offizieller, einheitlicher M5Stack-Stack     |
| Platform     | espressif32 7.0.1                       | Vom Resolver bestätigt (siehe CHANGELOG)     |

---

## Projektstruktur

```
Pocket Charlie/
├── platformio.ini        # Build-Konfiguration (Board, Libs, Flags)
├── lib/                  # Eigene, modulare Bibliotheken (je 1 Verantwortung)
│   ├── PcConfig/         #   -> Projektweite Konstanten (header-only Modul)
│   │   └── PcConfig.h
│   ├── App/              #   -> Orchestrator: besitzt Subsysteme, steuert Loop
│   │   ├── App.h / App.cpp
│   ├── Display/          #   -> Screen-Grundfunktionen + Boot-Screen
│   │   ├── Display.h / Display.cpp
│   ├── Input/            #   -> Touchscreen + Buttons (sauberer Snapshot)
│   │   ├── Input.h / Input.cpp
│   └── Face/             #   -> Charlies animiertes Gesicht (Canvas/PSRAM)
│       ├── Face.h / Face.cpp
├── src/
│   └── main.cpp          # Dünner Einstiegspunkt (setup/loop -> App)
├── docs/
│   └── LESSONS_LEARNED.md
├── README.md
└── CHANGELOG.md
```

### Architektur-Prinzipien

- **Composition Root:** `main.cpp` erzeugt nur die `App` und delegiert
  `setup()`/`loop()`. Die gesamte Logik lebt in Modulen.
- **Ein Modul = eine Verantwortung:** `Display` (Screen), `Input` (Eingabe),
  `Face` (Charlie-Rendering), `App` (Orchestrierung). Neue Fähigkeiten kommen
  als neues `lib/`-Modul dazu — `main.cpp` bleibt unverändert.
- **update() ⟂ render():** Animationslogik (zeitbasiert) ist von der
  Zeichenlogik getrennt → deterministisch und später testbar.
- **Nicht-blockierende Game-Loop:** Eingaben werden jeden Durchlauf gepollt,
  gerendert wird mit fester Bildrate (~30 FPS). Kein `delay()`-Ballett.

> **PlatformIO-Eigenheit (gelernt):** Das `include/`-Verzeichnis ist nur für
> `src/` sichtbar, **nicht** für `lib/`-Module. Deshalb ist auch die gemeinsame
> Konfiguration ein eigenes Modul: `lib/PcConfig/`.

**Erweiterungspunkte (nach Roadmap):**

| Baustein        | Ort                | Status / Phase              |
|-----------------|--------------------|-----------------------------|
| Eingabe         | `lib/Input/`       | ✅ Sprint 1                  |
| Gesicht/Animation | `lib/Face/`      | ✅ Sprint 1 (Idle)          |
| Emotionen/State | `lib/Persona/`     | Phase 1–2 (geplant)         |
| WLAN            | `lib/WifiService/` | Phase 0/3 (geplant)         |
| KI-Backend      | `lib/AiService/`   | Phase 3 (geplant)           |

---

## Build & Flash

PlatformIO liegt bei uns unter `%USERPROFILE%\.platformio\penv\Scripts\pio.exe`
(nicht im PATH — siehe LESSONS_LEARNED).

```bash
# Kompilieren (kein Gerät nötig):
pio run

# Auf den CoreS3 flashen (per USB-C-DATENkabel verbunden):
pio run --target upload

# Serielle Ausgabe ansehen:
pio device monitor
```

> **Flash-Troubleshooting** (aus der Praxis, Details in `docs/LESSONS_LEARNED.md`):
> - **Kein COM-Port / `Write Timeout`?** → echtes **Datenkabel** verwenden
>   (kein reines Ladekabel).
> - **Falscher Port?** → nicht die Bluetooth-Ports (COM3/COM4) nehmen; der
>   CoreS3 erscheint als eigener Port (bei uns COM5).
> - **Kein serieller Log, aber Display geht?** → ggf.
>   `-DARDUINO_USB_CDC_ON_BOOT=1` als Build-Flag ergänzen.

---

## Sprint 1 – Ziele & Stand

1. **Touchscreen testen** — ✅ `Input`-Modul liest Touch, Koordinaten werden
   geloggt und steuern Charlies Blick.
2. **Buttons testen** — ✅ BtnA/B/C (CoreS3: Touch-Zonen unten) + PWR werden
   geloggt und lösen Reaktionen aus.
3. **Erste kleine Idle-Animation** — ✅ Blinzeln, Augenbewegung, „Atmen",
   loop-fähig und flicker-frei.
4. **Charlie wirkt „lebendig"** — ✅ zufälliges Blinzeln + wandernder Blick +
   Reaktion auf Berührung.

*Nächster Feinschliff:* Grundpersönlichkeit ist in v0.1 definiert
([docs/PERSONALITY.md](docs/PERSONALITY.md)); als nächster möglicher Schritt ist
dort ein Emotions-System skizziert. Sprint 1 wird dafür nicht erweitert.

---

## Roadmap (Kurzfassung)

- **Phase 0 – Hardware-Basis** ✅
- **Phase 1 – Charlie lebt visuell** ✅ (Gesicht, Idle, Bootscreen; Emotionen folgen)
- Phase 2 – Charlie reagiert (lokale Interaktion, Menülogik)
- Phase 3 – Charlie denkt (KI-Backend; **keine API-Keys in der Firmware**)
- Phase 4 – Charlie spricht (Sprachassistenz)

Backlog, Sprints & Entscheidungen: **Notion-Workspace „Pocket Charlie HQ"**.

## Dokumentation

- [CHANGELOG.md](CHANGELOG.md) — Versionen & Änderungen
- [docs/PERSONALITY.md](docs/PERSONALITY.md) — Charlies Grundpersönlichkeit (Charakter, Tonfall, Emotionen)
- [docs/TEST_CHECKLIST.md](docs/TEST_CHECKLIST.md) — manuelle Hardware-Verifikation
- [docs/LESSONS_LEARNED.md](docs/LESSONS_LEARNED.md) — Stolpersteine & Lösungen
- [docs/SPRINT_2_PLAN.md](docs/SPRINT_2_PLAN.md) — Plan für **Sprint 2 – Lokale Persönlichkeit** (Next)
