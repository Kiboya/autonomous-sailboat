# Implémentation basée sur des règles

DirectionFunction : 

Arguments : sailboatLocation, windDirection, sailboatDirection, 

Return : strategy for direction

![IMG_3562.HEIC](src/pathscheme.heic)

### **Algorithme : Calculer la direction à suivre pour rejoindre une balise**

### **Entrées :**

1. **Position GPS actuelle du bateau** : (latitude, longitude).
2. **Orientation actuelle du voilier** : angle en degrés par rapport au nord (fournie par la boussole).
3. **Position GPS de la balise cible** : (latitude, longitude).
4. **Direction du vent** : angle en degrés par rapport au nord.

### **Sortie :**

- Direction optimale à suivre pour rejoindre la balise, exprimée en degrés par rapport au nord.

---

### **Étapes :**

1. **Calculer l'azimut vers la balise** :
    - Convertir les coordonnées GPS actuelles et celles de la balise en radians.
    - Utiliser la formule de l'azimut pour calculer la direction en degrés depuis la position actuelle vers la position de la balise
    
    - Δlong=longitudebalise−longitudebateau
    - x=sin(Deltatextlong)timescos(textlatitudebalise)x=sin(Deltatextlong)timescos(textlatitudebalise)
    - y=cos(textlatitudebateau)timessin(textlatitudebalise)−sin(textlatitudebateau)timescos(textlatitudebalise)timescos(Deltatextlong)y=cos(textlatitudebateau)timessin(textlatitudebalise)−sin(textlatitudebateau)timescos(textlatitudebalise)timescos(Deltatextlong)
    - textazimut=textatan2(x,y)textazimut=textatan2(x,y) (converti en degrés et normalisé entre 0° et 360°).
2. **Définir la zone d’interdiction de navigation face au vent** :
    - Fixer une zone de 45° de part et d’autre de la direction du vent où le bateau ne peut pas naviguer.
        - Par exemple, si le vent vient de 90° (est), la zone d’interdiction est entre 45° et 135°.
3. **Vérifier si l’azimut calculé est dans la zone d’interdiction** :
    - Si l’azimut vers la balise est dans la zone interdite :
        - Déterminer les bords de la zone interdite (ex. 45° et 135°).
        - Choisir une route alternative :
            - Soit en visant juste à l’extérieur de la zone interdite sur bâbord (à gauche du vent).
            - Soit en visant juste à l’extérieur sur tribord (à droite du vent).
            - La décision dépend de la proximité des bords de la zone par rapport à l’azimut initial.
4. **Retourner la direction optimale** :
    - Si l’azimut est en dehors de la zone interdite : retourner l’azimut vers la balise.
    - Sinon : retourner l’un des angles juste à l’extérieur de la zone interdite.

---

### **Exemple illustratif** :

### Entrées :

- Position actuelle : (48.8566, 2.3522) (Paris).
- Position de la balise : (48.8570, 2.3600).
- Direction du vent : 90° (vent venant de l'est).
- Orientation du bateau : 45°.

### Étapes exécutées :

1. Calcul de l'azimut vers la balise : disons que l'azimut est 100°.
2. Vérification de la zone interdite : la zone interdite est entre 45° et 135°.
3. L'azimut 100° est dans la zone interdite.
    - Choix d’une route alternative : viser 135° (bord tribord).
4. Résultat : retourner 135° comme direction à suivre.

[Données capteurs](https://www.notion.so/Donn-es-capteurs-18bc0536739e809ea75df23d55019de1?pvs=21)