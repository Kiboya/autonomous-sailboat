import socket
import base64
from pyrtcm import RTCMReader
import time
import io
import serial
import argparse
import sys

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
                sys.exit(1)

            print(f"[{time.strftime('%H:%M:%S')}] Connexion acceptée. Démarrage du lecteur RTCM...")
            sock_file = self.sock.makefile('rb')
            buffered = io.BufferedReader(sock_file)
            self.rtcm_reader = RTCMReader(buffered)

        except Exception as e:
            print(f"[{time.strftime('%H:%M:%S')}] Erreur de connexion : {e}")
            if self.sock:
                self.sock.close()
            sys.exit(1)

    def read_rtk(self):
        try:
            msg = next(self.rtcm_reader)
            if msg:
                raw_data, _ = msg
                return raw_data
            else:
                return None
        except Exception as e:
            print(f"[{time.strftime('%H:%M:%S')}] Erreur lecture RTCM : {e}")
            return None

    def close_connections(self):
        if self.sock:
            self.sock.close()
            print(f"[{time.strftime('%H:%M:%S')}] Connexion NTRIP fermée.")


def main():
    parser = argparse.ArgumentParser(description="RTCM to USB Serial Forwarder")
    parser.add_argument('--port', required=True, help='Port série USB (ex: /dev/ttyUSB0)')
    parser.add_argument('--baudrate', default=115200, type=int, help='Vitesse du port série (default: 9600)')
    args = parser.parse_args()

    try:
        serial_conn = serial.Serial(args.port, args.baudrate, timeout=1)
        time.sleep(2)
        print(f"[{time.strftime('%H:%M:%S')}] Port série {args.port} ouvert à {args.baudrate} bauds.")
    except Exception as e:
        print(f"Erreur ouverture port série : {e}")
        sys.exit(1)

    rtk = RTK()
    rtk.connect_ntrip()

    try:
        while True:
            raw_data = rtk.read_rtk()
            if raw_data:
                # msg = raw_data.hex().encode()
                msg = raw_data
                serial_conn.write(msg)
                # serial_conn.flush()
                print(f"[{time.strftime('%H:%M:%S')}] {len(raw_data)} octets RTCM envoyés sur {args.port}")
                print(msg)
            time.sleep(0.5)
    except KeyboardInterrupt:
        print("\nArrêt manuel par l'utilisateur.")
    finally:
        rtk.close_connections()
        serial_conn.close()
        print("Fermeture du port série.")

if __name__ == "__main__":
    main()



# import serial
# import time

# # Ouvre le port série à 9600 bauds
# ser = serial.Serial('/dev/ttyUSB0', 9600)
# time.sleep(2)  # Donne un peu de temps pour que le port s'initialise

# # Envoie la chaîne "20\n"
# while True:    
    # data = b"20\n"

    # ser.write(data)
    # ser.flush()

    # print("Envoi de " + str(data) + " sur /dev/ttyUSB0 terminé.")
    # time.sleep(2)  # Donne un peu de temps pour que le port s'initialise
