import http.server
import socketserver

PORT = 5000

HTML_LOBBY = """
<!DOCTYPE html>
<html>
<head>
    <title>TELEMATICA LOBBY</title>
    <style>
        body { background: #0f172a; color: white; font-family: sans-serif; text-align: center; padding: 50px; }
        .box { border: 2px solid #38bdf8; padding: 20px; display: inline-block; border-radius: 10px; background: #1e293b; }
        h1 { color: #38bdf8; }
        .role { font-weight: bold; color: #f43f5e; }
    </style>
</head>
<body>
    <h1>🛰️ SERVICIO DE RED: JUEGO DE CIBER-COMBATE</h1>
    <div class="box">
        <p>El servidor de juego está activo en:</p>
        <code>telematica.local:8080</code>
        <hr>
        <p>Los roles se asigan secuencialmente al conectar.</p>
        <p>Comanda a tu equipo y protege los servidores críticos.</p>
    </div>
</body>
</html>
"""

class IdentityHandler(http.server.SimpleHTTPRequestHandler):
    def do_GET(self):
        self.send_response(200)
        self.send_header("Content-type", "text/html")
        self.end_headers()
        self.wfile.write(HTML_LOBBY.encode())

print(f"[*] Panel Web en: http://localhost:{PORT}")
with socketserver.TCPServer(("", PORT), IdentityHandler) as httpd:
    httpd.serve_forever()
