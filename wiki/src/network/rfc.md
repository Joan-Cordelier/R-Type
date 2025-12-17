# RFC R-Type Protocol

**Version:** 1.0.0
**Status:** Draft
**Transport:** TCP (Control) & UDP (Gameplay)
**Encoding:** Binary (Big Endian)

## 1. Introduction

Ce document spécifie le protocole de communication utilisé par le moteur de jeu R-Type. Il s'agit d'un protocole binaire hybride utilisant TCP pour la fiabilité (connexion, lobby, événements critiques) et UDP pour la performance (mouvements, synchronisation en temps réel).

## 2. Structure des Paquets

Tous les échanges (TCP et UDP) respectent le format d'en-tête suivant.

### 2.1 En-tête (Header)

| Offset | Champ    | Type      | Taille | Description                                      |
|:-------|:---------|:----------|:-------|:-------------------------------------------------|
| 0x00   | `OPCODE` | `uint8_t` | 1 byte | Identifiant de la commande (voir section 3).     |
| 0x01   | `LENGTH` | `uint8_t` | 1 byte | Taille du Payload en octets (0 à 255).           |
| 0x02   | `PAYLOAD`| `bytes`   | N bytes| Données variables spécifiques à la commande.     |

**Note :** Si `LENGTH` est défini à 0, le paquet s'arrête après l'octet de longueur.

### 2.2 Types de Données

Sauf mention contraire, les types de données suivent ces règles :
* `uint8` : Entier non signé 8 bits.
* `uint32` : Entier non signé 32 bits (Big Endian sur le réseau).
* `float` : Flottant 32 bits (IEEE 754).
* `string` : Séquence d'octets ASCII, non terminée par null (la taille est définie par le contexte ou le header).

---

## 3. Commandes (OpCodes)

Les OpCodes sont définis dans `src/common/Data/MessageFactory.hpp`.

| OpCode | Nom           | Canal Préféré | Direction | Priorité | Description |
|:-------|:--------------|:--------------|:----------|:---------|:------------|
| `0x00` | `INCOMPLETE`  | -             | -         | -        | Interne (Buffer fragmenté) |
| `0x01` | `PARSING_ERR` | -             | -         | ERROR    | Erreur de lecture |
| `0x02` | `DEATH`       | TCP           | S -> C    | HIGH     | Notification de mort d'une entité |
| `0x03` | `MOVE`        | UDP           | Bidirect  | LOW      | Mise à jour de position |
| `0x04` | `SHOOT`       | UDP           | C -> S    | HIGH     | Action de tir |
| `0x05` | `CONNECT`     | TCP           | Bidirect  | CRITICAL | Handshake de connexion |
| `0x06` | `START`       | TCP           | S -> C    | CRITICAL | Lancement de la partie |
| `0x07` | `JOIN`        | TCP           | S -> C    | HIGH     | Assignation à une Room |
| `0x08` | `CRASH`       | TCP           | S -> C    | CRITICAL | Arrêt d'urgence serveur |
| `0x09` | `PLAYER`      | TCP           | S -> C    | MEDIUM   | Infos joueur (Score/Vie) |

---

## 4. Détail des Payloads

### 4.1 Connexion (Handshake)

**1. Requête Client (`CONNECT` - 0x05)**
* **Payload :** Vide (0 bytes).
* **Description :** Le client initie la connexion TCP.

**2. Réponse Serveur (`CONNECT` - 0x05)**
* **Payload :** `[PlayerID (4 bytes)]`
* **Description :** Le serveur confirme et attribue un ID unique au joueur.

**3. Assignation Room (`JOIN` - 0x07)**
* **Payload :** `[RoomID (1 byte)]`
* **Description :** Le serveur indique au client dans quel lobby il se trouve.

### 4.2 Gameplay

**1. Mouvement (`MOVE` - 0x03)**
* **Payload :** `[EntityID (4 bytes)]` + `[X (4 bytes float)]` + `[Y (4 bytes float)]`
* **Total :** 12 bytes.
* **Description :**
    * **C -> S :** Le client demande à bouger (contient son vecteur vitesse ou position désirée).
    * **S -> C :** Le serveur diffuse la position autoritaire de l'entité `EntityID`.

**2. Tir (`SHOOT` - 0x04)**
* **Payload :** `[EntityID (4 bytes)]`
* **Description :** Le client signale que l'entité `EntityID` effectue une action de tir. Le serveur instanciera le projectile.

**3. Mort (`DEATH` - 0x02)**
* **Payload :** `[EntityID (4 bytes)]`
* **Description :** Le serveur notifie que l'entité `EntityID` doit être détruite (explosion, disparition).