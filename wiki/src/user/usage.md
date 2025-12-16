# Lancer le Jeu

Le R-Type est un jeu client-serveur. Vous devez lancer un serveur central avant de pouvoir connecter des clients.

## 1. Lancer le Serveur

Le serveur gère la logique du jeu et synchronise les joueurs.

Ouvrez un terminal et exécutez :

```bash
# Linux
./build/r-type_server

**Note :** Par défaut, le serveur écoute sur le port **4789** (TCP et UDP). Assurez-vous que ce port est ouvert si vous jouez via internet.
Vous devriez voir le message : `TCP server listening on port 4789`.

## 2. Lancer le Client

Ouvrez un nouveau terminal (ou plusieurs pour simuler du multijoueur) et lancez le client graphique :

```bash
# Linux
./build/r-type_client

# Windows
.\build\Release\r-type_client.exe
```

## 3. Se Connecter

1.  Au lancement, le menu principal s'affiche.
2.  Un champ de texte vous permet de saisir l'adresse IP du serveur.
    * Si vous jouez en local (même machine), laissez vide ou mettez `127.0.0.1`.
    * Si le serveur est distant, entrez son IP (ex: `192.168.1.15`).
3.  Cliquez sur le bouton **PLAY** (ou START).
4.  Le jeu tentera de se connecter au port 4789. Si la connexion réussit, le vaisseau apparaîtra et le jeu commencera.
