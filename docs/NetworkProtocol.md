# R-Type Network Protocol Documentation

## Table of Contents
1. [Message Structure](#message-structure)
2. [How to Add a New Message](#how-to-add-a-new-message)
3. [How to Connect to Server](#how-to-connect-to-server)
4. [Existing OpCodes](#existing-opcodes)

---

## Message Structure

All messages follow this binary format:

```
+----------+--------+-----------------+
| OpCode   | Length | Payload         |
| (1 byte) | (1 byte) | (N bytes)     |
+----------+--------+-----------------+
```

- **OpCode**: Identifies the message type (see `enum OpCode` in `MessageFactory.hpp`)
- **Length**: Size of the payload in bytes (0-255)
- **Payload**: Message-specific data

---

## How to Add a New Message

### Step 1: Add OpCode

In `src/common/Data/MessageFactory.hpp`, add your new OpCode to the enum:

```cpp
enum OpCode : uint8_t {
    INCOMPLETE = 0x00,
    PARSING_ERROR = 0x01,
    DEATH = 0x02,
    MOVE = 0x03,
    SHOOT = 0x04,
    CONNECT = 0x05,
    START = 0x06,
    JOIN = 0x07,
    CRASH = 0x08,
    PLAYER = 0x09,
    MY_NEW_MESSAGE = 0x0A  // <-- Add here with next available value
};
```

### Step 2: Register Message in Table

In `src/common/Data/MessageFactory.cpp`, add your message to `initMessageTable()`:

```cpp
void MessageFactory::initMessageTable()
{
    // ... existing messages ...
    
    // Add your new message
    // Parameters: OpCode, {length, priority}
    // Use VARIABLE_LEN (-1) for variable-length messages
    _messageTable[MY_NEW_MESSAGE] = {4, Priority::MEDIUM};  // Fixed 4-byte payload
    // OR
    _messageTable[MY_NEW_MESSAGE] = {VARIABLE_LEN, Priority::HIGH};  // Variable length
}
```

### Step 3: Add Handler (Server-side)

In `src/server/Handler/MessageHandler.hpp`, declare the handler:

```cpp
private:
    void handleMyNewMessage(const DecodedMessage& msg);
```

In `src/server/Handler/MessageHandler.cpp`:

1. Add case to `dispatchMessage()`:
```cpp
switch (msg.opCode) {
    // ... existing cases ...
    case MY_NEW_MESSAGE:
        handleMyNewMessage(msg);
        break;
}
```

2. Implement the handler:
```cpp
void MessageHandler::handleMyNewMessage(const DecodedMessage& msg)
{
    LOG_INFO("Handling MY_NEW_MESSAGE from player " + std::to_string(msg.playerId));
    
    // Parse payload from msg.data
    // msg.data is a std::vector<uint8_t>
    
    // Process the message...
    
    // Send response if needed
    auto& factory = MessageFactory::getInstance();
    auto response = factory.createMessage(MY_NEW_MESSAGE, {/* payload bytes */});
    _session.sendTcp(msg.playerId, response);
}
```

### Step 4: Send Message (Client-side)

```cpp
auto& factory = MessageFactory::getInstance();

// Create message with payload
std::vector<uint8_t> payload = {0x01, 0x02, 0x03, 0x04};
auto msg = factory.createMessage(MY_NEW_MESSAGE, payload);

// Send via NetworkManager
networkManager.sendTcp(msg);
// OR
networkManager.sendUdp(msg);
```

---

## How to Connect to Server

### Server-Side Flow

The server automatically handles connections:

1. **TCP Accept** → `SessionManager::addPlayer(fd)` is called automatically
2. Player is assigned a unique `playerId`
3. When client sends `CONNECT` message → `handleConnect()` marks player as ready
4. Server responds with `CONNECT` (containing `playerId`) + `JOIN` (containing `roomId`)

### Client-Side Connection

```cpp
#include "Network/NetworkManager.hpp"
#include "../common/Data/MessageFactory.hpp"

int main()
{
    // 1. Create NetworkManager
    NetworkManager network;
    
    // 2. Connect to server (IP address, TCP port, UDP port)
    if (!network.connect("127.0.0.1", 4789, 4790)) {
        std::cerr << "Failed to connect to server" << std::endl;
        return 1;
    }
    
    // 3. Start network threads
    network.start();
    
    // 4. Send CONNECT message to register with server
    auto& factory = MessageFactory::getInstance();
    auto connectMsg = factory.createMessage(CONNECT, {});
    network.sendTcp(connectMsg);
    
    // 5. Wait for server response
    // The server will send back:
    //   - CONNECT message with your playerId (4 bytes)
    //   - JOIN message with your roomId (1 byte)
    
    // 6. Process incoming messages
    while (running) {
        // Check for messages by priority
        auto msg = network.popMessage(Priority::CRITICAL);
        if (!msg.has_value())
            msg = network.popMessage(Priority::HIGH);
        if (!msg.has_value())
            msg = network.popMessage(Priority::MEDIUM);
        if (!msg.has_value())
            msg = network.popMessage(Priority::LOW);
            
        if (msg.has_value()) {
            switch (msg->opCode) {
                case CONNECT:
                    // Extract playerId from payload (4 bytes, big-endian)
                    if (msg->data.size() >= 4) {
                        uint32_t playerId = 
                            (msg->data[0] << 24) |
                            (msg->data[1] << 16) |
                            (msg->data[2] << 8) |
                            msg->data[3];
                        std::cout << "Connected! Player ID: " << playerId << std::endl;
                    }
                    break;
                case JOIN:
                    // Extract roomId from payload (1 byte)
                    if (msg->data.size() >= 1) {
                        uint8_t roomId = msg->data[0];
                        std::cout << "Joined room: " << (int)roomId << std::endl;
                    }
                    break;
                // Handle other messages...
            }
        }
    }
    
    // 7. Cleanup
    network.stop();
    return 0;
}
```

### Connection Sequence Diagram

```
Client                              Server
  |                                    |
  |-------- TCP Connect -------------->|
  |                                    | --> addPlayer(fd) assigns playerId
  |                                    |
  |-------- CONNECT (empty) --------->|  (TCP)
  |                                    | --> handleConnect() marks connected=true
  |                                    |
  |<------- CONNECT (playerId) -------|  (TCP)
  |<------- JOIN (roomId) ------------|  (TCP)
  |                                    |
  |-------- LINK (playerId) --------->|  (UDP)
  |                                    | --> linkPlayerUdp() associates UDP address
  |                                    |
  |  [Player is now fully connected]   |
  |  [Can now send/receive UDP msgs]   |
  |                                    |
```

---

## Existing OpCodes

| OpCode | Value | Description | Payload | Priority |
|--------|-------|-------------|---------|----------|
| INCOMPLETE | 0x00 | Internal: incomplete message | - | - |
| PARSING_ERROR | 0x01 | Internal: parse error | - | ERROR |
| DEATH | 0x02 | Entity death notification | TBD | HIGH |
| MOVE | 0x03 | Movement update | x, y floats (8 bytes) | LOW |
| SHOOT | 0x04 | Shoot action | empty | HIGH |
| CONNECT | 0x05 | Connection handshake (TCP) | Request: empty, Response: playerId (4 bytes) | CRITICAL |
| START | 0x06 | Game start | TBD | CRITICAL |
| JOIN | 0x07 | Room join notification | roomId (1 byte) | CRITICAL |
| CRASH | 0x08 | Crash/error notification | TBD | CRITICAL |
| PLAYER | 0x09 | Player info | TBD | CRITICAL |
| LINK | 0x0A | UDP address linking (UDP) | playerId (4 bytes, big-endian) | CRITICAL |

---

## Priority Levels

Messages are processed in priority order:

1. **CRITICAL** - Connection, start, crash (processed first)
2. **HIGH** - Death, shoot, join
3. **MEDIUM** - Movement, player info
4. **LOW** - Non-essential updates
5. **ERROR** - Error conditions
