#include "a_header.h"

int runMainMenu(SDL_Window **window, Game **game)
{
    if (*game == NULL)
    {
        *game = (Game *)malloc(sizeof(Game));
        if (*game == NULL)
        {
            printf("ERROR: Failed to allocate memory for game\n");
            return 0;
        }
    }

    // check for correct input
    if (window == NULL)
    {
        printf("ERROR: runMainMenu - window is NULL\n");
        SDL_Quit();
        return 0;
    }

    // Load font
    TTF_Font *font = TTF_OpenFont("assets/a_font/Aceh-Medium.ttf", 24);
    if (!font)
    {
        printf("ERROR: Failed to load font: %s\n", TTF_GetError());
        return 0;
    }

    SDL_SetWindowTitle(*window, "Main menu");
    SDL_SetWindowSize(*window, WINDOW_WIDTH, WINDOW_HEIGHT);

    // Before creating new renderer, properly clean up the old one
    SDL_Renderer *oldRenderer = SDL_GetRenderer(*window);
    if (oldRenderer != NULL)
    {
        SDL_DestroyRenderer(oldRenderer);
    }

    // Clear any existing errors
    SDL_ClearError();

    // Create new renderer
    SDL_Renderer *rend = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
    if (rend == NULL)
    {
        printf("ERROR: unable to create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(*window);
        SDL_Quit();
        return 0;
    }

    // Set render color to white (R=255, G=255, B=255, A=255)
    SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);

    // Clear the renderer with the current draw color
    SDL_RenderClear(rend);

    // Present the renderer to display the white window
    SDL_RenderPresent(rend);

    // Create buttons
    Button *startButton = createButton(
        WINDOW_WIDTH / 2 - 100,
        WINDOW_HEIGHT / 2 - 150,
        "Start Game",
        NULL);

    Button *modeButton = createButton(
        WINDOW_WIDTH / 2 - 100,
        WINDOW_HEIGHT / 2 - 50,
        "MODE: 1 Player",
        NULL);

    Button *colorButton = createButton(
        WINDOW_WIDTH / 2 - 100,
        WINDOW_HEIGHT / 2 + 50,
        "Color: White",
        NULL);

    int numPlayers = 1;   // 1 for 1 player, 2 for 2 players
    int isWhiteColor = 1; // 1 for white, 0 for black

    // Event loop
    SDL_Event event;
    int running = 1;
    int mouseX, mouseY;

    while (running)
    {
        // Get mouse position
        SDL_GetMouseState(&mouseX, &mouseY);

        // Clear screen
        SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);
        SDL_RenderClear(rend);

        // Draw buttons
        drawButton(rend, startButton, font);
        drawButton(rend, modeButton, font);
        drawButton(rend, colorButton, font);

        // Present renderer
        SDL_RenderPresent(rend);

        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                running = 0;
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_CLOSE)
                {
                    running = 0;
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    if (isMouseOverButton(startButton, mouseX, mouseY))
                    {
                        (*game)->numPlayer = numPlayers;
                        (*game)->human_color = isWhiteColor;
                        return 2;
                    }
                    else if (isMouseOverButton(modeButton, mouseX, mouseY))
                    {
                        numPlayers = (numPlayers == 1) ? 2 : 1; // Toggle between 1 and 2 players
                        free(modeButton->text);
                        modeButton->text = strdup(numPlayers == 1 ? "MODE: 1 Player" : "MODE: 2 Players");
                    }
                    else if (isMouseOverButton(colorButton, mouseX, mouseY))
                    {
                        isWhiteColor = isWhiteColor == 1 ? 0 : 1;
                        free(colorButton->text);
                        colorButton->text = strdup(isWhiteColor ? "Color: White" : "Color: Black");
                    }
                    else
                    {
                    }
                }
                break;
            }
        }
    }

    // Cleanup
    destroyButton(startButton);
    destroyButton(modeButton);
    destroyButton(colorButton);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(rend);

    return 0;
}

int runGameWindow(SDL_Window **window, Game **game)
{
    // Check for correct input
    if (window == NULL || game == NULL)
    {
        printf("ERROR: runGameWindow - invalid parameters\n");
        SDL_Quit();
        return 0;
    }

    // Load font
    TTF_Font *font = TTF_OpenFont("assets/a_font/Aceh-Medium.ttf", 24);
    if (!font)
    {
        printf("ERROR: Failed to load font: %s\n", TTF_GetError());
        return 0;
    }

    SDL_SetWindowTitle(*window, "Chess Game");

    SDL_Renderer *old_rend = SDL_GetRenderer(*window);
    if (old_rend != NULL)
    {
        SDL_DestroyRenderer(old_rend);
    }

    // Create renderer
    SDL_Renderer *rend = SDL_CreateRenderer(*window, -1, SDL_RENDERER_SOFTWARE);
    if (rend == NULL)
    {
        printf("ERROR: unable to create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(*window);
        SDL_Quit();
        return 0;
    }

    load_piece_textures(rend);

    // Create buttons
    Button *menuButton = createButton(
        BOARD_WIDTH + ((WINDOW_WIDTH - BOARD_WIDTH) / 2) - 100, // Center in white strip
        25,
        "Main Menu",
        NULL);

    Button *resetButton = createButton(
        BOARD_WIDTH + ((WINDOW_WIDTH - BOARD_WIDTH) / 2) - 100, // Center in white strip
        100,
        "Reset",
        NULL);

    // Define colors
    SDL_Color light_square = {238, 216, 192, 255};
    SDL_Color dark_square = {165, 117, 80, 255};
    SDL_Color highlight = {255, 234, 0, 150};
    SDL_Color move_indicator = {0, 87, 183, 150};
    SDL_Color textColor = {0, 0, 0, 255};

    // event handling loop
    int running = -1;
    int clicked_first = -1; // the first left click
    int click_old = -1;     // remember the last get_moves position (so the program won't check the same move hundreds of times)
    int clicked_2nd = -1;   // the 2nd left click.
    int click_temp = -1;
    int click_right = -1;
    int PcMove[2] = {-1, -1}; // holds the suggested move for the computer (if there is one)
    int check = 1;            // as in check function result, not chess checkmate;
    int human_made_move = 0;  // indicates that the human made a move
    int moves_calulated = 0;  // indicates that the moves have been calculated

    SDL_Event e;

    SDL_Event event;

    while (running == -1)
    {
        if (moves_calulated == 0)
        {
            (*game)->board.reachable_positions = 0;
            clearMoveList((*game)->possible_moves);
            (*game)->possible_moves = calculate_all_moves(*game, (*game)->is_white_turn);
            check_check(*game); // check for checkmate, stalemate, etc.
            check_checkmate(*game);
            moves_calulated = 1;
        }

        // Clear screen with white background
        SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);
        SDL_RenderClear(rend);

        // First pass: Draw all squares
        for (int x = 0; x < BOARD_SIZE; x++)
        {
            for (int y = 0; y < BOARD_SIZE; y++)
            {
                SDL_Color *square_color;

                square_color = (x + y) % 2 == 0 ? &light_square : &dark_square;

                SDL_SetRenderDrawColor(rend, square_color->r, square_color->g, square_color->b, square_color->a);
                SDL_Rect square = {x * SQUARE_SIZE, y * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE};
                SDL_RenderFillRect(rend, &square);
            }
        }

        // Second pass: Draw highlight
        if ((*game)->selected_position != -1)
        {
            SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(rend, highlight.r, highlight.g, highlight.b, highlight.a);
            int row = (*game)->selected_position / 8;
            int col = (*game)->selected_position % 8;
            SDL_Rect highlight_square = {col * SQUARE_SIZE, row * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE};
            SDL_RenderFillRect(rend, &highlight_square);
        }

        // Third pass: Draw all pieces
        for (int y = 0; y < BOARD_SIZE; y++)
        {
            for (int x = 0; x < BOARD_SIZE; x++)
            {
                char piece = get_piece_at_position(&(*game)->board, y * 8 + x);
                if (piece != '.')
                {
                    int texture_index;
                    // Convert piece enum to texture index
                    switch (piece)
                    {
                    case 'P':
                        texture_index = 0;
                        break;
                    case 'N':
                        texture_index = 1;
                        break;
                    case 'B':
                        texture_index = 2;
                        break;
                    case 'R':
                        texture_index = 3;
                        break;
                    case 'Q':
                        texture_index = 4;
                        break;
                    case 'K':
                        texture_index = 5;
                        break;
                    case 'p':
                        texture_index = 6;
                        break;
                    case 'n':
                        texture_index = 7;
                        break;
                    case 'b':
                        texture_index = 8;
                        break;
                    case 'r':
                        texture_index = 9;
                        break;
                    case 'q':
                        texture_index = 10;
                        break;
                    case 'k':
                        texture_index = 11;
                        break;
                    default:
                        continue;
                    }

                    SDL_Rect pieceRect = {
                        x * SQUARE_SIZE,
                        y * SQUARE_SIZE,
                        SQUARE_SIZE,
                        SQUARE_SIZE};
                    SDL_RenderCopy(rend, piece_textures[texture_index], NULL, &pieceRect);
                }
            }
        }

        // Fourth pass: Highlight reachable positions
        Bitboard mask = (*game)->board.reachable_positions;
        SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(rend, move_indicator.r, move_indicator.g, move_indicator.b, move_indicator.a);
        for (int square = 0; square < 64; square++)
        {
            if (mask & position_to_Bitboard(square))
            {
                int centerX = (square % 8) * SQUARE_SIZE + SQUARE_SIZE / 2;
                int centerY = (square / 8) * SQUARE_SIZE + SQUARE_SIZE / 2;
                int radius = SQUARE_SIZE / 5;

                // Draw the dot (using a filled circle approximation)
                for (int dy = -radius; dy <= radius; dy++)
                {
                    for (int dx = -radius; dx <= radius; dx++)
                    {
                        if (dx * dx + dy * dy <= radius * radius)
                        {
                            SDL_RenderDrawPoint(rend, centerX + dx, centerY + dy);
                        }
                    }
                }
            }
        }

        // Fifth pass: Draw labels
        for (int x = 0; x < BOARD_SIZE; x++)
        {
            for (int y = 0; y < BOARD_SIZE; y++)
            {
                if (y == 7)
                {
                    char file_str[2] = {'a' + x, '\0'};
                    SDL_Color text_color = (x % 2 == 0) ? light_square : dark_square;
                    renderText(rend, font, file_str, (x + 1) * SQUARE_SIZE - 20,
                               (y + 1) * SQUARE_SIZE - 26, text_color);
                }
                if (x == 0)
                {
                    char rank_str[2] = {'8' - y, '\0'};
                    SDL_Color text_color = (y % 2 == 0) ? dark_square : light_square;
                    renderText(rend, font, rank_str, x * SQUARE_SIZE + 2,
                               y * SQUARE_SIZE + 2, text_color);
                }
            }
        }

        // Draw buttons
        drawButton(rend, resetButton, font);
        drawButton(rend, menuButton, font);

        // Draw score
        char score_text[50];
        int score = evaluate_board(*game);
        sprintf(score_text, "Evaluation: %d", score);
        renderText(rend, font, score_text,
                   BOARD_WIDTH + ((WINDOW_WIDTH - BOARD_WIDTH) / 2) - 100,
                   200,
                   textColor);

        // Draw check/checkmate status
        if ((*game)->isCheck == 0)
            renderText(rend, font, "Stalemate!",
                       BOARD_WIDTH + ((WINDOW_WIDTH - BOARD_WIDTH) / 2) - 100,
                       250,
                       textColor);
        else if ((*game)->isCheck == 1)
            renderText(rend, font, "White in Check!",
                       BOARD_WIDTH + ((WINDOW_WIDTH - BOARD_WIDTH) / 2) - 100,
                       250,
                       textColor);
        else if ((*game)->isCheck == 2)
            renderText(rend, font, "Black in Check!",
                       BOARD_WIDTH + ((WINDOW_WIDTH - BOARD_WIDTH) / 2) - 100,
                       250,
                       textColor);
        else if ((*game)->isCheck == 3)
            renderText(rend, font, "White in Checkmate!",
                       BOARD_WIDTH + ((WINDOW_WIDTH - BOARD_WIDTH) / 2) - 100,
                       250,
                       textColor);
        else if ((*game)->isCheck == 4)
            renderText(rend, font, "Black in Checkmate!",
                       BOARD_WIDTH + ((WINDOW_WIDTH - BOARD_WIDTH) / 2) - 100,
                       250,
                       textColor);
        else
            renderText(rend, font, "-",
                       BOARD_WIDTH + ((WINDOW_WIDTH - BOARD_WIDTH) / 2) - 100,
                       250,
                       textColor);

        // Draw move history
        int history_y = 300; // Start position for move history
        renderText(rend, font, "Move History:",
                   BOARD_WIDTH + ((WINDOW_WIDTH - BOARD_WIDTH) / 2) - 100,
                   history_y,
                   textColor);

        history_y += 30; // Space after title

        // Count total moves
        int total_moves = 0;
        Move *count_current = (*game)->move_history->head;
        while (count_current != NULL)
        {
            total_moves++;
            count_current = count_current->next;
        }

        // Skip to start showing only last 20 moves
        Move *current = (*game)->move_history->head;
        int moves_to_skip = total_moves > 20 ? total_moves - 20 : 0;
        for (int i = 0; i < moves_to_skip; i++)
        {
            current = current->next;
        }

        // Start move number from appropriate position
        int move_number = moves_to_skip + 1;

        char from_notation[3], to_notation[3];
        while (current != NULL && history_y < WINDOW_HEIGHT - 30)
        {
            position_to_notation(current->origin, from_notation);
            position_to_notation(current->target, to_notation);

            char move_text[20];
            if (move_number % 2 == 1)
            {
                // White's move
                if (current->captured != '.')
                    sprintf(move_text, "%d. %s-x%s (%c)", (move_number + 1) / 2, from_notation, to_notation, current->captured);
                else
                    sprintf(move_text, "%d. %s-%s", (move_number + 1) / 2, from_notation, to_notation);
            }
            else
            {
                // Black's move
                if (current->captured != '.')
                    sprintf(move_text, "   %s-x%s (%c)", from_notation, to_notation, current->captured);
                else
                    sprintf(move_text, "   %s-%s", from_notation, to_notation);
            }

            renderText(rend, font, move_text,
                       BOARD_WIDTH + ((WINDOW_WIDTH - BOARD_WIDTH) / 2) - 100,
                       history_y,
                       textColor);

            history_y += 20; // Space between moves
            current = current->next;
            move_number++;
        }

        // Present renderer
        SDL_RenderPresent(rend);

        // Engine move
        if ((*game)->numPlayer == 1) // If playing against computer
        {
            if (((*game)->human_color == 0 && (*game)->is_white_turn) ||
                ((*game)->human_color == 1 && !(*game)->is_white_turn))
            {
                int success = engine_move(*game);
                if (success)
                {
                    human_made_move = 1;                    // human made a move
                    moves_calulated = 0;                    // recalculate possible moves
                    toggle_turn(*game);                     // Toggle turn
                    (*game)->selected_position = -1;        // Clear selection
                    (*game)->board.reachable_positions = 0; // Clear reachable positions
                }
                check_check(*game);
                check_checkmate(*game);
            }
        }

        // Handle events
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                running = 0;
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_CLOSE)
                {
                    running = 0;
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.x < BOARD_WIDTH && event.button.y < BOARD_WIDTH) // if the mouse is in the board
                {
                    click_temp = (event.button.y / SQUARE_SIZE) * BOARD_SIZE + (event.button.x / SQUARE_SIZE); // 0 - 63 top left to bottom right

                    if (event.button.button == SDL_BUTTON_LEFT)
                    {
                        if ((*game)->selected_position != -1) // If we already have a piece selected
                        {
                            // Check if the new click position is in the reachable positions
                            if ((*game)->board.reachable_positions & position_to_Bitboard(click_temp))
                            {
                                move(*game, (*game)->selected_position, click_temp);                              // Make the move
                                addMove((*game)->move_history, (*game)->selected_position, click_temp, '.', '.'); // add move to move history
                                human_made_move = 1;                                                              // human made a move
                                moves_calulated = 0;                                                              // recalculate possible moves
                                if ((*game)->board.promotion_tile != -1)
                                {
                                    return 4; // promotion window
                                }
                                toggle_turn(*game);                     // Toggle turn
                                (*game)->selected_position = -1;        // Clear selection
                                (*game)->board.reachable_positions = 0; // Clear reachable positions
                                check_check(*game);
                                check_checkmate(*game);
                            }
                            else
                            {
                                // If clicking on a non-reachable position, treat as new selection
                                (*game)->selected_position = click_temp;
                                calcReachablePositions(*game);
                            }
                        }
                        else
                        {
                            // First click - select piece
                            (*game)->selected_position = click_temp;
                            calcReachablePositions(*game);
                        }
                    }
                }
                else // if click is outside the board
                {
                    if (event.button.button == SDL_BUTTON_LEFT)
                    {
                        (*game)->selected_position = -1; // Clear selection
                    }
                }
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    int mouseX, mouseY;
                    SDL_GetMouseState(&mouseX, &mouseY);

                    if (isMouseOverButton(menuButton, mouseX, mouseY))
                    {
                        running = 0;
                        return 1; // return to main menu
                    }
                    else if (isMouseOverButton(resetButton, mouseX, mouseY))
                    {
                        (*game)->move_history = createMoveList();
                        (*game)->is_white_turn = 1;
                        return 2;
                    }
                }
                break;
            }
        }
    }

    // Cleanup
    destroyButton(resetButton);
    destroyButton(menuButton);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(rend);

    // Add before return
    cleanup_piece_textures();

    return 0;
}

char runPromotionWindow(SDL_Window **window, Game **game)
{
    // Check inputs
    if (window == NULL || game == NULL)
    {
        printf("ERROR: runPromotionWindow - invalid parameters\n");
        return '.';
    }

    // Get current renderer
    SDL_Renderer *rend = SDL_GetRenderer(*window);
    if (rend == NULL)
    {
        printf("ERROR: unable to get renderer: %s\n", SDL_GetError());
        return '.';
    }

    // Calculate window dimensions and position
    int promo_size = SQUARE_SIZE * 2;
    SDL_Rect promo_rect = {
        (WINDOW_WIDTH - promo_size) / 2,
        (WINDOW_HEIGHT - promo_size) / 2,
        promo_size,
        promo_size};

    // Draw promotion window background
    SDL_SetRenderDrawColor(rend, 200, 200, 200, 255);
    SDL_RenderFillRect(rend, &promo_rect);

    // Draw border
    SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
    SDL_RenderDrawRect(rend, &promo_rect);

    // Setup piece positions
    SDL_Rect piece_rects[4];
    int texture_indices[4];

    // Determine if we're promoting white or black pieces
    int base_index = (*game)->is_white_turn ? 0 : 6;

    // Define the promotion pieces (Queen, Rook, Knight, Bishop)
    texture_indices[0] = base_index + 4; // Queen
    texture_indices[1] = base_index + 3; // Rook
    texture_indices[2] = base_index + 1; // Knight
    texture_indices[3] = base_index + 2; // Bishop

    // Calculate piece positions
    for (int i = 0; i < 4; i++)
    {
        piece_rects[i] = (SDL_Rect){
            promo_rect.x + (i % 2) * (promo_size / 2),
            promo_rect.y + (i / 2) * (promo_size / 2),
            promo_size / 2,
            promo_size / 2};
    }

    // Draw pieces
    for (int i = 0; i < 4; i++)
    {
        SDL_RenderCopy(rend, piece_textures[texture_indices[i]], NULL, &piece_rects[i]);
    }

    SDL_RenderPresent(rend);

    // Event loop
    SDL_Event event;
    int running = 1;
    char promotion_piece = '.';

    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                int mouseX = event.button.x;
                int mouseY = event.button.y;

                // Check which piece was clicked
                for (int i = 0; i < 4; i++)
                {
                    if (mouseX >= piece_rects[i].x && mouseX < piece_rects[i].x + piece_rects[i].w &&
                        mouseY >= piece_rects[i].y && mouseY < piece_rects[i].y + piece_rects[i].h)
                    {

                        // Set the promotion piece based on index
                        switch (i)
                        {
                        case 0:
                            promotion_piece = (*game)->is_white_turn ? 'Q' : 'q';
                            break; // Queen
                        case 1:
                            promotion_piece = (*game)->is_white_turn ? 'R' : 'r';
                            break; // Rook
                        case 2:
                            promotion_piece = (*game)->is_white_turn ? 'N' : 'n';
                            break; // Knight
                        case 3:
                            promotion_piece = (*game)->is_white_turn ? 'B' : 'b';
                            break; // Bishop
                        }
                        running = 0;
                        break;
                    }
                }
            }
            else if (event.type == SDL_QUIT)
            {
                return '.';
            }
        }
    }

    return promotion_piece;
}
