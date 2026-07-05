# 📋 Sprint 2 – Lokale Persönlichkeit (Plan)

**Status:** angenommen · in Planung
**Versionsziel:** `v0.3.0-dev` → Release `v0.3.0 – Cheesecake 🍰`
**Grundlage:** die Emotionen aus [PERSONALITY.md](PERSONALITY.md)

> **Nur Planung.** Dieser Sprint ist noch nicht implementiert; es existiert
> bewusst noch kein Code (kein Emotion-Enum, kein WLAN/KI/Audio).

> **Emotions-Mapping** (Enum ↔ PERSONALITY.md): `Neutral`↔Neutral ·
> `Happy`↔Happy · `Tired`↔Müde · `Thoughtful`↔Nachdenklich · `Annoyed`↔Genervt.

## 1. Sprint-Ziel
Charlie wirkt glaubwürdiger, weil sein Gesicht **fünf klar unterscheidbare,
lokale Stimmungen** zeigt — ausgelöst rein durch Zeit, Inaktivität und
Touch-Muster, **ohne Netz, KI oder Audio**. Kurz: *„Charlies Gesicht bekommt
Stimmungen."*

## 2. Scope (In Scope)
- **Emotion-Modell** `enum class Emotion { Neutral, Happy, Tired, Thoughtful, Annoyed }`
  (der in PERSONALITY.md dokumentierte nächste Architektur-Schritt).
- **Pro Emotion ein Parametersatz** (Augenöffnung/-form, Blinzelrate, Gaze,
  Mundform) — abgeleitet aus der Emotions-Tabelle in PERSONALITY.md.
- **Lokale Auslöser** (`millis()`-basiert) — siehe „Trigger-Details".
- **Sanfte Übergänge** + automatischer **Rückfall auf Neutral** nach Timeout.
- **Serial-Logs** bei Emotionswechsel (Verifikation).

### Trigger-Details (Klarstellungen)
- **Neutral** = Grundzustand.
- **Happy** = **Boot → kurz Happy → automatischer Rückfall auf Neutral**
  (Begrüßung beim Start); zusätzlich bei Touch.
- **Tired** = nach längerer Inaktivität (Timeout).
- **Annoyed** = schnelles Dauer-Antippen (Piesacken; Touch-Frequenz über Schwelle).
- **Thoughtful** = **bevorzugt: BtnB lang gedrückt → Thoughtful.**
  - *Technischer Hinweis:* BtnB ist auf dem CoreS3 eine **Touch-Zone** (unten
    Mitte). „Lang gedrückt" = Finger über einen Hold-Schwellwert in der Zone
    gehalten (M5Unified `pressedFor()` / Hold).
  - **Fallback:** falls Long-Press auf der Touch-Zone noch nicht sauber
    unterstützt wird → **BtnB kurz → Thoughtful für 2–3 Sekunden**
    (getriggerter Zustand mit Auto-Rückfall auf Neutral).

## 3. Out of Scope (bewusst nicht)
- ❌ WLAN · ❌ KI/Backend · ❌ Audio
- ❌ komplexes Menü-/Navigationssystem
- ❌ Persistenz (NVS / Speichern über Reboot)
- ❌ Tageszeit-Emotionen via RTC/NTP (braucht Zeitquelle → später)
- ❌ Architektur-Overkill (kein Event-Bus, keine Scripting-/Timeline-Engine,
  keine datengetriebene Animations-DSL)

## 4. Geplante Module / mögliche Architektur
Additiv zur bestehenden Struktur (`main → App → lib/`-Module), **kein Umbau**:

| Modul | Rolle in Sprint 2 |
|---|---|
| **`lib/Persona/`** *(neu)* | hält `Emotion current_`, wählt sie per **einfacher Regel-Logik** aus Zeit/Input (Inaktivitäts-Timer, Touch-Frequenz, BtnB-Hold) |
| **`lib/Face/`** *(erweitern)* | `setEmotion(Emotion)` + emotionsabhängige Parameter; `update()/render()`-Trennung bleibt |
| **`lib/PcConfig/`** | Emotions-Tuning (Timeouts, Parameter je Emotion) als zentrale Stellschrauben |
| **`App`** | verdrahtet `Input → Persona → Face`; `main.cpp` bleibt unverändert |

**Datenfluss:** `Input → Persona (wählt Emotion) → Face (rendert Emotion)`.
**Leitidee:** Emotion = *Daten* (ein Parametersatz aus einer Tabelle), nicht
`if`-Verzweigungen überall → wartbar und 1:1 aus PERSONALITY.md ableitbar.

## 5. Backlog-Items (klein geschnitten)
1. Emotion-Enum + Parametersatz-Struktur aus PERSONALITY.md ableiten
   (deckt Notion „Erste Charlie-Emotionen definieren" ab).
2. `Face`: emotionsabhängiges Rendering (Augen/Mund je Emotion; Neutral = Basis).
3. `Persona`-Modul: Emotion halten + Auslöser
   (Inaktivität→Tired, Touch→Happy, Piesacken→Annoyed, BtnB-Hold→Thoughtful).
4. Boot-Happy → Neutral-Rückfall + sanfte Übergänge + Neutral-Timeout.
5. App-Verdrahtung `Input→Persona→Face` + Serial-Logs für Emotionswechsel.
6. Parameter am Gerät tunen (Charakter-Feinschliff gegen PERSONALITY.md).
7. Doku: CHANGELOG, TEST_CHECKLIST, PERSONALITY-Abgleich.

## 6. Definition of Done
- ✅ Alle **5 Emotionen** auf dem CoreS3 klar unterscheidbar sichtbar.
- ✅ Jede Emotion durch **dokumentierten lokalen Auslöser** reproduzierbar
  (inkl. Boot→Happy→Neutral und BtnB→Thoughtful).
- ✅ Übergänge flüssig, **kein Flackern**; Neutral-Rückfall funktioniert.
- ✅ Stabil (keine Freezes/Reboots über mehrere Minuten).
- ✅ Emotionen **konsistent zu PERSONALITY.md** (Abgleich dokumentiert).
- ✅ Serial-Logs zeigen Emotionswechsel; Build grün; auf Hardware verifiziert.
- ✅ Kein Out-of-Scope-Punkt angefasst.

## 7. Risiken
| Risiko | Gegenmaßnahme |
|---|---|
| Emotionen wirken „zu viel"/kitschig | Guardrail „weniger ist mehr"; dezente Parameter, kurze Dauer |
| Emotion-Flackern / hektische Wechsel | Mindest-Verweildauer je Emotion (Hysterese) + Cooldowns |
| *Thoughtful* ohne echten Auslöser wirkt willkürlich | klarer Trigger (BtnB-Hold), sonst kurzer Fallback-Zustand |
| Long-Press auf Touch-Zone (BtnB) unzuverlässig | Fallback „BtnB kurz → Thoughtful für 2–3 s" |
| Rendering-Last steigt (mehr Formen/Frame) | Canvas/PSRAM ist da; ggf. Bildrate/Formkomplexität begrenzen |
| Scope-Creep Richtung Menü/State-Framework | `Persona` bleibt minimale Regel-Logik, kein Framework |
| Piesack-Schwelle falsch getunt | Touch-Frequenz-Schwelle als Config-Konstante, am Gerät kalibrieren |

## 8. Testplan auf echter Hardware (M5Stack CoreS3)
Je Emotion: **Auslöser → erwartetes Bild → Serial-Log**.
- **Neutral:** nach Boot-Happy-Rückfall → ruhiges Gesicht (Basis).
- **Happy:** Boot (kurz) & Touch → freundliche Augenbögen, kurzer Bounce.
- **Tired:** *X* s nichts tun → halb geschlossene Augen, langsameres Blinzeln.
- **Annoyed:** schnell mehrfach antippen → flacher Blick / Augenrollen.
- **Thoughtful:** BtnB lang (bzw. Fallback BtnB kurz → 2–3 s) → Blick oben-seitlich.
- **Übergänge:** Emotion → Neutral-Rückfall nach Timeout, flüssig, kein Flackern.
- **Stabilität:** mehrere Minuten Mischbetrieb → keine Freezes/Reboots.
- **Logs:** jeder Wechsel im Serial Monitor sichtbar.
→ Ergebnis als Abschnitt „Sprint 2" in `docs/TEST_CHECKLIST.md`.

## 9. Versionsziel
- **Entwicklung:** `v0.3.0-dev`
- **Release bei DoD:** `v0.3.0 – Cheesecake 🍰`

---

*Hinweis zur Einordnung:* Notion-Sprint 2 (Roadmap-Phase 2 „Charlie reagiert")
wird hier bewusst auf den **Persönlichkeits-/Emotions-Teil** fokussiert;
Menü- und Interaktionslogik bleiben für einen späteren Sprint.
