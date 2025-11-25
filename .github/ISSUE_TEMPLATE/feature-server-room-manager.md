---
name: 'Feature: Server - Room Manager'
about: Create a task for server room/lobby management
title: '[SERVER/ROOM] '
labels: ['enhancement', 'server', 'multiplayer', 'part-2']
assignees: ''
---

## Description
<!-- Describe the room/lobby management feature to implement -->

## Context
This feature is part of the Server subsystem, related to Track #2 - Advanced Networking (Part 2). The server must handle multiple game instances in parallel with lobby/room system for matchmaking and instance discoverability.

## Requirements
<!-- Check applicable requirements -->
- [ ] Server can run multiple game instances in parallel
- [ ] Multithreaded implementation
- [ ] Lobby/Room discovery system
- [ ] Players can join/leave rooms
- [ ] Robust error handling (no crashes)
- [ ] Notifications to clients on room events

## Technical Details
<!-- Describe technical implementation details -->

### Room Features
<!-- Check applicable features -->
- [ ] Room creation
- [ ] Room listing/discovery
- [ ] Room joining
- [ ] Room leaving
- [ ] Player limit per room
- [ ] Room state management
- [ ] Matchmaking logic
- [ ] Game rules per room
- [ ] Room admin/host controls

### Concurrency Design
<!-- Describe threading model -->
- Threading approach: 
- Synchronization mechanisms: 
- Resource sharing: 

### Data Structures
<!-- Key data structures for room management -->
```cpp
// Example structures
```

## Acceptance Criteria
<!-- Define what "done" means for this feature -->
- [ ] Code compiles without warnings
- [ ] Follows Epitech coding style
- [ ] Multiple rooms work simultaneously
- [ ] No race conditions or deadlocks
- [ ] Thread-safe implementation
- [ ] Handles client disconnections gracefully
- [ ] Notifies all clients correctly
- [ ] Unit tests pass
- [ ] Integration tests pass
- [ ] Documentation updated

## Scalability Requirements
<!-- If applicable -->
- Max concurrent rooms: 
- Max players per room: 4
- Expected load: 

## References
<!-- Links to design docs, RFCs, external resources -->
- Server Architecture: [Link]
- Project Subject: Track #2 - Multi-instance Server
- Related Protocol Messages: #[issue number]
