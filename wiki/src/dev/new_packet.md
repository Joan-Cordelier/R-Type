# Guide : Ajouter un Paquet Réseau

Ce guide détaille les étapes pour ajouter un nouveau type de message (OpCode) dans le protocole de communication Client-Serveur.

## 1. Définir l'OpCode

Ajoutez votre nouvel identifiant dans l'enum `OpCode` situé dans `src/common/Data/MessageFactory.hpp`.

```cpp
enum OpCode : uint8_t {
    // ... existants
    PLAYER = 0x09,
    MY_FEATURE = 0x0A  // <-- Nouvel OpCode (Hexadécimal)
};
```

## 2. Configurer le MessageFactory

Vous devez définir la taille et la priorité de votre message dans `src/common/Data/MessageFactory.cpp`.

Cherchez la fonction `initMessageTable()` :

```cpp
void MessageFactory::initMessageTable()
{
    // ...
    
    // Exemple 1 : Taille fixe (ex: 2 entiers = 8 octets)
    _messageTable[MY_FEATURE] = {8, Priority::MEDIUM};

    // Exemple 2 : Taille variable (ex: string ou liste)
    // _messageTable[MY_FEATURE] = {VARIABLE_LEN, Priority::LOW};
}
```

## 3. Gérer la Réception (Côté Serveur)

Si le message est envoyé par le client, vous devez le traiter dans le `MessageHandler`.

### 3.1 Déclaration
Dans `src/server/Handler/MessageHandler.hpp`, ajoutez la méthode de traitement :

```cpp
private:
    void handleMyFeature(const DecodedMessage& msg);
```

### 3.2 Dispatch
Dans `src/server/Handler/MessageHandler.cpp`, ajoutez le case dans `dispatchMessage` :

```cpp
switch (msg.opCode) {
    // ...
    case MY_FEATURE:
        handleMyFeature(msg);
        break;
}
```

### 3.3 Implémentation
Toujours dans `MessageHandler.cpp`, implémentez la logique :

```cpp
void MessageHandler::handleMyFeature(const DecodedMessage& msg)
{
    // 1. Lire les données (Deserialization)
    // Supposons que le payload contient 2 int (x, y)
    if (msg.data.size() < 8) return;
    
    // Rappel : Les données arrivent en octets bruts (uint8_t)
    // Vous devez peut-être reconstruire les variables
    
    // 2. Appliquer la logique
    LOG_INFO("Feature activée par le joueur " + std::to_string(msg.playerId));
    
    // 3. (Optionnel) Répondre ou Broadcaster
    // _session.broadcastTcp(...);
}
```

## 4. Envoyer le Message (Côté Client)

Pour envoyer ce message depuis le client (par exemple dans `InputSystem`), utilisez la `MessageFactory`.

```cpp
// Préparer les données
std::vector<uint8_t> payload;
// Remplir le payload...

// Créer le message
auto& factory = MessageFactory::getInstance();
auto msg = factory.createMessage(OpCode::MY_FEATURE, payload);

// Envoyer (TCP ou UDP selon le besoin)
networkManager.sendTcp(msg);
```