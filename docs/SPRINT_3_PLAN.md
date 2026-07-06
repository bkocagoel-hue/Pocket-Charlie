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
