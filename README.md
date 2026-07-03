# 🤖 Pocket Charlie

Ein kleiner, charmanter **Desktop-Begleiter** auf Basis des **M5Stack CoreS3**.
Fokus: Persönlichkeit, Display, Interaktion — später KI-Anbindung.
(Inspiriert von StackChan, aber kein Nachbau.)

> **MVP-Regel:** Kein Feature wird umgesetzt, bevor Display, Touch, WLAN und
> eine einfache Charlie-Gesichtsanzeige stabil funktionieren.

---

## Hardware & Toolchain

| Thema        | Wahl                                   | Warum                                              |
|--------------|----------------------------------------|----------------------------------------------------|
| Board        | M5Stack CoreS3 (ESP32-S3, 320×240)     | Zielgerät des Projekts                             |
| Build-System | **PlatformIO**                         | Reproduzierbar, Library-Management, CI-fähig       |
| Framework    | Arduino (auf ESP-IDF / FreeRTOS)       | Schnelle Iteration, beste M5-Unterstützung         |
| HAL / Grafik | **M5Unified** (+ M5GFX)                | Offizieller, einheitlicher M5Stack-Stack           |

---

## Projektstruktur

```
Pocket Charlie/
├── platformio.ini        # Build-Konfiguration (Board, Libs, Flags)
├── lib/                  # Eigene, modulare Bibliotheken (je 1 Verantwortung)
│   ├── PcConfig/         #   -> Projektweite Konstanten (header-only Modul)
│   │   └── PcConfig.h
│   ├── App/              #   -> Orchestrator: besitzt Subsysteme, steuert Loop
│   │   ├── App.h
│   │   └── App.cpp
│   └── Display/          #   -> Bildschirmausgabe (kapselt M5.Display / M5GFX)
│       ├── Display.h
│       └── Display.cpp
└── src/
    └── main.cpp          # Dünner Einstiegspunkt (setup/loop -> App)
```

### Warum `lib/`-Module?

Jedes Verzeichnis in `lib/` ist eine eigenständige Bibliothek. PlatformIO findet
und kompiliert sie automatisch und erzwingt damit **klare Grenzen** zwischen den
Bausteinen — genau das, was das Projekt beim Wachsen stabil hält.

> **Gelernt (PlatformIO-Eigenheit):** Das `include/`-Verzeichnis ist nur für
> `src/` sichtbar, **nicht** für `lib/`-Module. Da unser gesamter Code in
> `lib/`-Modulen lebt, ist auch die gemeinsame Konfiguration ein eigenes
> (header-only) Modul: `lib/PcConfig/`.

**Erweiterungspunkte (nach Roadmap):**

| Neuer Baustein | Ort                | Kommt in Phase             |
|----------------|--------------------|----------------------------|
| Touch/Buttons  | `lib/Input/`       | Phase 0/1 (Interaktion)    |
| Gesicht/Anim.  | `lib/Face/`        | Phase 1 (Charlie lebt)     |
| WLAN           | `lib/WifiService/` | Phase 0/3                  |
| KI-Backend     | `lib/AiService/`   | Phase 3 (Charlie denkt)    |
| Charakter/State| `lib/Persona/`     | Phase 1–2                  |

---

## Setup & Build

**1. PlatformIO installieren** (einmalig) — empfohlen als VS-Code-Erweiterung
„PlatformIO IDE", oder als CLI:

```bash
pip install platformio
```

**2. Kompilieren** (kein Gerät nötig):

```bash
pio run
```

**3. Auf den CoreS3 flashen** (per USB-C verbunden):

```bash
pio run --target upload
```

**4. Serielle Ausgabe ansehen:**

```bash
pio device monitor
```

> **Troubleshooting:** Erscheint kein serieller Log, aber das Display
> funktioniert, liegt es meist an der USB-CDC-Einstellung des CoreS3.
> Dann in `platformio.ini` `-DARDUINO_USB_CDC_ON_BOOT=1` als Build-Flag
> ergänzen (abhängig vom genutzten USB-Port).

---

## Roadmap (Kurzfassung)

- **Phase 0 – Hardware-Basis** ← *wir sind hier* (Display-Hello-World ✅)
- Phase 1 – Charlie lebt visuell (Gesicht, Emotionen, Bootscreen)
- Phase 2 – Charlie reagiert (lokale Interaktion)
- Phase 3 – Charlie denkt (KI-Backend; **keine API-Keys in der Firmware**)
- Phase 4 – Charlie spricht (Sprachassistenz)

Details, Backlog & Entscheidungen: **Notion-Workspace „Pocket Charlie HQ"**.
