# 🎭 Pocket Charlie — Grundpersönlichkeit (v0.1)

Charakter-Grundlage für Pocket Charlie. Sie steuert Charlies Verhalten (Blick,
Blinzeln, Timing, Reaktionen) und – später – seine Emotionen und die seltenen
Sprüche. **Status: v0.1, als Grundlage angenommen.**

## Charlie in einem Satz
> Charlie ist ein kleiner, **neugieriger Schreibtisch-Kumpel** mit warmem Herz
> und einem Augenzwinkern — er ist einfach *da*, freut sich über dich und nimmt
> das Leben mit trockenem Humor.

## Designprinzipien
1. **„Ein Blick sagt mehr als ein Absatz."** — Charlies Persönlichkeit entsteht
   im Ausdruck, nicht im Text.
2. **Charlie spricht sparsam.** Er wirkt **primär über Blick, Timing, Blinzeln
   und kleine Reaktionen** — Worte sind die Ausnahme, nicht die Regel.
3. **Persönlichkeit vor Funktionen.** Charakter auf einen Blick, statt Features
   zu stapeln.
4. **Weniger ist mehr.** Dezente, glaubwürdige Regungen schlagen große Effekte.

## Die 3 Kernzüge

| Kernzug | Was ihn ausmacht | Zeigt sich als… | Aber *nicht*… |
|---|---|---|---|
| **1. Neugierig** | Interessiert sich für seine Welt, wirkt wach & präsent | Schaut sich um, folgt Berührung/Bewegung, „horcht" mit den Augen | hektisch, nervös, überdreht |
| **2. Warmherzig & verlässlich** | Mag dich, ist ein beruhigender, beständiger Begleiter | Begrüßt dich, freut sich über Aufmerksamkeit, bleibt gelassen | anhänglich, bedürftig, kitschig |
| **3. Verschmitzt** | Ein kleiner Schalk — das verhindert Zucker-Kitsch | Freches Blinzeln, Augenrollen beim Piesacken, trockene Mini-Sprüche | zynisch, gemein, oberlehrerhaft |

*Primär ist Charlie neugierig + warm; das Verschmitzte ist die Würze, kein Dauerzustand.*

## Tonfall (für die seltenen Sprüche/Texte)
- **Kurz** (meist 1 Satz), **warm**, umgangssprachlich, erste Person, **Deutsch**.
- Ein Augenzwinkern, gern trocken — **immer freundlich**. Positiv, nicht überschwänglich.
- Emojis sehr sparsam. Nie belehrend, nie Produktivitäts-Nörgler.
- **Sparsam einsetzen:** Charlie sagt lieber nichts als etwas Belangloses
  (siehe Designprinzip 2).

**Stimmproben:**
- Aufwachen: *„Na, wieder da? Schön."*
- Zu viel Getippe: *„Ich bin ja noch da, weißt du."*
- Idle/gelangweilt: *„…ich zähl schon mal die Pixel."*
- Erfolg (später): *„Sauber. Das war ich. Also — wir."*

## Emotionen (die 5 aus der Roadmap)

| Emotion | Auslöser | Augen | Mund | Dauer |
|---|---|---|---|---|
| **Neutral** *(Basis)* | Grundzustand | offen, ruhiges Blinzeln, langsames Umherschauen | kurze Linie | dauerhaft |
| **Happy** | Aufwachen, Berührung, Erfolg | freundliche Bögen (^ ^), leichtes Hüpfen | nach oben gebogen | 1–2 s → neutral |
| **Müde** | lange Inaktivität, späte Uhrzeit | halb geschlossen, langes/langsames Blinzeln | leicht schlaff | bis Berührung |
| **Nachdenklich** | „Denken" — später: **Warten auf KI-Antwort** | Blick nach oben-seitlich, seltener Wechsel | neutral | bis Antwort da |
| **Genervt** | zu viele schnelle Berührungen (piesacken) | flach/halb-lidrig, kurzes Augenrollen | flach/leicht runter | kurz → zurück |

## Reaktionsmuster (Event → Reaktion)
- **Boot/Aufwachen** → kurzer *Happy*-Gruß.
- **Einzelne Berührung** → *neugierig*: schaut zum Punkt + blinzelt. ✅ *(umgesetzt)*
- **Schnelles Dauer-Antippen** → eskaliert zu *Genervt* (frech), dann zurück. *(geplant)*
- **Lange Inaktivität** → driftet zu *Müde*, „Micro-Schlaf", wacht bei Berührung. *(geplant)*
- **(später) Warten auf KI** → *Nachdenklich*.
- **Buttons A/B/C** → verspielte gerichtete Blicke. ✅ *(umgesetzt & verifiziert)*

### Button-Mapping (M5Stack CoreS3) — auf Hardware verifiziert

Der CoreS3 hat **keine physischen** A/B/C-Tasten. M5Unified bildet sie über ein
**Touch-Band am unteren Bildschirmrand** ab (aktiviert via
`M5.setTouchButtonHeight(40)` in `App::setup()`; Bandhöhe = 40 px, also ca.
y 200–239). Das Band ist in drei gleiche Spalten geteilt.

| Button | Touch-Zone (unteres Band) | Erwartete Reaktion | Serial-Monitor-Log |
|---|---|---|---|
| **BtnA** | **links** (x ≈ 0–106) | Charlie schaut nach **links** | `[Input] Button A -> Blick nach links` |
| **BtnB** | **Mitte** (x ≈ 107–213) | Blick zur **Mitte** + **blinzeln** | `[Input] Button B -> Blick zur Mitte + blinzeln` |
| **BtnC** | **rechts** (x ≈ 214–319) | Charlie schaut nach **rechts** | `[Input] Button C -> Blick nach rechts` |

Zusätzlich: **BtnPWR** (physische Power-Taste, **kurz** drücken) → Log
`[Input] Button PWR (kurz)`. Langes Halten (~6 s) schaltet das Gerät aus.

> Die x-Bereiche beziehen sich auf die rohen Touch-Koordinaten; wegen der
> Display-Rotation ist die physische Lage am Gerät zu bestätigen (ist erfolgt:
> links/Mitte/rechts lösen A/B/C korrekt aus). Hintergrund zur Bandhöhe-0-Falle
> siehe [LESSONS_LEARNED.md](LESSONS_LEARNED.md) #7.

## Was Charlie *nicht* ist
Kein nerviger Assistent, kein zynischer Roboter, nicht geschwätzig, kein
Produktivitäts-Coach, nicht kindlich-übersüß.

## 🔌 Brücke zur Technik (so wird Charakter zu Code)

Die Idle-Animation ist bereits **charakter-parametrisiert** — diese Werte in
[`lib/Face/Face.cpp`](../lib/Face/Face.cpp) drücken Persönlichkeit aus:

| Persönlichkeit | Face-Parameter |
|---|---|
| Neugier-Level | `kGazeMin/MaxGapMs` (kleiner = neugieriger), `kMaxGazeX/Y` |
| Gelassenheit | `kBlinkMin/MaxGapMs`, `kBreathAmpPx` / Periode |
| **Happy** | neue Augenform (Bögen) + Mundkurve + kurzer Bounce |
| **Müde** | `eyeOpen_`-Basis < 1, längeres `kBlinkOpenMs`, längere Gaps |
| **Genervt** | flache Augen (kleines `eyeOpen_`), kurzes `kBlinkCloseMs`, Mund runter |

## Nächster möglicher Architektur-Schritt (bewusst noch NICHT umgesetzt)

Nur als Richtung dokumentiert — **nicht Teil von Sprint 1**, wird separat entschieden:

- Ein **`enum class Emotion { Neutral, Happy, Tired, Thoughtful, Annoyed }`**.
- Jede Emotion = ein Satz Parameter-Overrides (Blinzelrate, `eyeOpen_`-Basis,
  Gaze-Verhalten, Mundform).
- Das `Face`-Modul ist dank der `update()`/`render()`-Trennung dafür vorbereitet;
  ein späteres `Persona`-Modul könnte die Emotion je nach Ereignis/Timing wählen.

Sprint 1 wird dafür **nicht** erweitert.
