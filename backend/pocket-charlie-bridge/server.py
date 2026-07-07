#!/usr/bin/env python3
# ============================================================================
#  pocket-charlie-bridge - minimaler lokaler Backend-Server (Sprint 4, E3;
#  Provider-System seit Sprint 6, E1; optionaler Local-AI-Provider seit E2)
#
#  Nur Python-Standardbibliothek: 0 Dependencies, kein Flask/FastAPI, keine
#  Datenbank, keine API-Keys, keine Cloud-Pflicht. Der M5Stack prueft ueber
#  GET /health, ob die Bridge im Heimnetz erreichbar ist; GET /thought liefert
#  einen kurzen Text ueber einen austauschbaren ThoughtProvider (Default:
#  "mock" - lokal/statisch, keine KI, immer verfuegbar).
#
#  Provider waehlen:  PC_BRIDGE_PROVIDER=mock   python server.py  (Default)
#                     PC_BRIDGE_PROVIDER=ollama python server.py
#  Ollama konfigurieren (optional, nur wenn Provider "ollama" genutzt wird):
#                     PC_OLLAMA_URL, PC_OLLAMA_MODEL
#
#  Start:   python backend/pocket-charlie-bridge/server.py
#  (oder:   py backend/pocket-charlie-bridge/server.py)
# ============================================================================

import json
import os
import random
import urllib.request
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

HOST = "0.0.0.0"  # im ganzen Heimnetz erreichbar (M5Stack nutzt die Laptop-IP)
PORT = 8787
VERSION = "0.4.0"
DEFAULT_PROVIDER = "mock"

# Displaytauglich halten (CoreS3-Screen, Firmware-Puffer ist 26 Zeichen).
MAX_THOUGHT_CHARS = 26

DEFAULT_OLLAMA_URL = "http://localhost:11434"
DEFAULT_OLLAMA_MODEL = "llama3.2"
OLLAMA_TIMEOUT_S = 4.0  # harte Grenze; darf die Bridge nie lange blockieren

CHARLIE_PERSONA_PROMPT = (
    "You are Charlie, a tiny, calm desktop companion robot. Reply with "
    "exactly one short, warm thought for your display. Max "
    f"{MAX_THOUGHT_CHARS} characters, lowercase, no quotes, no emoji, "
    "no explanation - only the thought itself."
)

# Kurze, statische lokale "Gedanken" (E4A: mock/static, keine KI, kein Key).
# Displaytauglich halten: max. ~26 Zeichen, ruhig und charmant.
THOUGHTS = [
    "still here.",
    "hello local.",
    "tiny spark.",
    "online-ish.",
    "bridge says hi.",
    "local heart.",
    "beep. friendly.",
]


class ThoughtProvider:
    """Liefert den Text fuer GET /thought. Austauschbar, /thought-Contract
    (`{"text": "..."}`) bleibt fuer die Firmware in jedem Fall stabil."""

    name = "base"

    def get_thought(self) -> str:
        raise NotImplementedError


class MockThoughtProvider(ThoughtProvider):
    """Lokal/statisch, keine KI, kein Netzwerk - immer verfuegbarer Fallback."""

    name = "mock"

    def get_thought(self) -> str:
        return random.choice(THOUGHTS)


def _clean_for_display(text: str) -> str:
    """Ein-Zeilen-Text, Anfuehrungszeichen weg, hart auf Display-Laenge
    gekuerzt. Bewusst kein Crash bei seltsamer LLM-Ausgabe."""
    text = " ".join(text.split())
    text = text.strip().strip('"').strip("'").strip()
    return text[:MAX_THOUGHT_CHARS]


class OllamaThoughtProvider(ThoughtProvider):
    """Optionaler lokaler LLM-Provider (z. B. Ollama auf dem Bridge-Host).
    Kein API-Key, keine Cloud-Pflicht - laeuft komplett im Heimnetz. Jeder
    Fehler (nicht erreichbar, Timeout, leere/kaputte Antwort) faellt sauber
    auf den Mock-Fallback zurueck; die Bridge crasht dadurch nie."""

    name = "ollama"

    def __init__(self, url: str, model: str, fallback: ThoughtProvider):
        self._url = url.rstrip("/") if url else ""
        self._model = model
        self._fallback = fallback

    def is_configured(self) -> bool:
        return bool(self._url) and bool(self._model)

    def get_thought(self) -> str:
        if not self.is_configured():
            print("[bridge] ollama nicht konfiguriert (URL/Modell leer), "
                  "falle auf mock zurueck.")
            return self._fallback.get_thought()

        try:
            raw = self._ask_ollama()
        except Exception as exc:  # Netzwerk-Grenze: darf nie crashen
            print(f"[bridge] ollama-Fehler ({exc.__class__.__name__}: {exc}), "
                  f"falle auf mock zurueck.")
            return self._fallback.get_thought()

        text = _clean_for_display(raw)
        if not text:
            print("[bridge] ollama lieferte leere Antwort, falle auf mock "
                  "zurueck.")
            return self._fallback.get_thought()
        return text

    def _ask_ollama(self) -> str:
        body = json.dumps({
            "model": self._model,
            "prompt": CHARLIE_PERSONA_PROMPT,
            "stream": False,
        }).encode("utf-8")
        req = urllib.request.Request(
            f"{self._url}/api/generate", data=body,
            headers={"Content-Type": "application/json"})
        with urllib.request.urlopen(req, timeout=OLLAMA_TIMEOUT_S) as resp:
            payload = json.loads(resp.read().decode("utf-8"))
        return payload.get("response", "")


_mock_provider = MockThoughtProvider()
PROVIDERS = {
    "mock": _mock_provider,
    "ollama": OllamaThoughtProvider(
        url=os.environ.get("PC_OLLAMA_URL", DEFAULT_OLLAMA_URL),
        model=os.environ.get("PC_OLLAMA_MODEL", DEFAULT_OLLAMA_MODEL),
        fallback=_mock_provider,
    ),
}


def resolve_provider(requested: str) -> ThoughtProvider:
    provider = PROVIDERS.get(requested)
    if provider is None:
        print(f"[bridge] unbekannter PC_BRIDGE_PROVIDER '{requested}', "
              f"falle zurueck auf 'mock'.")
        return PROVIDERS[DEFAULT_PROVIDER]
    return provider


ACTIVE_PROVIDER = resolve_provider(
    os.environ.get("PC_BRIDGE_PROVIDER", DEFAULT_PROVIDER))


class Handler(BaseHTTPRequestHandler):
    server_version = "pocket-charlie-bridge/" + VERSION

    def do_GET(self):
        if self.path == "/health":
            self._send_json(
                {"ok": True, "service": "pocket-charlie-bridge",
                 "version": VERSION, "provider": ACTIVE_PROVIDER.name,
                 "local_ai_configured": PROVIDERS["ollama"].is_configured()})
        elif self.path == "/thought":
            self._send_json({"text": ACTIVE_PROVIDER.get_thought()})
        else:
            self._send_json({"ok": False, "error": "unknown path"}, status=404)

    def _send_json(self, payload, status=200):
        body = json.dumps(payload).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    # Kompaktes, hilfreiches Log statt des ausschweifenden Default-Formats.
    def log_message(self, fmt, *args):
        print(f"[bridge] {self.client_address[0]} {fmt % args}")


def main():
    server = ThreadingHTTPServer((HOST, PORT), Handler)
    print(f"[bridge] pocket-charlie-bridge v{VERSION} "
          f"(provider: {ACTIVE_PROVIDER.name}) "
          f"lauscht auf http://{HOST}:{PORT}  (Strg+C beendet)")
    print(f"[bridge] Test im Browser: http://localhost:{PORT}/health")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\n[bridge] beendet.")


if __name__ == "__main__":
    main()
