import socket
import base64
from pyrtcm import RTCMReader
import time
import io

class RTK:
    def __init__(self, ntrip_host="caster.centipede.fr", ntrip_port=2101,
                 mountpoint="NORT", username="centipede", password="centipede"):
        self.ntrip_host = ntrip_host
        self.ntrip_port = ntrip_port
        self.mountpoint = mountpoint
        self.username = username
        self.password = password

        self.auth = f"{self.username}:{self.password}"
        self.auth_b64 = base64.b64encode(self.auth.encode()).decode()

        self.sock = None
        self.rtcm_reader = None

    def connect_ntrip(self):
        """Connexion au serveur NTRIP et initialisation du RTCMReader."""
        request = (
            f"GET /{self.mountpoint} HTTP/1.0\r\n"
            f"User-Agent: NTRIP PythonClient\r\n"
            f"Authorization: Basic {self.auth_b64}\r\n"
            f"\r\n"
        )

        print(f"[{time.strftime('%H:%M:%S')}] Connexion à {self.ntrip_host}:{self.ntrip_port}...")

        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.settimeout(10)

        try:
            self.sock.connect((self.ntrip_host, self.ntrip_port))
            self.sock.sendall(request.encode())

            # Lecture complète de l'en-tête HTTP jusqu'à \r\n\r\n
            response = b""
            while b"\r\n\r\n" not in response:
                chunk = self.sock.recv(1024)
                if not chunk:
                    break
                response += chunk

            header_text = response.decode(errors="ignore")
            print(f"[{time.strftime('%H:%M:%S')}] Réponse du serveur :\n{header_text.strip()}")

            if "ICY 200 OK" not in header_text:
                print(f"[{time.strftime('%H:%M:%S')}] Erreur : Connexion refusée par le serveur NTRIP.")
                self.sock.close()
                exit(1)

            # Utiliser la socket pour créer un flux binaire
            print(f"[{time.strftime('%H:%M:%S')}] Connexion acceptée. Démarrage du lecteur RTCM...")
            sock_file = self.sock.makefile('rb')
            buffered = io.BufferedReader(sock_file)
            self.rtcm_reader = RTCMReader(buffered)

        except Exception as e:
            print(f"[{time.strftime('%H:%M:%S')}] Erreur de connexion : {e}")
            if self.sock:
                self.sock.close()
            exit(1)

    def read_rtk(self):
        """Lire une trame RTCM (décodée et brute)."""
        try:
            msg = next(self.rtcm_reader)
            if msg:
                raw_data, decoded_msg = msg
                return decoded_msg, raw_data
            else:
                return None, None
        except StopIteration:
            print(f"[{time.strftime('%H:%M:%S')}] Fin du flux RTCM.")
            return None, None
        except Exception as e:
            print(f"[{time.strftime('%H:%M:%S')}] Erreur lors de la lecture RTCM : {e}")
            return None, None

    def close_connections(self):
        """Ferme proprement la connexion socket."""
        if self.sock:
            self.sock.close()
            print(f"[{time.strftime('%H:%M:%S')}] Connexion fermée.")

def main():
    rtk = RTK(
        mountpoint="NORT",  # Vérifie bien ce point de montage sur https://centipede.fr
        username="centipede",
        password="centipede"
    )

    rtk.connect_ntrip()

    for _ in range(5):  # Essaye de lire 5 trames
        decoded_msg, raw_data = rtk.read_rtk()
        if decoded_msg:
            print(f"[{time.strftime('%H:%M:%S')}] Message RTCM décodé : {decoded_msg}")
            print(f"[{time.strftime('%H:%M:%S')}] Trame brute : {raw_data.hex()}")
        else:
            print(f"[{time.strftime('%H:%M:%S')}] Aucune trame RTCM valide reçue.")
        time.sleep(1)

    rtk.close_connections()

if __name__ == "__main__":
    main()
