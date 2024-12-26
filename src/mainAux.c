#include "a_header.h"
#include <pthread.h>
#include <string.h>

int mainAuxRunGameGUI(void)
{
    Game *game = NULL;
    game = initGame();

    // initialize SDL2 for video
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("ERROR: unable to init SDL: %s\n", SDL_GetError());
        return 0;
    }

    // Initialize TTF
    if (TTF_Init() < 0)
    {
        printf("ERROR: TTF_Init failed: %s\n", TTF_GetError());
        return 0;
    }

    // create an SDL window
    SDL_Window *window = SDL_CreateWindow(
        "Main menu",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN); // using SDL_WINDOW_SHOWN according to the forum

    if (window == NULL) // make sure window was created successfully
    {
        printf("ERROR: unable to create window: %s\n", SDL_GetError());
    }

    // GUI-based state machine
    int gameState = 1;

    do
    {
        switch (gameState)
        {
        case 1:
            gameState = runMainMenu(&window, &game);
            break;
        case 2:
            if (game == NULL)
            {
                printf("ERROR: unable to create game\n");
                SDL_DestroyWindow(window);
                SDL_Quit();
                return 0;
            }
            gameState = initialize_board(game); // if success, gameState = 3
            break;
        case 3:
            gameState = runGameWindow(&window, &game);
            break;
        case 4:
        {
            char promotion_piece = runPromotionWindow(&window, &game);
            if (promotion_piece == '.')
            {
                printf("ERROR: promotion_piece is NULL\n");
            }
            gameState = handle_promotion(&game->board, promotion_piece);
            toggle_turn(game);
            game->selected_position = -1;
            game->board.reachable_positions = 0;
            break;
        }
        }

    } while (gameState != 0);

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
}
