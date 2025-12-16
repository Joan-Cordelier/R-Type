# Étude Comparative & Choix Techniques (Benchmark)

Ce document détaille et justifie les décisions architecturales prises lors du développement du moteur R-Type. Chaque choix résulte d'une analyse comparative face aux alternatives standards de l'industrie.

## 1. Langage : C++17

Le choix du langage est critique pour un moteur de jeu où chaque milliseconde compte.

| Critère | C++17 (Notre Choix) | Java / C# | Python | Rust |
|:---|:---|:---|:---|:---|
| **Performance** | Native (Compilé) | Bytecode (JIT) | Interprété (Lent) | Native |
| **Gestion Mémoire** | Manuelle (RAII) | Garbage Collector (GC) | Garbage Collector | Borrow Checker |
| **Latence** | Déterministe | "Stop-the-world" (GC) | Élevée | Déterministe |
| **Écosystème Jeu** | Standard (Unreal, etc.) | Moyen (Unity/C#) | Faible (Prototypage) | Émergent (Bevy) |

**Justification Technique :**
Nous avons écarté **Java** et **Python** car leur *Garbage Collector* introduit des pauses imprévisibles incompatibles avec la contrainte des 60 FPS (16ms par frame). [cite_start]Bien que **Rust** soit un candidat sérieux pour la sécurité mémoire, **C++17** a été retenu pour sa maturité, sa bibliothèque standard enrichie et sa gestion explicite de la mémoire via les *smart pointers* (`std::shared_ptr`, `std::unique_ptr`) que nous utilisons massivement dans notre `Registry` pour gérer les composants [cite: 388-389].

## 2. Architecture : ECS vs OOP Classique

L'architecture logicielle définit la maintenabilité et la performance du jeu.

### Approche OOP (Object Oriented Programming)
*Structure :* `class Player : public Spaceship : public Entity`
* **Problème :** Héritage multiple complexe (Diamond Problem) et rigidité.
* **Performance :** Les objets sont dispersés dans le tas (Heap), entraînant de nombreux *Cache Misses*.

### Approche ECS (Entity Component System) - Notre Choix
*Structure :* Composition. [cite_start]Une entité est un simple identifiant [cite: 380-382]. Les données sont stockées dans des structures dédiées.
* **Architecture :** Nous utilisons des `ComponentArray` basés sur des tables de hachage (`std::unordered_map`) pour associer les données aux entités[cite: 379].
* **Modularité :** Ajouter une fonctionnalité (ex: `Shield`) se fait sans modifier la classe `Player`, simplement en attachant un nouveau composant à l'exécution via `registry.addComponent`[cite: 391].

## 3. Protocole Réseau : Hybride TCP/UDP

Le réseau est le goulot d'étranglement principal d'un jeu multijoueur.

### Le Problème du "Head-of-Line Blocking" (TCP)
TCP garantit que les paquets arrivent dans l'ordre. Si le paquet 1 est perdu, le paquet 2 (déjà arrivé) doit attendre la retransmission du 1. Cela crée un délai soudain (lag spike) inacceptable pour le temps réel.

### Notre Stratégie Hybride

| Protocole | Usage dans R-Type | Justification |
|:---|:---|:---|
| **UDP** | Gameplay (Position, Tirs) | **Vitesse > Fiabilité**. [cite_start]Nous utilisons `UDPClient` et `UDPServer` pour les flux rapides [cite: 261-262]. Si un paquet de position est perdu, le suivant arrivera peu après. |
| **TCP** | Contrôle (Login, Score) | **Fiabilité > Vitesse**. [cite_start]Pour les actions critiques comme la connexion (`CONNECT`) ou la mort (`DEATH`), nous utilisons `TCPClient` pour garantir la réception [cite: 236-237]. |

## 4. Bibliothèques Graphiques : SDL2 vs SFML

Pour le rendu client, nous avons comparé les deux leaders du marché C++.

* **SFML :** Orientée Objet, API C++ simple.
* **SDL2 :** API C, bas niveau, standard industriel.

**Choix : SDL2**
Nous avons choisi SDL2 pour sa robustesse et sa gestion fine des événements systèmes et des textures via `SDL_Renderer`. [cite_start]Elle nous a permis de structurer proprement notre propre couche d'abstraction C++ (`src/engine/graphic/Renderer.cpp`) qui gère le cache de textures et de polices [cite: 737-741].

## 5. Sérialisation : Binaire Custom vs JSON

* **JSON :** Verbeux et lent à parser.
* **Binaire Custom :** Compact et rapide.

**Choix : Protocole Binaire**
Notre `MessageFactory` encode les données directement en octets (Big Endian).
* *Exemple :* Une position (X, Y) prend exactement **8 octets** (2 floats de 4 octets) dans notre protocole [cite: 323-324], contre ~30 octets en JSON.
* **Gain :** Réduction drastique de la bande passante et sérialisation instantanée via `LinearBuffer` [cite: 263-264].