# Données capteurs

## GPS

Relié à la pico

GPS chip to pico pi en I2C avec la correction RTK déjà appliqué

Fonctionnement :

PC sur la berge envoie correctifs RTK à Xbee

→ Xbee envoie correctifs à GPS via liaison série

→ GPS corrige ses données (longitude, latitude, altitude, …)

→ GPS renvoie les données corrigées via i2c à la pico

On a pensé : peutêtre que l’altitude pourra être utilisé pour detecter des écart de mesure important qui pourrait indiquer que la série de mesure est peut-être fausse

## Centrale inertielle

I2C avec la carte Raspberry PI, sur le même bus avec un adresse différente.

Angle en degrés : pour les inclinaison de l’appareil sur le plan horizontale (avant(-90) arrière(+90) et vertical (sur les cotés à gauche -90 à droite +90). 

Rotation sur X : angles par rapport au nord (direction du bateau) 

Boussole : valeur entre 0n et 360 (nord = 0 dans le sens d’une aiguille d’une montre)

Accélération peut-être obtenue en fonction de la gravité (en G). A voir si on l’utilise pour intégrer et retourner la vitesse

## Girouette