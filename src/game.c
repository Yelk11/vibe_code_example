#include "../include/game.h"
#include "../include/map.h"
#include "../include/player.h"
#include "../include/enemy.h"
#include "../include/ui.h"
#include <stdlib.h>
#include <ncurses.h>

// Initialize game state
void init_game(long seed)
{
    // Initialize random number generator
    srand(seed);

    // Initialize UI
    init_ui();

    // Initialize game state
    current_floor = 0;
    game_turn = 0;

    // Initialize message log
    message_log.num_messages = 0;

    // Initialize player
    init_player();

    // Initialize first floor
    init_floor(current_floor);

    // Add welcome message
    add_message("Welcome to the dungeon!");
}

// Clean up game resources
void cleanup_game()
{
    cleanup_ui();
}

// Main game loop
void game_loop()
{
    int input;

    while (1)
    {
        // Update field of view
        update_fov();

        // Render game state
        render_game();

        // Get player input
        input = getch();

        // Handle input
        handle_input(input);

        // Update game state
        update_game();

        // Check if player is dead
        if (player.health <= 0)
        {
            show_death_screen();
            break;
        }
    }
}

// Handle player input
void handle_input(int input)
{
    int dx = 0, dy = 0;

    switch (input)
    {
    case 'a':
        dx = -1;
        break; // Move left
    case 'd':
        dx = 1;
        break; // Move right
    case 'w':
        dy = -1;
        break; // Move up
    case 's':
        dy = 1;
        break; // Move down
    case 'q':
        dx = -1;
        dy = -1;
        break; // Move diagonally up-left
    case 'e':
        dx = 1;
        dy = -1;
        break; // Move diagonally up-right
    case 'z':
        dx = -1;
        dy = 1;
        break; // Move diagonally down-left
    case 'c':
        dx = 1;
        dy = 1;
        break; // Move diagonally down-right
    case 'i':
        view_inventory();
        break; // Open inventory
    case '.':
        break; // Wait one turn
    case 'Q':
        cleanup_game();
        exit(0);
        break; // Quit game
    }

    if (dx != 0 || dy != 0)
    {
        move_player(dx, dy);
    }
}

// Update game state
void update_game()
{
    // Update enemies
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        Enemy *enemy = &current_floor_ptr()->enemies[i];
        if (enemy->active)
        {
            update_enemy(enemy);
        }
    }

    // Update status effects
    for (int i = 0; i < MAX_STATUS_EFFECTS; i++)
    {
        if (player.status[i].type != STATUS_NONE)
        {
            if (--player.status[i].duration <= 0)
            {
                player.status[i].type = STATUS_NONE;
            }
        }
    }

    // Update ability cooldowns
    for (int i = 0; i < player.num_abilities; i++)
    {
        if (player.abilities[i].current_cooldown > 0)
        {
            player.abilities[i].current_cooldown--;
        }
    }

    // Regenerate mana
    if (player.mana < player.max_mana)
    {
        player.mana += player.mana_regen;
        if (player.mana > player.max_mana)
        {
            player.mana = player.max_mana;
        }
    }

    game_turn++;
}

// Render game state
void render_game()
{
    // First render the map
    render_map();
    // Render UI elements
    render_messages();
    render_status();
    refresh_screen();
}

// Show death screen and handle retry option
void show_death_screen()
{
    clear();
    int width;
    int height;
    get_terminal_size(&width, &height);
    mvprintw(height / 2 - 2, width / 2 - 5, "YOU DIED!");
    mvprintw(height / 2, width / 2 - 12, "Press any key to exit");
    refresh();
    getch();
}