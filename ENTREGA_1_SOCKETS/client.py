import socket
import threading
import sys

DEFAULT_HOST = "telematica.local"
DEFAULT_PORT = 8080

def receive_messages(sock):
    while True:
        try:
            data = sock.recv(1024)
            if not data: break
            print(f"\n[NOTIFICACIÓN] {data.decode().strip()}")
            print("> ", end="", flush=True)
        except: break

def main():
    host = DEFAULT_HOST
    port = DEFAULT_PORT
    if len(sys.argv) >= 3:
        host = sys.argv[1]
        port = int(sys.argv[2])

    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        ip_address = socket.gethostbyname(host)
        client_socket.connect((ip_address, port))
        print(f"[*] Conectado a {host} ({ip_address})")
    except Exception as e:
        print(f"[!] Error: {e}")
        return

    threading.Thread(target=receive_messages, args=(client_socket,), daemon=True).start()

    try:
        while True:
            msg = input("> ")
            if not msg: continue
            client_socket.send(msg.encode())
            if msg.upper() == "QUIT": break
    except KeyboardInterrupt: pass
    finally: client_socket.close()

if __name__ == "__main__":
    main()
