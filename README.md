# Roguelike Dungeon Game

A classic roguelike dungeon crawler written in C using ncurses. Explore procedurally generated dungeons, fight enemies, collect items, and try to survive!

## Features

- **Procedurally Generated Dungeons**: Each floor features unique square rooms connected by hallways
- **Item System**:
  - Weapons (Iron Swords) for combat
  - Armor (Leather Armor) for protection
  - Health Potions for healing
  - Gold for collecting treasure
  - Floor Keys to unlock stairs to the next level
- **Combat System**: Fight enemies using equipped weapons and armor
- **Field of View**: Dynamic visibility system that reveals areas as you explore
- **Inventory Management**: Collect, equip, use, and drop items
- **Status Effects**: Various effects that can help or hinder your progress

## Controls

- Movement:
  - `w` - Move up
  - `s` - Move down
  - `a` - Move left
  - `d` - Move right
  - `q` - Move diagonally up-left
  - `e` - Move diagonally up-right
  - `z` - Move diagonally down-left
  - `c` - Move diagonally down-right
  - `.` - Wait one turn

- Inventory:
  - `i` - Open inventory
  - In inventory:
    - `u` - Use selected item
    - `d` - Drop selected item
    - `e` - Equip selected item
    - `q` - Close inventory

- Other:
  - `Q` - Quit game

## Game Elements

- `@` - Player
- `.` - Floor
- `#` - Wall
- `<` - Stairs up
- `%` - Locked stairs down
- `/` - Weapon
- `[` - Armor
- `!` - Potion
- `$` - Gold
- `K` - Floor Key

## Building and Running

### Prerequisites

- GCC compiler
- ncurses library
- make

### Building

```bash
make clean  # Clean previous build
make        # Compile the game
```

### Running

```bash
./game      # Start the game
```

## Game Mechanics

### Rooms and Navigation
- Each floor contains square rooms connected by hallways
- Rooms vary in size but maintain square proportions
- Hallways are single-tile wide for consistent navigation

### Items and Equipment
- Items spawn randomly in rooms (70% chance per room)
- Equipment affects your combat capabilities:
  - Weapons increase attack power
  - Armor provides defense
- Potions can be used to restore health
- Gold can be collected for score/future features

### Floor Progression
1. Find the Floor Key (marked as 'K')
2. Locate the locked stairs (marked as '%')
3. Use the key to unlock the stairs
4. Proceed to the next floor

### Combat
- Move into enemies to attack them
- Damage is calculated based on:
  - Your weapon's power
  - Enemy's defense
  - Random factors

## Development

The game is built with a modular architecture:
- `map.c` - Dungeon generation and floor management
- `player.c` - Player stats and inventory management
- `enemy.c` - Enemy behavior and combat
- `item.c` - Item definitions and interactions
- `ui.c` - Display and user interface
- `game.c` - Main game loop and input handling 