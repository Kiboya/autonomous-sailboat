# Données capteurs

## GPS

Relié à la pico

GPS chip to pico pi en I2C avec la correction RTK déjà appliqué

Fonctionnement :

Le PC sur la berge envoie des correctifs RTK à une Xbee 1 synchronisée avec une Xbee 2.

→ La Xbee 2 envoie correctifs à GPS via liaison série.

→ Le GPS corrige ses données (longitude, latitude, altitude, …).

→ Le GPS renvoie les données corrigées via i2c à la pico.

On a supposé : peut être que l’altitude pourra être utilisé pour detecter des écart de mesure important qui pourrait indiquer que la série de mesure est peut-être fausse. (non exploré).

## Centrale inertielle

Connecté en I2C avec la carte Raspberry PI, sur le même bus avec un adresse différente.

Renvoie des angles en degrés : pour les inclinaison de l’appareil sur le plan horizontale (avant(-90) arrière(+90) et vertical (sur les cotés à gauche -90 à droite +90). 

Renvoie la rotation sur X : angles par rapport au nord (direction du bateau).

La boussole renvoie : valeur entre 0 et 360 (nord = 0 dans le sens d’une aiguille d’une montre).

L'accélération peut-être obtenue en fonction de la gravité (en G). A voir si on l’utilise pour intégrer et retourner la vitesse

## Girouette
