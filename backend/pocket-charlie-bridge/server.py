#!/usr/bin/env python3
# ============================================================================
#  pocket-charlie-bridge - minimaler lokaler Backend-Server (Sprint 4, E3)
#
#  Nur Python-Standardbibliothek: 0 Dependencies, kein Flask/FastAPI, keine
#  Datenbank, keine API-Keys, keine KI, keine Cloud. Der M5Stack prueft ueber
#  GET /health, ob die Bridge im Heimnetz erreichbar ist.
#
#  Start:   python backend/pocket-charlie-bridge/server.py
#  (oder:   py backend/pocket-charlie-bridge/server.py)
# ============================================================================

import json
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

HOST = "0.0.0.0"  # im ganzen Heimnetz erreichbar (M5Stack nutzt die Laptop-IP)
PORT = 8787
VERSION = "0.1.0"


class Handler(BaseHTTPRequestHandler):
    server_version = "pocket-charlie-bridge/" + VERSION

    def do_GET(self):
        if self.path == "/health":
            self._send_json(
                {"ok": True, "service": "pocket-charlie-bridge",
                 "version": VERSION})
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
          f"lauscht auf http://{HOST}:{PORT}  (Strg+C beendet)")
    print(f"[bridge] Test im Browser: http://localhost:{PORT}/health")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\n[bridge] beendet.")


if __name__ == "__main__":
    main()
