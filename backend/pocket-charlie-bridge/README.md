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
| `GET /health` | `{"ok": true, "service": "pocket-charlie-bridge", "version": "0.4.1", "provider": "mock", "local_ai_configured": true}` |
| `GET /thought` | `{"text": "still here."}` — Text kommt vom aktiven `ThoughtProvider`, Contract bleibt immer `{"text": "..."}` |

Unbekannte Pfade liefern `404`.

## Provider wählen

```bash
# Default: lokal/statisch, keine KI, kein Netzwerk
PC_BRIDGE_PROVIDER=mock python backend/pocket-charlie-bridge/server.py

# Optional: lokales LLM über Ollama (oder kompatiblen HTTP-Endpunkt)
PC_BRIDGE_PROVIDER=ollama python backend/pocket-charlie-bridge/server.py
```

| Env-Variable | Default | Zweck |
|---|---|---|
| `PC_BRIDGE_PROVIDER` | `mock` | `mock` oder `ollama`; unbekannte Werte fallen mit Log-Hinweis auf `mock` zurück |
| `PC_OLLAMA_URL` | `http://localhost:11434` | Basis-URL des lokalen Ollama-Servers |
| `PC_OLLAMA_MODEL` | `llama3.2` | Modellname, muss lokal via `ollama pull` vorhanden sein |
| `PC_OLLAMA_TIMEOUT` | `10` (Sekunden) | Hartes Timeout für den Ollama-Request; ungültige Werte fallen auf den Default zurück |

`mock` ist und bleibt Default und Fallback. Der `ollama`-Provider läuft komplett
lokal (kein API-Key, keine Cloud-Pflicht), hat ein hartes Timeout und fällt bei
jedem Fehler oder leerer Antwort automatisch auf `mock` zurück — die Bridge
crasht dadurch nie, auch wenn Ollama nicht läuft. Der `/thought`-Contract für
die Firmware ändert sich dabei nicht.

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
