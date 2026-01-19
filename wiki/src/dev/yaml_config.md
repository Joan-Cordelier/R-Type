# Système de Configuration YAML

Le jeu R-Type utilise des fichiers YAML pour configurer tous les aspects du gameplay. Ce système permet de modifier le comportement du jeu sans recompiler.

## Structure des Fichiers

```
yaml/
├── main_loop.yaml      # Configuration principale du jeu
├── upgrades.yaml       # Système d'améliorations du joueur
└── enemies/            # Configurations des boss
    ├── obelisk.yaml
    ├── blaster.yaml
    └── pata_pata.yaml
```



## main_loop.yaml

Fichier principal contenant la configuration de la boucle de jeu, les ennemis et les niveaux.

### Game Loop

```yaml
game_loop:
  tick_rate: 30                    # Ticks par seconde
  max_messages_per_frame: 100      # Messages réseau max par frame
  min_players_to_start: 1          # Joueurs minimum pour démarrer
```

### Difficulté

```yaml
difficulty:
  easy: 0.5          # Multiplicateur de difficulté
  normal: 1.0
  hard: 1.5
  impossible: 3.0
```

### Types d'Ennemis

```yaml
enemy_types:
  basic_enemy:
    sprite: "textures/ships/enemy_ship.png"
    texture_id: "enemy_ship"
    health: 50
    damage: 10
    speed: 100
    score: 100
    width: 50.0
    height: 50.0
    velocity_x: 0.0
    velocity_y: 100.0
    move_amplitude: 100.0
    move_frequency: 0.5
```

### Configuration des Niveaux

```yaml
levels:
  - id: 1
    name: "Zone 1 - Asteroid Belt"
    background: "textures/backgrounds/level1.png"
    difficulty_multiplier: 1
    waves:
      - id: 1
        start_delay: 1.0
        groups:
          - type: "basic_enemy"    # Référence à enemy_types
            count: 3
            interval: 0.8
      - id: 2
        start_delay: 2.0
        boss: "obelisk"            # Invoque un boss (voir enemies/)
        groups: []
```

### Configuration du Joueur

```yaml
player:
  initial_position:
    x: 100.0
    y: 300.0
  hitbox:
    width: 30.0
    height: 30.0
    offset_x: 45.0
    offset_y: 50.0
    sprite_width: 120.0
    sprite_height: 130.0
  stats:
    health: 100
    max_health: 100
    attack_damage: 10
    attack_speed: 10
    defense: 1
    speed: 200
```

### Projectiles

```yaml
projectiles:
  player:
    speed: 400.0
    offset_x: 52.0
    offset_y: 30.0
  enemy:
    speed: 200.0
    offset_x: 18.0
    offset_y: 50.0
```



## upgrades.yaml

Système d'améliorations présentées au joueur entre les vagues.

### Structure d'une Amélioration

```yaml
upgrades:
  - id: "move_speed_1"           # ID unique
    name: "Propulseurs Mk.I"     # Nom affiché
    description: "Augmente la vitesse de deplacement de 15%."
    type: "stat_boost"           # Type: stat_boost ou hybrid
    target: "movement_speed"     # Stat ciblée
    value: 15.0                  # Valeur du boost
    rarity: "common"             # Rareté: common/rare/epic/legendary
    visuals:
      card_color: "blue"         # Couleur de la carte
```

### Types d'Améliorations

| Type | Description |
||-|
| `stat_boost` | Modifie une seule stat |
| `hybrid` | Modifie plusieurs stats (utilise `effects`) |
| `add_weapon` | Ajoute une arme/drone |

### Targets Disponibles

| Target | Description |
|--|-|
| `movement_speed` | Vitesse de déplacement |
| `max_health` | Points de vie maximum |
| `current_health_percent` | Restaure un % des PV |
| `damage_multiplier` | Multiplicateur de dégâts |
| `fire_rate` | Cadence de tir |
| `extra_projectiles` | Projectiles supplémentaires |
| `add_diagonal` | Active les tirs diagonaux |
| `projectile_scale` | Taille des projectiles |

### Exemple Hybrid

```yaml
- id: "projectile_size_up"
  name: "Chargeur Plasma"
  type: "hybrid"
  effects:
    - target: "projectile_scale"
      value: 0.5
    - target: "damage_multiplier"
      value: 0.10
  rarity: "epic"
```



## enemies/*.yaml (Boss)

Chaque boss est défini dans un fichier séparé avec phases et patterns d'attaque.

### Structure d'un Boss

```yaml
enemies:
  - id: "obelisk"
    name: "Ancient Obelisk"
    type: "boss"
    
    stats:
      health: 1000
      max_health: 1000
      health_per_player: 500    # PV ajoutés par joueur supplémentaire
      speed: 5.0
      collision_damage: 20
      score_value: 8000
    
    visuals:
      texture_path: "textures/ships/bosses/Obelisk.png"
      scale: 2
      width: 190
      height: 240
      animations:
        idle: { ... }
        channeling: { ... }
    
    behavior:
      phases: [ ... ]
      attack_patterns: [ ... ]
```

### Phases du Boss

```yaml
phases:
  - id: 1
    name: "Awakening"
    trigger_health_percentage: 100    # Déclenché à 100% HP
    patterns: ["enter_screen"]
    
  - id: 2
    name: "Zone_Control"
    trigger_health_percentage: 99
    loop_patterns: true               # Répète les patterns
    patterns:
      - "spawn_void_zones"
      - "energy_pulse"
      - "screen_shake"
```

### Types de Patterns d'Attaque

```yaml
attack_patterns:
  # Spawner de zones de dégâts
  - id: "spawn_void_zones"
    type: "area_spawner"
    count: 3
    duration: 3.0
    damage: 15
    warning_time: 1.5
    texture_override: "textures/..."
    
  # Ring de projectiles
  - id: "energy_pulse"
    type: "projectile_ring"
    projectile_id: "boss_orb"
    count: 8
    speed: 200.0
    
  # Pause entre attaques
  - id: "wait_medium"
    type: "wait"
    duration: 2.0
```



## Ajouter du Contenu

### Ajouter un Ennemi

1. Ajouter l'entrée dans `enemy_types` de `main_loop.yaml`
2. Référencer le type dans une `wave.groups`

### Ajouter un Boss

1. Créer `yaml/enemies/mon_boss.yaml`
2. Définir stats, visuals, phases et patterns
3. Référencer avec `boss: "mon_boss"` dans une wave

### Ajouter une Amélioration

1. Ajouter l'entrée dans `upgrades.yaml`
2. Le jeu piochera automatiquement parmi les upgrades disponibles



## API C++ (GameLoopConfig)

Le code charge les fichiers YAML via la classe `GameLoopConfig`:

```cpp
GameLoopConfig config;
config.loadFromFile("yaml/main_loop.yaml");
config.loadUpgradesFromFile("yaml/upgrades.yaml");
config.loadBossesFromDirectory("yaml/enemies");

// Accès aux données
const auto& player = config.getPlayerConfig();
const auto& enemies = config.getEnemyTypes();
const auto& levels = config.getLevels();
const auto& upgrades = config.getUpgrades();
const auto& bosses = config.getBosses();
```
