# 📋 Sprint 3 – Lokale Interaktion (Plan)

**Status:** aktiv · **Version:** `v0.4.0-dev` → Release-Arbeitstitel `v0.4.0 – Donut 🍩`
**Grundlage:** [PERSONALITY.md](PERSONALITY.md) · baut auf der Emotion Engine v1 (v0.3.0) auf.

> **Kein** „Charlie denkt mit KI". Dieser Sprint = **Charlie reagiert lokal besser**.
> KI-Backend kommt später als eigener, bewusster Sprint (Guardrails dann öffnen).

## 1. Sprint-Ziel
Charlie soll sich lokal **interaktiver, persönlicher und verständlicher** anfühlen —
weiterhin **ohne WLAN, KI, Backend, Audio**. Zielgefühl: *„Ich tippe Charlie an —
und er fühlt sich ein kleines bisschen lebendiger an."*

## 2. Scope (In Scope)
- **Lokale Interaktionsschicht:** Eingaben mit etwas Kontext (einfacher / wiederholter /
  schneller Touch, Buttons) statt reiner Einzel-Trigger.
- **Kleine Modus-/Menülogik:** wenige lokale Modi (z. B. Face / Status / Thought),
  über BtnA/BtnC gewechselt. Kein verschachteltes UI.
- **Statusmeldungen & Gedankenblasen:** sehr kurze Microcopy (`hmm`, `ok`, `tap?`,
  `zZz`, `?`), dezent, gut lesbar, minimalistisch.
- **Mood light:** einfache, nachvollziehbare Regelbasis über der kurzfristigen Emotion.
- **Visuelle Identität** behutsam stärken (violette Akzente); **Schnurrbart** als
  Pixel-Sprite evaluieren (nur einbauen, wenn er wirklich gut aussieht).

## 3. Out of Scope (bewusst nicht)
- ❌ WLAN · ❌ KI/Backend/Cloud · ❌ API-Keys/Secrets · ❌ Audio
- ❌ komplexes/verschachteltes Menüsystem, überladene UI
- ❌ Architektur-Großumbauten, Event-Bus, Scripting/Timeline-Engine
- ❌ Release erstellen/veröffentlichen (dieser Sprint bleibt `-dev`)
- ❌ `main.cpp` ändern (außer zwingend nötig + vorher begründet)

## 4. Geplante Module (additiv, klein)
| Modul | Rolle |
|---|---|
| **`lib/Interaction/`** *(neu)* | `InputContext`: klassifiziert den Input-Snapshot zu **Intents** (SingleTap, RapidTap, BtnA/B/C, PWR) + Aktivitätskontext |
| **`lib/Persona/`** *(erweitern)* | Emotion-FSM bleibt; **Mood light** als dezente Grundstimmung; `Phrases`/Microcopy |
| **`lib/App/`** *(erweitern)* | kleine **Modus-Logik**; verdrahtet Interaction → Persona → Face |
| **`lib/Face/`** *(erweitern)* | Statusmeldungen/Gedankenblasen rendern; Schnurrbart-Sprite (optional) |
| **`lib/PcConfig/`** | Version + Interaktions-/Mood-Konstanten (Timings, Schwellen) |

Leitidee: klein, klar, datengetrieben; keine große State-Machine, wenn eine
einfache Lösung reicht.

## 5. Backlog-Items (mit Notion-Bezug)
1. `InputContext`-Modul (Intent-Klassifikation), read-only eingehängt.
2. Kontextabhängige lokale **Touch-Reaktionen** *(Notion: „Lokale Touch-Reaktionen umsetzen")*.
3. Kleine **Modus-/Menülogik** BtnA/BtnC *(Notion: „Einfache Menuelogik definieren")*.
4. **Statusmeldungen/Gedankenblasen** + Microcopy *(Notion: „Lokale Sprueche/Statusmeldungen")*.
5. **Mood Engine light** (Regelbasis, dokumentiert) *(neues Item – vorgeschlagen)*.
6. **Schnurrbart** als Pixel-Sprite evaluieren; nur aktivieren, wenn sauber.
7. Doku: PERSONALITY (Persona-/Mood-Regeln), CHANGELOG, TEST_CHECKLIST.

## 6. Definition of Done
- Version `v0.4.0-dev`; lokale Interaktionslogik implementiert.
- Emotion Engine funktioniert weiter (keine Regression).
- Modus-/Menülogik vorhanden **oder** bewusst begründet verschoben.
- Statusmeldungen/Gedankenblasen lokal **oder** bewusst begründet verschoben.
- Lokale Persona-/Mood-Regeln dokumentiert.
- Violette Identität intakt; Schnurrbart geprüft.
- Docs + Notion aktuell; Build grün; Hardware-Checkliste aktualisiert.
- Kein WLAN/KI/Audio; keine Secrets im Code.

## 7. Risiken
| Risiko | Gegenmaßnahme |
|---|---|
| Scope-Creep Richtung App/Menü | wenige feste Modi, kein verschachteltes UI |
| Button-Semantik-Kollision | erst prüfen (BtnA/C aktuell nur Gaze → frei), BtnB=Thoughtful bleibt |
| Emotion-Engine-Regression | Interaction read-only starten; jede Einheit einzeln bauen/testen |
| Text-Überladung | Microcopy sehr kurz, dezent, selten |
| Mood unkontrolliert/hektisch | einfache Regeln, Hysterese/Cooldowns, Zufall sparsam & seedbar |

## 8. Testplan (echte Hardware, M5Stack CoreS3)
Boot (`v0.4.0-dev`) · Normalzustand · Touch einfach · Touch mehrfach · schnelles
Tippen → Annoyed · BtnA/BtnB/BtnC · Inaktivität → Tired → Sleeping · Aufwachen ·
Gedankenblase · Statusmeldung · kein Flackern/Freeze/Reboot.

## 9. Versionsziel
- Entwicklung: `v0.4.0-dev`
- Release bei DoD: `v0.4.0 – Donut 🍩` (separat, erst nach Hardware-Test)

---

## Einheit 4 – Button-Menüführung (umgesetzt, v0.4.0-dev)

**Bedienphilosophie:** *Buttons navigieren, Touch interagiert emotional.*
- `BtnA` = vorheriger Screen · `BtnC` = nächster Screen · `BtnB` = Aktion im Screen.
- **Touch** bleibt Charlie-Interaktion (Happy/Mood/Wecken) und navigiert **nicht**;
  der aktuelle Screen bleibt erhalten.
- **Long-Press: nicht** umgesetzt (auf CoreS3-Touch-Zonen nicht zuverlässig) →
  TODO; aktuell nur Short-Press.

**Screens (Reihenfolge Face → Clock → Mood → Info, umlaufend):**
- **Face** – Standard; Emotion Engine, Microcopy, Mood, Sleep/Wake wie bisher.
  `BtnB` → Thoughtful.
- **Clock** – **Uptime** (`uptime HH:MM:SS`). Bewusst **kein RTC/NTP**: ohne
  zuverlässige Zeitquelle wäre echte Uhrzeit nicht vertrauenswürdig → ehrlich
  Uptime statt vorgetäuschter Uhrzeit.
- **Mood** – `mood` + Level (high/neutral/low) + aktuelle Emotion.
- **Info** – `Pocket Charlie` / `Donut` / `v0.4.0-dev`.
- `BtnB` auf Clock/Mood/Info → kurze `ok`-Rückmeldung.

**Architektur:** neues `lib/Menu/` (reine Navigation, getrennt von Emotion),
`Display::showScreen()` (Text-Renderer, flicker-frei – nur bei Änderung neu
gezeichnet), `Persona::pokeThoughtful()` (BtnB-Aktion → Menü von Persona
entkoppelt). Text-Screens nutzen violette Akzente. `main.cpp` unverändert.

---

## Einheit 6 – Augenbrauen-Expression (umgesetzt, v0.4.0-dev)

**Fokuswechsel:** Statt des Schnurrbarts bekommt Charlie eine kleine
**Augenbrauen-Ausdrucksebene** – lesbarere Emotionen bei geringerem Risiko
(kein Mund-/Nasen-Effekt, flexibler pro Emotion). Schnurrbart-Status siehe unten.

**Ziel:** minimalistische Pixel-Augenbrauen, die Emotionen besser lesbar machen –
schlicht, wenige Pixel, kein Flackern, monochrom (weiß wie der Mund), **ohne** die
violette Iris zu überdecken; Emotionen unterstützen, nicht ersetzen.

**Umsetzung (rein additiv, datengetrieben – gleiche Mechanik wie `EmotionStyle`):**
- Neuer Stil-Datensatz `eyebrowFor(Emotion)` in `Face.cpp` mit vier Feldern:
  `lift` (heben/senken), `tilt` (innere Enden tiefer = streng / höher = weich),
  `asym` (eine Braue höher = nachdenklich/verwirrt), `hidden` (Sleeping → keine).
- Weiche Interpolation über `kStyleLerp` (`sBrowLift/Tilt/Asym/Vis_`) → **kein
  Flackern**, Übergänge passen zum restlichen Gesicht.
- Gekapselte Render-Funktion `Face::drawEyebrows(cx, eyeY, lift, tilt, asym, vis)`:
  zwei kurze weiche Striche über den Augen; `vis` skaliert Breite/Dicke, sodass
  die Brauen beim Einschlafen ruhig „zusammengehen" statt hart zu verschwinden.

**Varianten je Emotion:** Neutral flach/dezent · Happy leicht angehoben ·
Tired tief/schwer · Thoughtful asymmetrisch (eine höher) · Annoyed innen deutlich
geneigt (streng) · Sleeping keine · Curious/Confused/Excited/WakingUp vorbereitet.

**Bewusst nicht angetastet:** Emotion Engine (`Persona`/`Emotion`), Augen-/Pupillen-/
Blink-Logik, `main.cpp`. Geänderte Dateien: `lib/Face/Face.h`, `lib/Face/Face.cpp`,
`lib/PcConfig/PcConfig.h`, `docs/SPRINT_3_PLAN.md`.

### Schnurrbart-Status (zurückgestuft, nicht gelöscht)
Der Schnurrbart bleibt eine charmante Idee, ist auf dem kleinen Display aber
riskant (Mund-/Nasen-Effekt, überladenes Gesicht, unsaubere Sprites, wenig
emotionsflexibel). Er bleibt daher als **optionales späteres Feature / Easter Egg
/ Skin** erhalten: Architektur (`Face::drawMustache`, Flag
`config::kEnableMoustache`, Default `false`) steht und ist reaktivierbar. In
Sprint 3 hat die Augenbrauen-Expression Vorrang.

---

## Einheit 7 – Local Widgets v1 (Modell dokumentiert, v0.4.0-dev)

**Ziel:** die vorhandene lokale Screen-Struktur als erste **lokale Widgets** sauber
benennen und dokumentieren — **ohne** großes UI-Framework, kein Overengineering.

**Denkmodell:**
```
Screen = aktuell sichtbare Ansicht (welche)
Widget = kleiner lokaler Inhalt / Funktion innerhalb einer Ansicht (was)
```
In Sprint 3 liegen Screen und Widget bewusst **1:1** übereinander (ein Widget je
Screen). Das reicht und bleibt schlicht.

**Entscheidung: Option A (dokumentieren).** Die bestehende Struktur ist bereits
sauber geschichtet und braucht **keine** Widget-Abstraktion:
- `Menu` (`lib/Menu/Menu.h`) = reine Navigation (welcher Screen), entkoppelt von Emotion.
- **Face-Widget** = eigenes Modul `Face` (Off-Screen-`M5Canvas`, flicker-frei).
- **Text-Widgets** (Clock/Mood/Info) teilen **einen** generischen Renderer
  `Display::showScreen(title, main, sub)`; die Daten liefert `App::renderScreen()`.

Eine Basisklasse/Registry/Plugin-Struktur wäre bei 4 statischen Screens
Overengineering → **nicht** umgesetzt.

**Widget-Übersicht (Stand Donut):**
| Widget | Ansicht | Inhalt | Quelle |
|---|---|---|---|
| **Face** | Face-Screen | Gesicht, Emotion, Augen/Pupillen, Blinzeln, Augenbrauen, Microcopy, `zZz`/`?`, Touch-Reaktion, Mood im Hintergrund | `lib/Face/` |
| **Clock / Uptime** | Clock-Screen | **Uptime** `HH:MM:SS` (bewusst kein RTC/NTP → ehrliche Laufzeit) | `App::renderScreen` + `Display::showScreen` |
| **Mood** | Mood-Screen | `mood` + Level (high/neutral/low) + aktuelle Emotion | `App::renderScreen` + `Persona` |
| **Info** | Info-Screen | `Pocket Charlie` / `Donut` / `v0.4.0-dev` | `App::renderScreen` + `PcConfig` |

**Bedienprinzip unverändert:** Buttons = Navigation (`BtnA` zurück, `BtnC` vor,
`BtnB` Aktion/Thoughtful), Touch = emotionale Interaktion; umlaufend
Face → Clock → Mood → Info → Face.

**Guardrails:** alles lokal — keine KI, kein WLAN, kein Backend, kein Audio, keine
Cloud. `main.cpp` unverändert.

**TODO / offen (bewusst nicht umgesetzt):**
- **Battery-Widget** — technisch machbar über `M5.Power.getBatteryLevel()` /
  `M5.Power.isCharging()` (CoreS3 = AXP2101). Zurückgestellt: Prozentwert kann grob/
  springend sein, bräuchte einen 5. Screen + neuen Hardware-Test. Nur einbauen, wenn
  der Wert stabil und ehrlich ist — **kein** Fake-Akkustand.
- **Clock vs. Uptime** — echte Uhrzeit erst, wenn eine zuverlässige Zeitquelle sauber
  geklärt ist; bis dahin bleibt **Uptime** der ehrliche Standard.
- Optionales Code-Polish (Option B): kleine benannte Render-Helfer in
  `App::renderScreen()` + Codename `Donut` zentral in `PcConfig` — rein kosmetisch,
  nur wenn gewünscht (dann mit Build + Hardware-Test).
