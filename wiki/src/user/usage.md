# Lancer le Jeu

Le R-Type est un jeu client-serveur. Vous devez lancer un serveur central avant de pouvoir connecter des clients.

---

## 1. Lancer le Serveur

Le serveur gère la logique du jeu, les salons et synchronise les joueurs.

### Mode Normal

```bash
# Linux
./build/r-type_server
```

Vous devriez voir :
```
TCP server listening on port 4789
UDP server listening on port 4789
```

### Mode Admin (Console Interactive)

Pour accéder à la console d'administration avec des commandes de gestion :

```bash
./build/r-type_server -a
```

En mode admin, vous pouvez utiliser les commandes suivantes :

| Commande | Description |
|----------|-------------|
| `help` | Affiche toutes les commandes disponibles |
| `rooms` | Liste tous les salons actifs |
| `users` | Liste tous les joueurs connectés |
| `kick <id>` | Expulse un joueur |
| `ban <id>` | Bannit un joueur |
| `stats` | Affiche les statistiques du serveur |
| `netstats` | Affiche les statistiques réseau (bande passante) |
| `connections` | Affiche l'état des connexions TCP/UDP |
| `quit` | Arrête le serveur |

---

## 2. Lancer le Client

Ouvrez un nouveau terminal et lancez le client graphique :

```bash
# Linux
./build/r-type_client

```

> **Note** : Le client doit être lancé depuis le dossier racine du projet pour trouver les fichiers `yaml/` et `textures/`.

---

## 3. Se Connecter au Serveur

1. Au lancement, le menu principal s'affiche
2. Cliquez sur **Play**
3. Entrez l'adresse IP du serveur :
   - **Local** (même machine) : `127.0.0.1` ou laisser vide
   - **Réseau local** : IP du PC serveur (ex: `192.168.1.15`)
   - **Internet** : IP publique du serveur
4. Cliquez sur **Connect**

---

## 4. Ports Réseau

| Port | Protocole | Utilisation |
|------|-----------|-------------|
| 4789 | TCP | Authentification, lobby, chat, création de salons |
| 4789 | UDP | Gameplay temps réel (mouvements, projectiles, sync) |

> **Firewall** : Si vous hébergez un serveur public, ouvrez le port 4789 en TCP et UDP.

---

## 5. Options de Lancement

### Serveur

| Option | Description |
|--------|-------------|
| `-a` | Mode admin avec console interactive |
| `-q` | Mode silencieux (logs minimaux) |

### Client

Le client se lance sans options. La configuration se fait via le menu in-game.

---

## 6. Dépannage

### Le client ne se connecte pas

1. Vérifiez que le serveur est lancé et affiche "listening on port 4789"
2. Vérifiez l'adresse IP entrée dans le client
3. Vérifiez votre firewall (port 4789 TCP/UDP)

### Le client crash au lancement

1. Assurez-vous que le dossier `yaml/` est présent
2. Assurez-vous que le dossier `textures/` est présent
3. Lancez le client depuis le dossier racine du projet

### Le serveur ne démarre pas

1. Vérifiez qu'aucun autre programme n'utilise le port 4789
2. Vérifiez que le fichier `yaml/main_loop.yaml` existe
