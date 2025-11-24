---
name: 'Feature: Gameplay - Enemy AI'
about: Create a task for enemy behavior and AI implementation
title: '[GAMEPLAY/AI] '
labels: ['enhancement', 'gameplay', 'ai', 'content']
assignees: ''
---

## Description
<!-- Describe the enemy type or AI behavior to implement -->

## Context
This feature is part of the Gameplay subsystem, related to Track #3 - Advanced Gameplay (Part 2). R-Type features many monsters with varying movement patterns and attacks. Enemy behavior should be implemented in a reusable way.

## Requirements
<!-- Check applicable requirements -->
- [ ] Enemy behavior is server-authoritative
- [ ] Movement patterns are smooth and predictable
- [ ] Frame-rate independent (uses timers)
- [ ] Collision detection works correctly
- [ ] Spawning mechanism implemented
- [ ] Death/destruction handled properly
- [ ] Reusable behavior system (scripting/components)

## Technical Details
<!-- Describe technical implementation details -->

### Enemy Type
<!-- What enemy is this? -->
- Name: 
- Reference: (e.g., Bydos slave, Snake-style monster, Boss)
- Sprite/Asset: 

### Behavior Specification
<!-- Describe AI behavior -->
- Movement pattern: 
- Attack pattern: 
- Health/Damage: 
- Spawn conditions: 
- Special abilities: 

### Implementation Approach
<!-- How will this be implemented? -->
- [ ] Hardcoded C++ behavior
- [ ] Component-based behavior
- [ ] Script-based behavior (Lua/Python)
- [ ] Data-driven configuration
- [ ] Behavior tree
- [ ] State machine

### Code Structure
```cpp
// Example pseudocode or class structure
```

## Acceptance Criteria
<!-- Define what "done" means for this feature -->
- [ ] Code compiles without warnings
- [ ] Follows Epitech coding style
- [ ] Enemy spawns correctly
- [ ] Movement pattern works as specified
- [ ] Attack pattern works as specified
- [ ] Collision detection accurate
- [ ] Death animation/effect plays
- [ ] Server synchronizes to all clients
- [ ] No performance issues (maintains 60 FPS)
- [ ] Gameplay tests pass
- [ ] Behavior is reusable for similar enemies

## Gameplay Balance
<!-- If applicable -->
- Difficulty level: 
- Spawn frequency: 
- Health points: 
- Damage dealt: 

## Visual Reference
<!-- Add screenshots, videos, or references from original R-Type -->
- Original R-Type reference: [link/screenshot]

## References
<!-- Links to design docs, gameplay specs, external resources -->
- Gameplay Specification: [Link]
- Project Subject: Track #3 - Advanced Gameplay
- R-Type Enemy Reference: [Stage breakdown](https://www.vgmuseum.com/mrp/2/rtype.htm)
