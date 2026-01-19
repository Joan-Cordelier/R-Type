# Monitoring & Dashboard

Ce système permet de surveiller en temps réel la santé du serveur R-Type (performances, réseau, entités ECS) via une interface graphique.

##  Comment ça marche ? (Architecture)

Le système fonctionne en 3 étapes automatiques :

1.  **R-Type Server (Source)** : Le jeu calcule ses statistiques et les rend disponibles sur une page web cachée (`/metrics`).
2.  **Prometheus (Collecteur)** : Ce conteneur va lire la page du serveur toutes les 5 secondes et enregistre l'historique.
3.  **Grafana (Visuel)** : Ce conteneur interroge Prometheus pour dessiner les courbes et les jauges.

##  Lancement Rapide

Le monitoring utilise **Docker Compose**. Assurez-vous que Docker est lancé.

1.  Allez dans le dossier du serveur :
    ```bash
    cd src/server
    ```

2.  Démarrez l'infrastructure de monitoring :
    ```bash
    docker compose up -d
    ```
    *(L'option `-d` permet de lancer en arrière-plan sans bloquer le terminal)*.

3.  Lancez votre serveur de jeu normalement (dans un autre terminal) :
    ```bash
    ./r-type_server
    ```

##  Accéder au Dashboard

Une fois lancé, ouvrez votre navigateur :

* **Lien :** [http://localhost:3000](http://localhost:3000)
* **Utilisateur :** `admin`
* **Mot de passe :** `admin`

Une fois connecté, cliquez sur **Dashboards** > **R-Type Server Monitor**.

> **Note :** Pensez à régler la période de temps (en haut à droite) sur **"Last 5 minutes"** pour voir les données en temps réel.

## Dépannage

###  Je ne vois aucune donnée ("No Data")
1.  Vérifiez que `./r-type_server` tourne bien.
2.  Vérifiez que la période de temps dans Grafana n'est pas sur "Last 6 hours" (mettez "Last 5 minutes").

###  Erreur "Datasource not found" ou configuration cassée
Si Grafana semble perdu ou n'affiche pas les bonnes sources, c'est souvent un problème de cache. Il faut faire un **nettoyage complet** :

```bash
# 1. Arrêter et supprimer les volumes de données (IMPORTANT : le -v)
docker compose down -v

# 2. Relancer proprement
docker compose up -d