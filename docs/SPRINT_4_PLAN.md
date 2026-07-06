# 📋 Sprint 4 – Éclair ⚡ (Plan)

**Status:** aktiv · **Version:** `v0.5.0-dev` → Release-Arbeitstitel `v0.5.0 – Éclair ⚡`
**Branch:** `sprint-4-eclair-online-widgets` · **Basis:** Donut-Stand (`cd27f42`, v0.4.0-dev)
**Thema:** **Online Widgets v1 + Emotion Expansion v2 + Persona Polish**

> Firmware-intern heißt der Codename **`Eclair`** (ASCII — das Display-Font kann
> kein `É`); Akzent + ⚡ nur in Docs/Release-Texten.

## 1. Guardrail-Wechsel (bewusst)

Bisher (Sprint 0–3): komplett lokal — kein WLAN, keine KI, kein Backend, kein Audio.
**Ab Sprint 4 wird vorsichtig geöffnet:**

| erlaubt ✅ | weiterhin verboten ❌ |
|---|---|
| WLAN (optional, non-blocking) | **API-Keys/Secrets in der Firmware** |
| kleines lokales Backend („Bridge") | echte KI-Anbindung (erst nach expliziter Freigabe) |
| einfache Online-Widgets | Audio/Mikrofon/Sprachmodus |
| | WebSocket/MQTT, Cloud-Sync, Accounts |
| | Release/Tag/Push ohne Freigabe |

## 2. Wichtigster Grundsatz: local-first

Charlie darf **nie** vom Internet abhängen. Ohne WLAN/Backend funktionieren
weiter: Face, Emotionen, Augenbrauen, Touch-Reaktionen, Button-Navigation,
Mood light, Microcopy, Sleep/Wake, Uptime, Info. Online ist **Zusatz** — ein
kleines Fenster nach draußen. Offline ist ein **normaler Zustand**, kein Fehler:
keine Crashes, keine Boot-Blockade, keine endlosen Retries.

## 3. Sprint-Ziel

```text
Charlie bleibt ein kleiner lokaler Companion.
Wenn Internet da ist, bekommt er neue kleine Fähigkeiten.
Wenn Internet weg ist, bleibt er ruhig, charmant und benutzbar.
```

## 4. Architektur (klein, im bestehenden Modul-Muster)

| Modul | Aufgabe |
|---|---|
| `lib/Network/NetworkManager` | WiFi-Zustands-FSM: `disabled` (keine Credentials) → `connecting` (async, Timeout) → `online` / `offline` (seltene Retries mit Backoff). Non-blocking, `update(now)` gepollt. |
| `lib/Online/OnlineClient` | Backend-Ping `/health` + `/thought`. Kurze Timeouts, Antworten gekürzt. Requests laufen in einem kleinen FreeRTOS-Task (2. Core) → die 30-FPS-Loop friert nie ein. |
| `backend/pocket-charlie-bridge` | Mini-HTTP-Server, **Python-Stdlib, 0 Dependencies** (`server.py`). Endpunkte: `GET /health`, `GET /thought` (v1 statisch/lokal, keine KI, keine Keys). |
| `lib/PcConfig/PcSecrets.example.h` | Vorlage für SSID/Passwort/Bridge-URL. Echte `PcSecrets.h` ist **gitignored**; via `__has_include` baut die Firmware auch ohne — dann Offline-Modus. |

Bewusst **nicht**: UI-Framework, Plugin-System, WebSocket/MQTT, Accounts,
Datenbank, kompliziertes Deployment. Emotion Engine / Face / Mood hängen **nie**
vom Netz ab; Persona bekommt höchstens Einweg-Ereignisse.

**Warum Python-Stdlib fürs Backend:** null Dependencies, ein File, läuft mit
jedem Python 3 (auf dem Dev-Rechner sogar über PlatformIOs mitgeliefertes
Python) — Node/Express bräuchte Installation + npm für zwei JSON-Endpunkte.

## 5. Screens & Bedienung

Neue Reihenfolge (5 Screens, umlaufend):

```text
Face → Clock → Mood → Online → Info
```

**Ein** `Online`-Screen (statt separater WiFi-/Bridge-Screens): für den Nutzer
ist „online" ein Konzept; der Screen zeigt den Substatus —
`wifi offline/trying/online` → `bridge ok/down` → kurzer Online-Thought.

| Eingabe | Wirkung |
|---|---|
| `BtnA` / `BtnC` | vorheriger / nächster Screen (unverändert) |
| `BtnB` auf Face | Thoughtful (unverändert) |
| `BtnB` auf Online | **Refresh**: Ping / Thought holen |
| Touch | emotionale Interaktion (Happy/Mood/Wake) — **keine** Navigation |

## 6. Widgets (v1-Scope + Backlog)

| Widget | Beschreibung | Sprint-4-Status |
|---|---|---|
| WiFi-Status | `wifi` offline/trying/online | **bauen** (im Online-Screen) |
| Bridge-Ping | `bridge` ok/down | **bauen** |
| Online Thought | kurzer Satz vom Backend | **bauen** (minimal) |
| Time / Real Clock | echte Zeit via NTP/Backend | prüfen → **TODO**, Clock bleibt ehrlich Uptime |
| Weather Mini | Wetter via Bridge (Mock möglich) | vorbereiten/**TODO** — kein Key in Firmware |
| Daily Vibe | Tagesgefühl/Spruch | vorbereiten/TODO |
| Tiny News | 1 Mini-Headline | später |
| Study Buddy / Focus Timer | Lernmodus / Pomodoro light | später |
| Battery | Akkustand (`M5.Power`) | später, falls stabil |
| Chess / Döner Radar | Spiel-/Easter-Egg-Ideen | deutlich später |

## 7. Emotion Expansion v2

Befund: `Curious, Confused, Excited, Sad, WakingUp` sind **bereits vorbereitet**
(Enum + Face-Stil `styleFor` + Augenbrauen `eyebrowFor`). Es fehlen nur:

- **Namen** in `Persona::emotionName()` (liefert aktuell `"?"` für sie),
- **Trigger** (klein, additiv — bestehende Regeln bleiben unangetastet),
- **Microcopy** (`Phrases.h`), Doku, Hardware-Tests.

Geplante Verknüpfung Online ↔ Persona (Einweg-Ereignisse, nie blockierend):

| Ereignis | Emotion |
|---|---|
| WLAN verbindet | Curious/Thoughtful |
| WLAN online / Bridge ok / erster Thought | Excited (kurz) |
| Bridge down / Timeout | Confused (kurz, dann Neutral) |
| wiederholter Online-Fehler | dezent Sad (kein Dauerzustand) |
| Aufwachen aus Sleeping | WakingUp (statt hartem Sprung) |

Microcopy-Stil bleibt: ruhig, charmant, minimalistisch, leicht frech — z. B.
`hmm?`, `oh?` (Curious) · `huh?`, `eh?` (Confused) · `online!`, `nice` (Excited)
· `still here` (Sad) · `...`, `morning` (WakingUp) · offline-charmant:
`offline / still me`, `net? / nah`, `bridge / zzz`.

## 8. Einheiten & Teststrategie

| Einheit | Inhalt | Commit-Regel |
|---|---|---|
| **E1 – Setup** | Branch, `v0.5.0-dev`, Codename `Eclair`, dieser Plan; keine Netzwerklogik | Commit nach grünem Build |
| **E2 – WLAN-Grundlage** | NetworkManager, Secrets-Handling, Offline-Fallback, Online-Screen (WiFi-Status) | Build + **Hardware-Test vor Commit** |
| **E3 – Backend Bridge** | `server.py` (`/health`), Backend-README, OnlineClient-Ping, Timeout | Build + **Hardware-Test vor Commit** |
| **E4 – Online Thought** | `/thought`, BtnB-Refresh, Fallbacks, Emotion-Verknüpfung | Build + **Hardware-Test vor Commit** |
| **E5 – Docs/Polish** | TEST_CHECKLIST, README-Dev-Setup, CHANGELOG `[Unreleased]`, PERSONALITY | Doku-Commit nach Prüfung |

## 9. Definition of Done (Turbo v1)

Branch existiert · `v0.5.0-dev` gesetzt · Sprint-4-Doku vorhanden · WLAN optional
& non-blocking · Online/Offline-Status sichtbar · Bridge mit `/health` · M5Stack
kann Bridge pingen · Online-Screen existiert · BtnB-Refresh funktioniert ·
Offline-Fallback funktioniert · **keine Secrets im Repo, keine Keys in Firmware**
· Donut-Funktionen intakt · Build grün · Hardware-Testpunkte dokumentiert ·
kein Release/Tag/Push ohne Freigabe.

## 10. Risiken

| Risiko | Gegenmaßnahme |
|---|---|
| Boot hängt am WLAN | async connect + Timeout; Boot erreicht immer das Face |
| HTTP friert UI ein | Requests im FreeRTOS-Task (2. Core), kurze Timeouts, Fallback-Text |
| Secrets landen im Repo | `PcSecrets.h` gitignored, nur `*.example.h` committed, README |
| Emotion-Engine-Regression | Trigger additiv; lokale Regeln unverändert; Hardware-Checkliste |
| Scope-Creep (10 Widgets) | v1 = WiFi + Bridge + Thought; Rest Backlog/TODO |
| Endlos-Retries / Log-Spam | Backoff mit Obergrenze; nur Statuswechsel loggen |

## 11. Offener Punkt aus Sprint 3

`v0.4.0 – Donut 🍩` ist **nicht released** (kein Tag); `main` steht 2 Commits vor
`origin/main` (Push wartet auf Freigabe). Das Donut-Release kann später von
`main` nachgeholt werden oder bewusst in `v0.5.0` aufgehen — Entscheidung offen.
