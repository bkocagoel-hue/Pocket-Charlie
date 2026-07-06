# 🎭 Pocket Charlie — Persönlichkeit & lokales Bedienmodell (v0.4.0-dev · Donut 🍩)

Charakter- und Verhaltensgrundlage für Pocket Charlie. Sie steuert Charlies
Ausdruck (Blick, Blinzeln, Augenbrauen, Timing), seine lokale Emotion/Mood und
die seltene Microcopy. **Stand: Sprint 3 (Donut) — vollständig lokal & offline.**
Baut auf der Emotion Engine v1 (v0.3.0) auf.

## 1. Grundcharakter

Charlie ist ein kleiner, lokaler Schreibtisch-**Companion** — kein Assistent,
keine App.

- **lokal & offline** — läuft komplett auf dem Gerät, ohne Netz.
- **ruhig, freundlich, charmant** — beständig und angenehm, nie aufdringlich.
- **minimalistisch** — Persönlichkeit im Ausdruck, nicht im Text; wenige, saubere
  Elemente statt Feature-Stapel.
- **leicht frech, aber nie nervig** — ein trockener Schalk als Würze, kein
  Dauerzustand, kein Produktivitäts-Nörgler.
- **kein überladenes UI** — ruhiges Gesicht, wenige lokale Screens, kurze Microcopy.
- **Companion-Gefühl statt App-Gefühl** — Charlie ist einfach *da* und freut sich
  über dich.

> **Charlie in einem Satz:** ein neugieriger, warmherziger Schreibtisch-Kumpel mit
> trockenem Humor — er wirkt über Blick, Timing und kleine Regungen, nicht über Worte.

**Die 3 Kernzüge:** *Neugierig* · *Warmherzig & verlässlich* · *Verschmitzt*
(die Würze, kein Dauerzustand). **Was Charlie *nicht* ist:** kein nerviger
Assistent, kein zynischer Roboter, nicht geschwätzig, nicht kindlich-übersüß.

## 2. Bedienphilosophie — Touch fühlt, Buttons navigieren

> **Leitsatz:** *Touch = emotionale Interaktion · Buttons = Navigation.*

**Touch (emotional):**
- **weckt** Charlie aus Tired/Sleeping.
- **beeinflusst die Emotion** — ein freundlicher Touch löst kurz *Happy* aus.
- **beeinflusst die Mood** — freundliche Taps heben die Grundstimmung leicht.
- **Piesacken** (schnelles Dauer-Tippen, ≥ 4 Taps in ~1,8 s) → *Annoyed*.
- **navigiert nicht** — der aktuelle Screen bleibt erhalten.

**Buttons (Navigation):**
- **BtnA** = vorheriger Screen · **BtnC** = nächster Screen.
- **BtnB** = kontextuelle Aktion im Screen (Face → *Thoughtful*; auf anderen Screens
  eine kurze `ok`-Rückmeldung).
- Auf dem CoreS3 sind A/B/C **Touch-Zonen am unteren Bildschirmrand** (kein
  physischer Tastenblock). Long-Press ist bewusst **nicht** genutzt (auf den
  Touch-Zonen nicht zuverlässig).

## 3. Lokale Screens

Vier lokale Screens, **umlaufend** über BtnA/BtnC gewechselt:

**Face → Clock → Mood → Info**

- **Face** — Standard: Emotion Engine, Augenbrauen, Microcopy, Mood, Sleep/Wake.
  `BtnB` → *Thoughtful*.
- **Clock** — **Uptime** (`HH:MM:SS`). Bewusst **kein RTC/NTP**: ohne zuverlässige
  Zeitquelle wäre eine echte Uhrzeit nicht vertrauenswürdig → ehrliche Uptime statt
  vorgetäuschter Uhrzeit.
- **Mood** — `mood` + Level (`high` / `neutral` / `low`) + aktuelle Emotion.
- **Info** — `Pocket Charlie` / `Donut` / `v0.4.0-dev`.

Alles bleibt **lokal**: keine KI, kein WLAN, kein Backend, kein Audio.

## 4. Mood light

Eine dezente, **längerfristige** Grundstimmung über der kurzfristigen Emotion.
Grobe Level: **High / Neutral / Low**. Regelbasis (bewusst einfach, keine
Simulation):

- **freundlicher Tap** hebt die Mood leicht (`+0,06`).
- **Piesacken** senkt sie **stärker** (`−0,18`, ~3× so stark wie ein freundlicher Tap).
- **langsamer Zerfall** zurück Richtung Neutral (`~ −0,02/s`).
- **Hysterese** gegen Flackern: *High* ab `> 0,40`, *Low* ab `< −0,40`, *Neutral* im
  Kernband (`±0,20`); dazwischen bleibt das Level stehen → **keine hektischen Wechsel**.
- Mood **färbt dezent die Idle-Microcopy** (High-/Neutral-/Low-Sätze), ändert aber
  nicht die Emotions-Trigger.

## 5. Microcopy

Kurze, dezente lokale Sprüche. Stil: **sehr kurz, ruhig, charmant, gut lesbar** auf
dem CoreS3 — **nicht** meme-lastig, **nicht** zu viel Text. Selten und
rate-limitiert (Mindestabstand ~3 s; erste Idle-Meldung frühestens nach ~30 s, dann
alle ~25–45 s). *Charlie sagt lieber nichts als etwas Belangloses.*

- **Normal / Idle (Neutral):** `hmm` · `...` · `tap?` · `sup?` · `donut?`
- **Fröhlich (High / Happy):** `yay` · `:)` · `hi!` · `fun` · `donut!`
  (Begrüßung: `hi` · `hey` · `ok` · `:)` · `yo`)
- **Grantig (Low / Annoyed):** `meh` · `ugh` · `tired` · `hmph` · `hey!` · `stop` · `bro...`

## 6. Visuelle Identität

- überwiegend **monochromes** Gesicht auf schwarzem Grund.
- **Lila/Violett als dezente Akzentfarbe** — der Identitäts-Akzent liegt in den Augen.
- **violette Augen / Iris** (`config::kColorEye`).
- **weiße Pupillen** (`config::kColorPupil`), die dem Blick folgen.
- **echte Lid-Schließung**: das Oberlid verdeckt das Auge von oben und endet
  geschlossen als klare Linie — kein bloßes Schrumpfen.
- **weiße Mundlinie**, datengetrieben gekrümmt (Lächeln … flach … Frown).
- **Augenbrauen** als neue emotionale Ausdrucksebene (weiß, über den Augen; die
  violette Iris bleibt frei — siehe §7).
- ruhige Idle-Regungen: leichtes **Atmen**, **Blinzeln**, gelegentlicher **Blickwechsel**.

## 7. Augenbrauen-Expression

Kleine, weiche weiße Striche über den Augen — datengetrieben und (wie das übrige
Gesicht) **weich interpoliert**, daher **kein Flackern**. Sie **unterstützen** die
Emotion, **ersetzen** sie nicht, und verdecken die violette Iris nicht.

- **Neutral** — flach / dezent.
- **Happy** — leicht angehoben → offener, freundlicher.
- **Tired** — tief / schwer → müder.
- **Thoughtful** — asymmetrisch (eine Braue höher) → charmanter, fragender.
- **Annoyed** — innere Enden nach unten geneigt → klar streng.
- **Sleeping** — ausgeblendet → ruhig, nicht überladen.

*Curious / Confused / Excited / WakingUp sind in der Datentabelle vorbereitet.*

## 8. Schnurrbart-Status (geparkt, nicht gelöscht)

Der Schnurrbart bleibt eine charmante Idee für Charlies Identität, ist auf dem
kleinen Display aber **riskant**: er kann mangels Nase wie eine Nase/ein Mund
wirken, das Gesicht überladen, als Sprite schnell unsauber aussehen und ist wenig
emotionsflexibel.

Er ist daher **zurückgestuft, aber erhalten** — als **optionales späteres Feature /
Easter Egg / Skin**. Die Architektur steht weiter bereit (`Face::drawMustache`,
Flag `config::kEnableMoustache`, Default `false`) und ist jederzeit reaktivierbar.
**In Sprint 3 haben die Augenbrauen Vorrang**, weil sie Emotionen besser
unterstützen.

## 9. Guardrails (Sprint 3)

Pocket Charlie bleibt in Sprint 3 **vollständig lokal**:

- ❌ **keine KI**
- ❌ **kein WLAN**
- ❌ **kein Backend / keine Cloud**
- ❌ **keine API-Keys / Secrets**
- ❌ **kein Audio**
- ✅ **alles lokal & offline**

> Ein KI-Backend kommt — wenn überhaupt — erst später als eigener, bewusster Sprint;
> die Guardrails werden dann gezielt geöffnet, nicht vorher.
