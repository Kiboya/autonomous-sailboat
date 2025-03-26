import tkinter as tk
import yaml
import sys
import threading
import time
from xbee import Xbee
from rtk import RTK  # Assurez-vous que la classe RTK est bien importée
import threading

class Interface:
    def __init__(self, yaml_config, port, baud_rate):
        self.yaml_config = yaml_config
        self.entries = {}
        self.checkbox_vars = {}
        self.xbee_lock = threading.Lock()  # 🔒 Verrou pour éviter les conflits d'écriture

        self.init_xbee(port, baud_rate)
        self.init_rtk()

        # Création de l'interface à partir de la configuration
        self.create_interface()

        # Lancer un thread pour recevoir les messages de XBee
        self.receive_thread = threading.Thread(target=self.receive_data_from_xbee, daemon=True)
        self.receive_thread.start()

        # Lancer un thread pour envoyer les corrections RTK
        self.rtk_thread = threading.Thread(target=self.send_rtk_corrections, daemon=True)
        self.rtk_thread.start()

    def init_xbee(self, xbee_port, xbee_baud_rate):
        """Initialiser la connexion XBee"""
        self.xbee = Xbee(xbee_port, xbee_baud_rate)

    def init_rtk(self):
        """Initialiser la connexion au serveur RTK"""
        self.rtk = RTK()
        self.rtk.connect_ntrip()

    def create_interface(self):
        """Création de la fenêtre principale"""
        self.frame = tk.Tk()
        self.frame.title(self.yaml_config["Config"]["Title"])

        row = 0
        containers = self.yaml_config["Config"]["Containers"]

        for container in containers:
            container_frame = tk.LabelFrame(self.frame, text=container["Name"], padx=10, pady=10)
            container_frame.grid(row=row, column=0, sticky="w", padx=5, pady=5)
            row += 1

            if "outputs" in container:
                for name, value in container["outputs"].items():
                    self.create_output_param(container_frame, name, value)

            if "inputs" in container:
                for name in container["inputs"]:
                    self.create_input_param(container_frame, name)

    def create_input_param(self, container_frame, name):
        """Créer un champ d'entrée (lecture seule)"""
        param_frame = tk.Frame(container_frame)
        param_frame.pack(fill='x', padx=5, pady=2)

        tk.Label(param_frame, text=f"{name} : ").pack(side='left')

        entry = tk.Entry(param_frame, state="readonly")
        entry.insert(0, "0")
        entry.pack(side='right', fill='x', expand=True)

        self.entries[name] = entry

    def create_output_param(self, container_frame, name, value):
        """Créer un champ de sortie"""
        param_frame = tk.Frame(container_frame)
        param_frame.pack(fill='x', padx=5, pady=2)

        tk.Label(param_frame, text=f"{name} : ").pack(side='left')

        entry = tk.Entry(param_frame)
        entry.insert(0, str(value))
        entry.pack(side='right', fill='x', expand=True)

        self.entries[name] = entry

        entry.bind("<Return>", lambda event, name=name: self.send_output_to_xbee(name))
        entry.bind("<FocusOut>", lambda event, name=name: self.send_output_to_xbee(name))

    def send_output_to_xbee(self, name):
        """Envoie une mise à jour des valeurs des outputs via XBee"""
        value = self.entries[name].get()
        with self.xbee_lock:  # 🔒 Empêche l'accès concurrent au port série
            try:
                self.xbee.send_key_value(name, value)
                time.sleep(0.05)  # Petite pause pour éviter les conflits d'écriture
            except Exception as e:
                print(f"Erreur d'envoi XBee : {e}")

    def receive_data_from_xbee(self):
        """Recevoir des données XBee et les mettre à jour dans l'interface"""
        while True:
            key, value = self.xbee.receive_key_value()
            if key and value and key in self.entries:
                self.entries[key].config(state="normal")
                self.entries[key].delete(0, tk.END)
                self.entries[key].insert(0, value)
                self.entries[key].config(state="readonly")

    def send_rtk_corrections(self):
        """Lire les corrections RTK et les envoyer en XBee"""
        while True:
            _, raw_data = self.rtk.read_rtk()
            if raw_data:
                with self.xbee_lock:  # 🔒 Sécurisation de l'accès au port série
                    try:
                        self.xbee.send_key_value("RTK",raw_data.hex())
                        time.sleep(0.05)  # Petite pause pour éviter les conflits d'écriture
                    except Exception as e:
                        print(f"Erreur d'envoi RTK : {e}")

def load_yaml_config(yaml_file):
    """Charge la configuration YAML"""
    with open(yaml_file, 'r') as file:
        return yaml.load(file, Loader=yaml.FullLoader)

def main():
    args = dict(arg.split('=') for arg in sys.argv[1:])

    path = args.get("path", "interface_config/corrector_config.yaml")
    port_xbee = args.get("port", "COM7")
    baud_rate = args.get("baud_rate", 9600)

    config = load_yaml_config(path)

    interface = Interface(config, port_xbee, baud_rate)
    interface.frame.mainloop()

if __name__ == '__main__':
    main()
