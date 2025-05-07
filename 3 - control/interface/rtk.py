import socket
import base64
from pyrtcm import RTCMReader
import time

class RTK:
    def __init__(self, ntrip_host = "caster.centipede.fr", ntrip_port = 2101, mountpoint = "NORT", username = "centipede", password = "centipede"):
        # Paramètres NTRIP
        self.ntrip_host = ntrip_host
        self.ntrip_port = ntrip_port
        self.mountpoint = mountpoint
        self.username = username
        self.password = password

        

        # Encodage de l'authentification en base64
        self.auth = f"{self.username}:{self.password}"
        self.auth_b64 = base64.b64encode(self.auth.encode()).decode()

        # Initialisation du socket et du RTCMReader
        self.sock = None
        self.rtcm_reader = None

    def connect_ntrip(self):
        """ Se connecter au serveur NTRIP """
        request = f"GET /{self.mountpoint} HTTP/1.0\r\n"
        request += "User-Agent: NTRIP PythonClient\r\n"
        request += f"Authorization: Basic {self.auth_b64}\r\n"
        request += "\r\n"

        print(f"[{time.strftime('%H:%M:%S')}] Connexion à {self.ntrip_host}:{self.ntrip_port}...")

        # Connexion au serveur NTRIP
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.settimeout(5)

        try:
            self.sock.connect((self.ntrip_host, self.ntrip_port))
            self.sock.send(request.encode())

            # Lecture de la réponse
            response = self.sock.recv(1024).decode(errors="ignore")
            print(f"[{time.strftime('%H:%M:%S')}] Réponse du serveur :\n{response}")

            if "ICY 200 OK" not in response:
                print(f"[{time.strftime('%H:%M:%S')}] Erreur : Connexion refusée")
                self.sock.close()
                exit(1)

            print(f"[{time.strftime('%H:%M:%S')}] Connexion réussie ! Réception des trames RTCM...")
            self.rtcm_reader = RTCMReader(self.sock.makefile('rb'))  # Création du lecteur RTCM

        except Exception as e:
            print(f"[{time.strftime('%H:%M:%S')}] Erreur : {e}")
            if self.sock:
                self.sock.close()
            exit(1)

    def read_rtk(self):
        """ Lire une trame RTCM et la renvoyer """
        try:
            msg = next(self.rtcm_reader)  # Récupère une trame RTCM complète
            if msg:
                raw_data = msg[0]  # Données brutes RTCM
                decoded_msg = msg[1]  # Données décodées

                # Renvoie le message décodé
                return decoded_msg, raw_data
            else:
                return None, None
        except StopIteration:
            print(f"[{time.strftime('%H:%M:%S')}] Fin du flux RTCM")
            return None, None
        except Exception as e:
            print(f"[{time.strftime('%H:%M:%S')}] Erreur lors du décodage RTCM : {e}")
            return None, None

    def close_connections(self):
        """ Fermer les connexions ouvertes """
        if self.sock:
            self.sock.close()


def main():
    # # Paramètres de connexion NTRIP
    # ntrip_host = "caster.centipede.fr"
    # ntrip_port = 2101
    # mountpoint = "NATS"
    # username = "centipede"
    # password = "centipede"

    # Initialiser l'objet RTK
    # rtk = RTK(ntrip_host, ntrip_port, mountpoint, username, password)
    rtk = RTK()

    # Connexion au serveur NTRIP
    rtk.connect_ntrip()

    # Exemple de boucle qui appelle read_rtk pour obtenir les trames
    for _ in range(5):  # Par exemple, récupérer 5 trames
        decoded_msg, raw_data = rtk.read_rtk()
        if decoded_msg:
            print(f"Message RTCM décodé : {decoded_msg}")
            print(f"Données brutes RTCM : {raw_data.hex()}")
        time.sleep(1)  # Attente entre les appels pour ne pas saturer le processeur

    # Fermer les connexions
    rtk.close_connections()

if __name__ == "__main__":
    main()