# Test-Checkliste (manuelle Hardware-Verifikation)

Manuelle Tests auf echter Hardware (M5Stack CoreS3). Neueste zuerst.

Legende: ✅ bestanden · ⏳ offen · ⚠️ Einschränkung/Befund

---

## Sprint 1 – Charlie lebt visuell (v0.2.0-dev)

**Verifiziert am:** 2026-07-04 · **Gerät:** M5Stack CoreS3 (Upload-Port COM5)

| #  | Testfall                     | Methode                          | Ergebnis |
|----|------------------------------|----------------------------------|----------|
| 1  | Upload / Flash               | `pio run -t upload`              | ✅ erfolgreich |
| 2  | Sauberer Start               | Gerät einschalten                | ✅ bootet sauber |
| 3  | Gesicht wird angezeigt       | visuell                          | ✅ Augen + Mund sichtbar |
| 4  | Idle: Blinzeln               | visuell                          | ✅ Charlie blinzelt |
| 5  | Idle: kein Flackern          | visuell                          | ✅ flicker-frei |
| 6  | Touch erkannt                | Bildschirm antippen              | ✅ Blinzeln bei Tap |
| 7  | Blick folgt Touch            | mehrere Stellen antippen         | ✅ Blick folgt letzter Position |
| 8  | Serial: Touch-Down-Log       | Serial Monitor                   | ✅ Position wird geloggt |
| 9  | Serial: Touch-Release-Log    | Serial Monitor                   | ✅ Log beim Loslassen |
| 10 | Stabilität                   | ~2–3 min Laufzeit                | ✅ kein Freeze / Reboot |
| 11 | **Buttons A/B/C**            | unteres Band antippen            | ✅ A/B/C lösen aus (verifiziert 2026-07-04) |
| 12 | Button PWR (physisch)        | Power-Taste **kurz** drücken     | ⏳ offen – bitte testen (Log: `[Input] Button PWR (kurz)`) |

> Hinweis zu #12: Power-Taste nur **kurz** antippen. Langes Halten (~6 s) schaltet
> den CoreS3 hardwareseitig aus.

### ⚠️ Befund: Buttons A/B/C auf dem CoreS3

**Unser Code ist korrekt** – `M5.BtnA/B/C.wasPressed()` ist die richtige API.
A/B/C sind auf dem CoreS3 aber **standardmäßig inaktiv**.

Ursache (belegt aus dem installierten M5Unified-Quellcode):
- M5Unified bildet A/B/C über ein **Touch-Band am unteren Rand** ab. Ein Touch
  zählt nur als Button, wenn `raw.y >= 240 - Bandhöhe`
  (`M5Unified.cpp`, `update()`).
- Die **Bandhöhe ist per Default 0** (`_touch_button_height = 0`,
  `M5Unified.hpp:624`) und wird in `M5.begin()` **nicht** gesetzt.
- Der CoreS3-Touch endet bei y ≈ 239 → die Bedingung `raw.y >= 240` wird nie
  erfüllt. (Beim Core2 reicht das Touchglas unter das Display, dort klappt es.)

**Aktivierung (umgesetzt in v0.2.0-dev):** In `App::setup()` wird nach
`M5.begin()` ein Touch-Band gesetzt:
```cpp
M5.setTouchButtonHeight(config::kTouchButtonHeight);   // 40 px Band unten
```
**So testest du:** unteren Bildschirmrand antippen und den Serial Monitor
beobachten – die Logs (`Button A/B/C -> ...`) zeigen, welche Zone getroffen
wurde. Wegen der Display-Rotation kann die physische Lage der Zonen abweichen;
per Log am Gerät bestätigen, welche Stelle A/B/C auslöst.

**✅ Verifiziert am 2026-07-04:** A/B/C lösen am Gerät zuverlässig aus (Blick
links/Mitte/rechts, Blinzeln bei B). Die Zonen-Lage war in der Praxis brauchbar.

---

## Sprint 0 – Hardware-Basis (v0.1.0)

**Verifiziert am:** 2026-07-03 · **Gerät:** M5Stack CoreS3

| # | Testfall                | Methode              | Ergebnis |
|---|-------------------------|----------------------|----------|
| 1 | Build                   | `pio run`            | ✅ erfolgreich |
| 2 | Upload / Flash          | `pio run -t upload`  | ✅ erfolgreich |
| 3 | „Hello Pocket Charlie"  | visuell              | ✅ korrekt angezeigt |
| 4 | Version 0.1.0           | –                    | ✅ veröffentlicht |
