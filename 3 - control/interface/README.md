# Interface de Communication XBee & RTK

## Description
Ce projet est une interface graphique développée en **Python** avec **Tkinter**, permettant la communication entre un module **XBee** et un récepteur **RTK**. L'interface affiche et envoie des données via XBee, tout en relayant les corrections RTK.

## Fonctionnalités
- **Initialisation de la connexion XBee** :
  - Configure le port série pour la communication XBee.
  - Envoie des données sous forme de paires clé-valeur.
- **Connexion au serveur RTK** :
  - Initialise et établit la connexion au serveur NTRIP.
  - Récupère les corrections RTK et les envoie via XBee.
- **Interface graphique avec Tkinter** :
  - Charge une configuration YAML pour structurer l'interface.
  - Crée dynamiquement des champs d'entrée et de sortie.
  - Met à jour les champs automatiquement à la réception de nouvelles données.

## Installation
### Prérequis
- Python 3
- Modules Python nécessaires (installer avec pip) :
  ```bash
  pip install pyserial pyyaml pyrtcm
  ```

## Utilisation
Lancer le programme en précisant les paramètres :
```bash
python3 interface.py port=/dev/ttyUSB0 baud_rate=9600 path=interface_config/interface.yaml
```

### Arguments disponibles :
| Argument      | Description                          | Valeur par défaut |
|--------------|----------------------------------|----------------|
| `port`       | Port série du module XBee       | `/dev/ttyUSB0` |
| `baud_rate`  | Baud rate de la communication XBee | `9600`         |
| `path`       | Fichier YAML de configuration   | `interface_config/corrector_config.yaml` |

