# Commandes & Contrôles

Voici la liste des contrôles pour piloter votre vaisseau. Le jeu supporte le Clavier via la SDL2.

## Menu Principal

| Action | Entrée | Description |
|:-------|:-------|:------------|
| **Saisir IP** | Clavier | Tapez l'adresse IP du serveur. [cite_start]Utilisez `Backspace` pour corriger. |
| **Lancer** | Clic Gauche | [cite_start]Cliquez sur le bouton "Start Game" pour rejoindre la partie. |

## En Jeu (Vaisseau)

[cite_start]Les contrôles de déplacement supportent à la fois les flèches directionnelles et le format ZQSD/WASD, mappés via les Scancodes SDL.

| Action | Touche (Alternative) | Touche (Classique) |
|:-------|:---------------------|:-------------------|
| **Haut** | `W` (ou Z selon clavier) | Flèche Haut `↑` |
| **Bas** | `S` | Flèche Bas `↓` |
| **Gauche** | `A` (ou Q selon clavier) | Flèche Gauche `←` |
| **Droite** | `D` | Flèche Droite `→` |
| **Tirer** | `Espace` | `Espace` |

### Détails du Gameplay
* **Mouvement :** Le vaisseau a une vitesse fixe. [cite_start]Les diagonales sont normalisées pour ne pas aller plus vite en diagonale.
* [cite_start]**Animation :** Le vaisseau s'incline visuellement vers le haut ou le bas selon votre direction verticale (changement de frame du sprite).
* **Tir :** Maintenez ou appuyez sur `Espace` pour tirer des projectiles. [cite_start]Une cadence de tir (Cooldown) limite la fréquence des tirs.