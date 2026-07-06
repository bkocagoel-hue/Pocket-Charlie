# pocket-charlie-bridge 🌉

Minimaler lokaler Backend-Server für **Pocket Charlie** (Sprint 4 – Éclair ⚡).
Der M5Stack CoreS3 prüft über `GET /health`, ob die Bridge im Heimnetz
erreichbar ist. **Nur Python-Standardbibliothek: 0 Dependencies, keine
API-Keys, keine KI, keine Datenbank, keine Cloud.**

## Starten

Aus dem Projekt-Root (irgendein Python 3 genügt):

```bash
python backend/pocket-charlie-bridge/server.py
```

oder unter Windows:

```bash
py backend/pocket-charlie-bridge/server.py
```

Der Server lauscht auf Port **8787** (alle Interfaces, `0.0.0.0`).
Schnelltest im Browser auf dem Laptop:
<http://localhost:8787/health> · <http://localhost:8787/thought>

## Endpunkte

| Endpunkt | Antwort |
|---|---|
| `GET /health` | `{"ok": true, "service": "pocket-charlie-bridge", "version": "0.2.0"}` |
| `GET /thought` | `{"text": "still here."}` — zufälliger kurzer Satz aus einer **lokalen, statischen** Liste (E4A: mock/static; keine KI, keine externen Requests, kein API-Key) |

Unbekannte Pfade liefern `404`.

## Bridge-URL für den M5Stack konfigurieren

Der M5Stack braucht die **Heimnetz-IP des Laptops** — **nicht** `localhost`
(das wäre der M5Stack selbst!).

1. Laptop-IP herausfinden (Windows): `ipconfig` → Eintrag **IPv4-Adresse**
   des WLAN-Adapters, z. B. `192.168.1.23`.
2. In `lib/PcConfig/PcSecrets.h` (gitignored, Vorlage: `PcSecrets.example.h`)
   eintragen:

```cpp
constexpr const char* kBridgeUrl = "http://192.168.1.23:8787";
```

3. Firmware neu flashen. Laptop und M5Stack müssen im **selben WLAN** sein.

## Windows-Firewall

Beim ersten Start fragt Windows ggf. nach einer Freigabe für Python —
**„Privates Netzwerk" erlauben**, sonst erreicht der M5Stack die Bridge nicht
(Symptom: Browser auf dem Laptop zeigt `/health`, der M5Stack meldet
`bridge down`). Erscheint keine Abfrage, in den Firewall-Einstellungen
eingehende Verbindungen für Python im privaten Netz zulassen.

## Sicherheit

- Kein API-Key nötig — die Bridge liefert nur lokale, statische Antworten.
- Nur im eigenen Heimnetz betreiben; nicht ins Internet exponieren.
