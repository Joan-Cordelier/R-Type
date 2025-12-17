# Architecture Serveur & Réseau

L'architecture réseau du projet R-Type est conçue pour être **asynchrone**, **multithreadée** et **non-bloquante**. Elle sépare strictement la gestion des sockets (I/O) de la logique de jeu (Game Loop) pour garantir que le lag réseau ne ralentisse pas la simulation.



[Image of server network architecture diagram]


## 1. Composants Principaux

### 1.1 SessionManager
C'est le point d'entrée du serveur. Il possède et gère :
* `TCPServer` : Gère les connexions persistantes et les messages critiques.
* `UDPServer` : Gère les flux de données rapides (positions).
* `ThreadedQueue` : La file d'attente thread-safe qui stocke les messages reçus.

### 1.2 Les Serveurs (I/O Threads)
Le `TCPServer` et l'`UDPServer` tournent chacun dans leur propre **std::thread** indépendant (`_tcpThread` et `_udpThread`).

* **Technologie :** Ils utilisent `poll()` (système UNIX) pour écouter plusieurs descripteurs de fichiers (sockets) sans bloquer le CPU.
* **Réception :** Dès qu'un paquet arrive, il est décodé par la `MessageFactory`.
* **Stockage :** Si le paquet est valide, il est poussé (`push`) dans la `ThreadedQueue` partagée.

### 1.3 MessageHandler (Le Pont)
Le `MessageHandler` fait le lien entre le réseau et le jeu. Il est appelé par la boucle de jeu principale.
* Il dépile (`pop`) les messages de la `ThreadedQueue`.
* Il traite les messages selon leur priorité (CRITICAL > HIGH > MEDIUM > LOW).
* Il exécute l'action associée (ex: appeler `GameHandler` pour créer un joueur).

### 1.4 GameHandler (La Boucle de Jeu)
C'est le thread principal ("Main Thread"). Il ne touche jamais directement aux sockets.
1.  **Process Messages :** Il demande au `MessageHandler` de traiter les inputs reçus.
2.  **Update ECS :** Il met à jour les Systèmes (Mouvement, Collision).
3.  **Broadcast :** Il demande au `SessionManager` d'envoyer les mises à jour aux clients.

---

## 2. Flux de Données (Data Flow)

### Réception d'un message (Client -> Serveur)
1.  **Client** envoie un paquet UDP `MOVE`.
2.  **UDPServer Thread** détecte l'activité via `poll()`.
3.  **UDPServer** lit le buffer, le désérialise en `DecodedMessage`.
4.  **UDPServer** pousse le message dans la `ThreadedQueue` (partagée).
5.  **Main Thread** (dans `GameHandler`) appelle `pop()`.
6.  **MessageHandler** identifie `OpCode::MOVE` et met à jour la vélocité dans le `Registry` (ECS).

### Envoi d'un message (Serveur -> Client)
1.  **GameHandler** décide qu'un joueur a explosé.
2.  Il appelle `sessionManager.broadcastTcp(DEATH_MSG)`.
3.  **SessionManager** place le message dans la `OutgoingQueue` du `TCPServer`.
4.  **TCPServer Thread** dépile le message et l'écrit dans le socket via `send()`.

---

## 3. Gestion des Clients (Session)

La classe `Player` représente une session utilisateur.

* **Mapping TCP/UDP :** Chaque joueur est identifié par son socket TCP (`tcpFd`). Une fois connecté, le serveur attend le premier paquet UDP pour associer l'adresse IP/Port UDP au joueur (`udpLinked`).
* **Reconnexion :** Le système permet de gérer les déconnexions via le callback `setOnDisconnect` du `TCPServer`, nettoyant ainsi les entités ECS associées pour éviter les fantômes.