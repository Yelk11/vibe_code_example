# Enhanced Rogue-like Game

A feature-rich ASCII-based rogue-like game where you explore dungeons, fight enemies, and collect items!

## Features

- Player character (`@`) with health system
- Multiple enemies (`E`) that move around and engage in combat
- Various items to collect:
  - Coins (`$`) for scoring
  - Health potions (`*`) to restore health
  - Power-ups (`+`) for bonus points
- Complex dungeon layout with multiple rooms and corridors
- Combat system with health tracking
- Scoring system

## Building the Game

To compile the game, simply run:

```bash
make
```

## Running the Game

After compiling, run the game with:

```bash
./game
```

## How to Play

### Movement
- Use `W` to move up
- Use `S` to move down
- Use `A` to move left
- Use `D` to move right
- Press `Q` to quit the game

### Combat
- Move adjacent to enemies to engage in combat
- Each combat interaction:
  - Deals 10 damage to you
  - Deals 25 damage to the enemy
- Defeating an enemy awards 100 points

### Items
- `$` Coins: Collect for points
- `*` Health Potions: Restore health
- `+` Power-ups: Award bonus points

### Terrain
- `.` represents floor tiles
- `#` represents walls
- The dungeon consists of multiple connected rooms

## Game Over
The game ends when either:
- Your health reaches 0
- You press 'Q' to quit

Your final score will be displayed when the game ends.

## Requirements

- GCC compiler
- Unix-like environment (Linux, macOS) 