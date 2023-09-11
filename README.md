# 1924B - Mini Boite Noire
> SYSTÈME D’ENREGISTREMENT DE TRAJECTOIRES DE VOL

## Table of Contents
* [Information générales](#information-générales)
* [Technologies utilisées](#technologies-utilisées)
* [Fonctionnalités](#fonctionnalités)
* [Configuration](#configuration)
* [Utilisation](#utilisation)
* [Statut du projet](#statut-du-projet)
* [Améliorations](#améliorations)
* [Contact](#contact)
<!-- * [License](#license) -->


## Information générales
le présent projet a pour objectif la collecte et le stockage des données de
mesures et de localisation d’un aéronef au moyen d’une centrale inertielle et d’un système
de positionnement GPS/GNSS. Par le biais de la conjonction de ces technologies, il est envisageable
d’enregistrer des données d’une précision remarquable concernant les paramètres du vol
et la trajectoire empruntée par l’aéronef. En cas de survenue d’un incident, ces enregistrements
jouent un rôle déterminant en permettant d’établir les causes potentielles. Pour une analyse
approfondie des spécifications techniques, veuillez consulter le cahier des charges complet disponible
en annexe.
<!-- You don't have to answer all the questions - just the ones relevant to your project. -->


## Technologies utilisées
- MPLAB Harmony v2_06
- Microcontrôleur - PIC32MX274F256D
- Centrale inertielle - Bosch BNO055
- GNSS - CAM-M8C-0


## Fonctionnalités
• Sauvegarde des données inertielles chaque 500ms par défaut.
• Sauvegarde des données de localisation chaque 5’000ms par défaut.
• Possibilité de configurer les temps de sauvegarde.
• Résistance aux chocs.
• Bonne autonomie / Low power.
• Global Positioning System. (GPS)
• Global navigation satellite systems. (GNSS).
• Timestamp par satellite.
• Centrale inertielle :
- Accéléromètre 3-axes.
- Gyroscope 3-axes.
• Charge, lecture et config. par USB-C.


## Configuration
La carte éléctronique doit être assemblée en considération des fichiers de fabrication et de la BOM (Bill of materials). Le firmware doit ensuite être implémenté dans microcntrôleur PIC et une carte SD doit être insérée, afin de lire les données de déplacement.


## Utilisation
Il faut flâcher le code firmware par le bias de MPLAB-X avec harmony sur le microcontroleur du PCB produit, pour ce faire, il y a un connecteur de programmation BERG. Une fois le code flashé, le module sous-marin peut être utilisé et sous condition qu'une carte SD est insérée, les données de déplacements y seront sauvegardé en format CSV.


## Statut du projet
Le projet est complété mais des améliorations peuvent toujours avoir lieu.


## Contact
Créer par Ali Zoubir : ali.zoubir@etml-es.ch

