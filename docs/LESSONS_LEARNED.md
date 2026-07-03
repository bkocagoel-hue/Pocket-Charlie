# Lessons Learned

Praktische Stolpersteine und ihre Lösungen – damit wir (und andere) sie nicht
zweimal erleben. Neueste zuerst.

---

## Sprint 1 – Visuelle Persönlichkeit

### 7. CoreS3: Buttons A/B/C sind standardmäßig inaktiv
**Symptom:** `M5.BtnA/B/C.wasPressed()` löst auf dem CoreS3 nie aus.
**Ursache:** M5Unified bildet A/B/C über ein Touch-Band am unteren Rand ab
(`raw.y >= 240 − Bandhöhe`). Die Bandhöhe ist per Default **0**
(`M5Unified.hpp:624`) und wird in `M5.begin()` nicht gesetzt. Anders als beim
Core2 (Touchglas reicht unter das Display) endet der CoreS3-Touch bei y ≈ 239 →
Bedingung nie erfüllt.
**Lösung:** bei Bedarf `M5.setTouchButtonHeight(<pixel>)` (oder
`setTouchButtonHeightByRatio`) in `setup()` aufrufen. `BtnPWR` (physisch) ist
davon unabhängig und funktioniert ohne Zutun. Details:
[TEST_CHECKLIST.md](TEST_CHECKLIST.md).

---

## Sprint 0 – Hardware-Basis & erster Flash

### 1. USB-C-Kabel ohne Datenleitungen
**Symptom:** Der CoreS3 bekam Strom (Display/LED an), wurde von Windows aber
**nicht** erkannt – kein COM-Port, `Write Timeout` beim Upload.
**Ursache:** reines Ladekabel ohne Datenadern (D+/D−).
**Lösung:** ein echtes USB-C-**Datenkabel** verwenden.
**Merksatz:** „lädt" ≠ „überträgt Daten".

### 2. Falscher COM-Port (Bluetooth statt ESP32)
**Symptom:** Upload greift auf den falschen Port zu oder schlägt fehl.
**Ursache:** Windows blendet Bluetooth-Ports (häufig COM3/COM4) mit ein.
**Lösung:** Der CoreS3 erscheint als eigener Port – bei uns **COM5**. Vor dem
Upload den richtigen Port im Geräte-Manager prüfen; bei Bedarf in
`platformio.ini` `upload_port`/`monitor_port` fest setzen.

### 3. PlatformIO-CLI nicht im PATH
**Symptom:** `pio` wird im Terminal nicht gefunden.
**Ursache:** Die Installation über die VS-Code-Extension legt die CLI unter
`%USERPROFILE%\.platformio\penv\Scripts\pio.exe` ab – nicht im globalen PATH.
**Lösung:** vollen Pfad nutzen oder das Scripts-Verzeichnis zum PATH hinzufügen.

### 4. Header-Namenskollision (`Config.h`)
**Symptom:** Compiler-Fehler `'config' has not been declared`, obwohl
`Config.h` existiert – ohne „Datei nicht gefunden".
**Ursache:** Das ESP32-Framework bringt eigene `config.h`-Dateien mit (mbedtls,
Bluetooth …). Auf case-insensitiven Dateisystemen (Windows) zog
`#include "Config.h"` versehentlich einen Framework-Header ein.
**Lösung:** projekt-spezifisches Präfix → `PcConfig.h`.

### 5. `include/` ist für `lib/`-Module unsichtbar
**Symptom:** `PcConfig.h: No such file or directory` aus einem `lib/`-Modul.
**Ursache:** PlatformIO legt das projektweite `include/` nur auf den
Include-Pfad von `src/`, nicht auf den der `lib/`-Module.
**Lösung:** gemeinsame Header als eigenes `lib/`-Modul organisieren
(hier: `lib/PcConfig/`).

### 6. „Erst grün bauen, dann pinnen"
**Erkenntnis:** Beim ersten Build die Library-Versionen bewusst **nicht**
gepinnt – der Resolver zog `espressif32 7.0.1` (statt der erwarteten 6.x).
Vorab-Pinning hätte den Build unnötig gebrochen.
**Regel:** neue Toolchain zuerst zum Laufen bringen, dann die real genutzten
Versionen für reproduzierbare Builds festschreiben.
