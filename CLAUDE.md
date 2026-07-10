# CLAUDE.md — Projektregeln für Claude Code

Diese Datei definiert die projektspezifischen Arbeitsregeln für Claude Code.
Wo möglich, haben diese Regeln Vorrang vor generischem Standardverhalten.

## Projektbeschreibung

Pocket Charlie ist ein DIY-KI-Companion-Gerät auf Basis des **M5Stack
CoreS3** (ESP32-S3, PlatformIO, Arduino-Framework, M5Unified/M5GFX).
Fokus: Persönlichkeit, Display, Interaktion, lokale Widgets und optionale
KI-Anbindung über eine lokale Bridge. Inspiriert von StackChan, aber kein
Nachbau.

Das Projekt arbeitet **sprintweise, in kleinen, einzeln testbaren
Einheiten** — jede Einheit wird gebaut, auf echter Hardware verifiziert und
erst danach committet. Öffentliche Releases folgen einer Dessert-Codenamen-
Linie (Apple Pie → Brownie → Cheesecake → Éclair → Fudge → …).

## Arbeitsweise

- Kleine, in sich abgeschlossene Schritte statt großer Umbauten.
- Neue Fähigkeiten kommen als eigenes Modul unter `lib/` — nicht als
  wachsender Zusatz in bestehenden Modulen ohne klare Verantwortung.
- Nach jedem Build: Ergebnis, Dauer, RAM/Flash-Auslastung und geänderte
  Dateien nennen.
- Keine Features vorgreifend bauen, die noch nicht im aktuellen Sprint
  dran sind.
- Bei Unklarheit über Scope oder Architektur-Entscheidung: nachfragen statt
  annehmen.

## Sicherheitsregeln

- **Keine Secrets, API-Keys oder privaten Daten** in Projektdateien,
  Commits, Logs oder Beispielcode.
- `PcSecrets.h` (falls vorhanden) bleibt unangetastet und wird nie
  committet — nur `PcSecrets.example.h` als Vorlage.
- KI-/Online-Funktionen laufen ausschließlich über die lokale Bridge,
  niemals mit direkt in der Firmware eingebetteten Zugangsdaten.
- Das Gerät muss offline vollständig funktionsfähig bleiben (local-first,
  keine Cloud-Pflicht).

## Git-Regeln

- Committet wird erst **nach ausdrücklicher Hardware-Bestätigung** durch
  den User (Ausnahme: reine Doku-Änderungen nach kurzer Prüfung ohne
  Hardware-Test).
- **Kein Push, kein Tag, kein Release, kein GitHub-Release-Draft ohne
  ausdrückliche Freigabe** — niemals automatisch, auch nicht nach
  erfolgreichem Commit.
- Vor jedem Commit prüfen: `git status`, `git diff --stat`, keine
  unerwarteten Änderungen an `main.cpp`, `PcSecrets.h`, Bridge-Dateien;
  keine Secrets im Diff.
- Nach jedem Commit bestätigen: Hash, Message, Branch, HEAD, Zustand des
  Arbeitsbaums, „kein Push/Tag/Release".
- Bevorzugt neue Commits statt `--amend`; keine destruktiven Git-Befehle
  (`reset --hard`, `push --force`, `clean -f`) ohne ausdrückliche Anweisung.

## Hardwaretest-Regeln

- Jede Firmware-Änderung wird auf echter Hardware (M5Stack CoreS3) getestet,
  bevor sie committet wird.
- Nach dem Build werden konkrete Hardware-Testpunkte formuliert und dem
  User zur Ausführung vorgelegt.
- Erst nach expliziter Rückmeldung „Test bestanden" durch den User wird
  committet.
- Ergebnisse werden dokumentiert (siehe Obsidian-Vault, Abschnitt unten) —
  insbesondere: kein Freeze/Reboot/Flackern, korrektes Sound-Verhalten,
  korrekte Navigation.

## `main.cpp`

`main.cpp` ist ein dünner Einstiegspunkt und soll **möglichst stabil**
bleiben. Änderungen daran nur, wenn wirklich nötig — neue Fähigkeiten
gehören in eigene Module unter `lib/`.

## Externer Obsidian-Vault

Es existiert ein externer Obsidian-Vault als lokales Projektgedächtnis
unter `Pocket-Charlie-Obsidian\Pocket-Charlie-Obsidian` (außerhalb dieses
Repos). Er dokumentiert Sprints, Hardwaretests, Entscheidungen, Prompts
und Release-Notizen. GitHub bleibt für Code, Notion bleibt für
Backlog/Sprint-Board zuständig.

- **Obsidian-Dateien werden nur geändert, wenn der User es ausdrücklich
  verlangt** — nicht automatisch als Nebeneffekt von Code-Arbeit in diesem
  Repo.
- Dieses Repository (`Pocket Charlie`) wird durch Arbeit am Obsidian-Vault
  nie verändert, und umgekehrt.

## Freigabe-Pflicht (Zusammenfassung)

**Keine Pushes, Tags oder Releases ohne ausdrückliche Freigabe des Users.**
Das gilt unabhängig davon, wie klein oder offensichtlich die Änderung
erscheint.

## Notion

Notion bleibt Backlog- und Sprint-Board. Claude ändert Notion-Inhalte nur nach ausdrücklicher Anweisung des Users. Notion ist nicht die Quelle für Code-Wahrheit; maßgeblich für Code ist dieses Repository.

## Claude-Code-Subagents

Subagents dürfen zur Prüfung, Dokumentation und Testplanung eingesetzt werden.

Empfohlene Rollen:
- `pocket-code-reviewer`: prüft Codeänderungen, ändert aber keine Dateien.
- `hardware-test-planner`: erstellt Hardware-Testchecklisten, ändert aber keine Code-Dateien.
- `obsidian-scribe`: dokumentiert nur im externen Obsidian-Vault und nur auf ausdrückliche Anweisung.
- `release-sheriff`: prüft Release-Bereitschaft, führt aber keine Pushes, Tags oder Releases aus.

Subagents dürfen keine Secrets lesen, speichern oder in Dateien übernehmen.
Subagents dürfen keine Git-Schreiboperationen ausführen, außer der User weist dies ausdrücklich an.