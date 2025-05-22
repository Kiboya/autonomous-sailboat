# Bateau Lavoir

### **Document de Spécifications Logiciel (SRS)**

**Projet : Bateau Lavoir**

**Groupe : concevoir, implémenter et évaluer les méthodes de planification (tâche et trajectoire) du bateau**

**Version : 1.0**

**Date : Vendredi 17 Janvier 2025**

**Auteurs : Bastien DELFOUILLOUX & Alexis LEQUEUX**

**Concurrents direct : Jean-Baptiste DE ROBILLARD DE BEAUREPAIRE & Siméon GAUDIN.**

---

### **1. Introduction**

1.1 **Objectif**

L’objectif est de développer un module logiciel permettant au voilier autonome de rejoindre une destination depuis sa position actuelle. Ce module donnera donc un itinéraire défini en temps réel en fonction des conditions environnementales et des données de navigation reçues.

1.2 **Portée**

- Fonction principale : génération et suivi d’itinéraires par rapport à un point cardinal (le nord).
- Non inclus : gestion des capteurs, gestion des actionneurs.

1.3 **Définitions et Acronymes**

- SRS : Software Requirements Specification

1.4 **Références**

---

### **2. Description Générale**

2.1 **Vue d’ensemble du système**

Le module d’itinéraire interagit avec :

- **Capteurs** : centrale inertielle, GPS et girouette.
- **Données :** Accélération, accélération angulaire, champ magnétique sur X, Y et Z obtenue via le magnétomètre, direction du vent, position.
- **Actionneurs :** safran & voile (via un treuil)

2.2 **Fonctions principales**

- Calcul de la direction à prendre par rapport au Nord.
- Mise à jour de l’itinéraire en fonction des conditions en temps réel.

2.3 **Contraintes**

- Données capteurs mise à jour en permanence (avec de fortes variations possibles). Etablir une moyenne des dernières valeurs
- Calcul rapide pour une réaction efficace de la trajectoire du bateau.
- La manière dont un voilier peut être piloté en fonction des conditions extérieur n’est pas des plus triviales (zig zag en cas de vent de face)
- Ne pas recalculer pour des variations trop faibles (seuil à définir).
- Faire la différence entre le vent météo et le vent apparent que l’on reçoit via la boussole

2.4 **Hypothèses et dépendances**

- Réponse rapide des actionneurs (≤ 1 seconde)
- Les capteurs sont calibrés correctement.

---

### **3. Exigences Fonctionnelles**

3.1 **Identification des points cardinaux**

- RF-01 : Le logiciel doit déterminer l’orientation actuelle en fonction du Nord.

3.2 **Calcul de l’itinéraire**

- RF-03 : Générer un itinéraire optimal entre deux points (WP).
- RF-04 : Mettre à jour l’itinéraire en cas de changement des conditions environnementales.

3.3 **Suivi et corrections**

- RF-05 : Détecter les écarts importants par rapport à la direction prévue.

---

### **4. Exigences Non Fonctionnelles**

4.1 **Performance**

- NFR-01 : Temps de calcul pour un itinéraire ≤ 500 ms.
- NFR-02 : Temps de réaction des actionneurs ≤ 500 ms.

---

### **5. Diagrammes et Scénarios d’Utilisation**

5.1 **Diagramme de Cas d’Utilisation**

Insérer un diagramme illustrant les interactions principales :

- Capteurs -> Module d’itinéraire -> safran et voile.

5.2 **Exemple de scénario**

**Scénario 1 : Itinéraire avec une déviation détectée**

1. Le voilier connait la balise à rejoindre (destination finale).
2. Le logiciel calcule la trajectoire optimale.
3. Le logiciel envoie la position que les actionneurs doivent prendre
4. Une rafale de vent provoque un écart.
5. Le logiciel détecte une forte déviation (exemple > 10°)
6. Le logiciel calcule de nouveau la trajectoire à suivre
7. Le logiciel envoie la position que les actionneurs doivent prendre

---

### **6. Validation et Tests**

6.1 **Tests Unitaires**

- Test de calcul de direction.

6.2 **Tests d’Intégration**

- Vérification de la communication avec les capteurs et actionneurs

6.3 **Tests de Performance**

- Validation du temps de calcul des itinéraires.
- Validation du recalcule de l’itinéraire suite à une perturbation

---

### **7. Annexes**

[Piloter la trajectoire d’un voilier](https://www.notion.so/Piloter-la-trajectoire-d-un-voilier-17ec0536739e8041b90cdf3d8490ec54?pvs=21)

[Implémentation déjà faite](https://www.notion.so/Impl-mentation-d-j-faite-185c0536739e80649261fb7d10b10f2c?pvs=21)

## Planification de trajectoire

[Méthodes possibles ](https://www.notion.so/M-thodes-possibles-00dd5d558f7c4f62bcd66430d3e8a76e?pvs=21)

[Conseils d’Apo](https://www.notion.so/Conseils-d-Apo-1bfdf0bec9dc4c7daf630b7e4f322df2?pvs=21)

### Données d’entrainement

[Pistes](https://www.notion.so/Pistes-253eecc4ae234dc1bb0382f5348742e6?pvs=21)

[Implémentation basée sur des règles](https://www.notion.so/Impl-mentation-bas-e-sur-des-r-gles-189c0536739e806bbf29e2c0896a549e?pvs=21)