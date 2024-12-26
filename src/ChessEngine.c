#include "a_header.h"
#include "chessEngineBrain.h"

int engine_move(Game *game)
{
    // Start timing
    clock_t start = clock();
    long long positions_counted = 0;
    int engine_color = game->human_color == 1 ? 0 : 1;
    const int SEARCH_DEPTH = 3; // 0 is min depth (only root node)

    SearchResult best_result;

    if (game->possible_moves->size > 0)
    {
        Move *best_move = NULL;
        int best_score = engine_color == 1 ? -1000000000 : 1000000000;
        Move *current_move = game->possible_moves->head;

        // Try each possible move and use minimax to evaluate
        while (current_move != NULL)
        {
            Game temp_game;
            memcpy(&temp_game, game, sizeof(Game));

            // Handle different move types (normal, promotion, castling)
            if (current_move->promotion_piece != '.')
            {
                move(&temp_game, current_move->origin, current_move->target);
                temp_game.board.promotion_tile = current_move->target;
                handle_promotion(&temp_game.board, current_move->promotion_piece);
            }
            else if (handle_castling(&temp_game, get_piece_at_position(&game->board, current_move->origin), current_move->origin, current_move->target))
            {
                move(&temp_game, current_move->origin, current_move->target);
            }
            else
            {
                move(&temp_game, current_move->origin, current_move->target);
            }

            check_check(&temp_game);
            check_checkmate(&temp_game);
            toggle_turn(&temp_game);

            SearchResult result = minimax(&temp_game, SEARCH_DEPTH, -1000000000, 1000000000, &positions_counted);
            int score = result.score;

            // Update best move based on engine color
            if ((engine_color == 1 && score > best_score) ||
                (engine_color == 0 && score < best_score))
            {
                best_score = score;
                best_move = current_move;
                best_result = result;
            }

            current_move = current_move->next;
        }

        // Execute the best move found
        move(game, best_move->origin, best_move->target);
        addMove(game->move_history, best_move->origin, best_move->target, '.', '.'); // add move to move history

        if (best_move->promotion_piece != '.')
        {
            game->board.promotion_tile = best_move->target;
            handle_promotion(&game->board, best_move->promotion_piece);
        }

        // Print the best move and statistics
        char notation1[3], notation2[3];
        double time_spent = (double)(clock() - start) / CLOCKS_PER_SEC;
        position_to_notation(best_move->origin, notation1);
        position_to_notation(best_move->target, notation2);

        // Calculate additional statistics
        double positions_per_second = positions_counted / time_spent;
        int depth_reached = SEARCH_DEPTH;
        double branching_factor = pow(positions_counted, 1.0 / depth_reached);

        printf("\nEngine Move Analysis:\n");
        printf("├─ Search depth:            %d\n", depth_reached + 1);
        printf("├─ Best Move:               %s → %s %c\n", notation1, notation2, best_move->promotion_piece);

        printf("├─ Positions analyzed:      %'lld\n", positions_counted);
        printf("├─ Time spent:              %.3f seconds\n", time_spent);
        printf("├─ Speed:                   %.0f positions/second\n", positions_per_second);
        printf("└─ Avg. branching factor:   %.1f\n", branching_factor);

        printf("\nBest line:\n");
        for (int i = 0; i < best_result.length; i++)
        {
            char from[3], to[3];
            position_to_notation(best_result.moves[i].origin, from);
            position_to_notation(best_result.moves[i].target, to);
            printf("%d: %s → %s\n", i + 1, from, to);
        }
    }
    else
    {
        return 0;
    }
    return 1;
}

SearchResult minimax(Game *game, int depth, int alpha, int beta, long long *positions_counted)
{
    SearchResult result = {0};
    result.length = 0;

    // Base cases
    if (depth == 0 || game->possible_moves->size == 0 || game->isCheck == 3 || game->isCheck == 4 || game->isCheck == 0)
    {
        (*positions_counted)++;
        if (game->isCheck == 3)
            result.score = -999999;
        else if (game->isCheck == 4)
            result.score = 999999;
        else if (game->isCheck == 0)
            result.score = 0;
        else
            result.score = evaluate_board(game);
        return result;
    }

    if (game->is_white_turn)
    {
        result.score = -1000000000;
        game->possible_moves = calculate_all_moves(game, 1);
        Move *current_move = game->possible_moves->head;

        while (current_move != NULL)
        {
            Game temp_game;
            memcpy(&temp_game, game, sizeof(Game));

            // Handle different move types (normal, promotion, castling)
            if (current_move->promotion_piece != '.')
            {
                move(&temp_game, current_move->origin, current_move->target);
                temp_game.board.promotion_tile = current_move->target;
                handle_promotion(&temp_game.board, current_move->promotion_piece);
            }
            else if (handle_castling(&temp_game, get_piece_at_position(&game->board, current_move->origin), current_move->origin, current_move->target))
            {
                move(&temp_game, current_move->origin, current_move->target);
            }
            else
            {
                move(&temp_game, current_move->origin, current_move->target);
            }

            toggle_turn(&temp_game);

            // Update game state and get evaluation
            check_check(&temp_game);
            check_checkmate(&temp_game);

            SearchResult child_result = minimax(&temp_game, depth - 1, alpha, beta, positions_counted);

            if (child_result.score > result.score)
            {
                result.score = child_result.score;

                // Ensure we don't exceed array bounds
                if (child_result.length + 1 >= 4096)
                {
                    result.length = 4095;
                    printf("Error: Move array overflow. Truncating to 4095 moves.\n");
                }
                else
                {
                    // Copy current move but clear the pointers
                    result.moves[0] = *current_move;
                    result.moves[0].next = NULL;
                    result.moves[0].prev = NULL;

                    // Copy the child's moves
                    memcpy(&result.moves[1], child_result.moves, child_result.length * sizeof(Move));
                    result.length = child_result.length + 1;
                }
            }

            alpha = alpha > result.score ? alpha : result.score;
            if (beta <= alpha)
                break;

            current_move = current_move->next;
        }
        return result;
    }
    else
    {
        result.score = 1000000000;
        game->possible_moves = calculate_all_moves(game, 0);
        Move *current_move = game->possible_moves->head;

        while (current_move != NULL)
        {
            Game temp_game;
            memcpy(&temp_game, game, sizeof(Game));

            // Handle different move types (normal, promotion, castling)
            if (current_move->promotion_piece != '.')
            {
                move(&temp_game, current_move->origin, current_move->target);
                temp_game.board.promotion_tile = current_move->target;
                handle_promotion(&temp_game.board, current_move->promotion_piece);
            }
            else if (handle_castling(&temp_game, get_piece_at_position(&game->board, current_move->origin), current_move->origin, current_move->target))
            {
                move(&temp_game, current_move->origin, current_move->target);
            }
            else
            {
                move(&temp_game, current_move->origin, current_move->target);
            }

            toggle_turn(&temp_game);

            // Update game state and get evaluation
            check_check(&temp_game);
            check_checkmate(&temp_game);

            SearchResult child_result = minimax(&temp_game, depth - 1, alpha, beta, positions_counted);

            if (child_result.score < result.score)
            {
                result.score = child_result.score;

                // Ensure we don't exceed array bounds
                if (child_result.length + 1 >= 4096)
                {
                    result.length = 4095;
                    printf("Error: Move array overflow. Truncating to 4095 moves.\n");
                }
                else
                {
                    // Copy current move but clear the pointers
                    result.moves[0] = *current_move;
                    result.moves[0].next = NULL;
                    result.moves[0].prev = NULL;

                    // Copy the child's moves
                    memcpy(&result.moves[1], child_result.moves, child_result.length * sizeof(Move));
                    result.length = child_result.length + 1;
                }
            }

            beta = beta < result.score ? beta : result.score;
            if (beta <= alpha)
                break;

            current_move = current_move->next;
        }
        return result;
    }
}

int evaluate_board(Game *game)
{
    int score = 0;

    // Handle terminal positions first
    if (game->isCheck == 4)
        return 1000000000; // black in checkmate
    if (game->isCheck == 3)
        return -1000000000; // white in checkmate
    if (game->isCheck == 0)
        return 0; // stalemate

    int white_score = 0;
    int black_score = 0;

    // Material and position evaluation (existing code)
    Bitboard black_pieces = game->board.black_pieces;
    Bitboard white_pieces = game->board.white_pieces;

    while (black_pieces)
    {
        int pos = get_and_clear_LSB(&black_pieces);
        black_score += piece_value(get_piece_at_position(&game->board, pos), pos);
    }

    while (white_pieces)
    {
        int pos = get_and_clear_LSB(&white_pieces);
        white_score += piece_value(get_piece_at_position(&game->board, pos), pos);
    }

    score = white_score - black_score;
    return score;
}

int piece_value(char piece, int position)
{
    int base_value;
    int position_value;
    int mirror_position = 63 - position;

    // Get base piece value
    switch (piece)
    {
    case 'P':
        base_value = PAWN_VALUE;
        position_value = PAWN_SQUARE_TABLE[position];
        break;
    case 'N':
        base_value = KNIGHT_VALUE;
        position_value = KNIGHT_SQUARE_TABLE[position];
        break;
    case 'B':
        base_value = BISHOP_VALUE;
        position_value = BISHOP_SQUARE_TABLE[position];
        break;
    case 'R':
        base_value = ROOK_VALUE;
        position_value = ROOK_SQUARE_TABLE[position];
        break;
    case 'Q':
        base_value = QUEEN_VALUE;
        position_value = QUEEN_SQUARE_TABLE[position];
        break;
    case 'K':
        base_value = KING_VALUE;
        position_value = KING_SQUARE_TABLE[position];
        break;
    case 'p':
        base_value = PAWN_VALUE;
        position_value = PAWN_SQUARE_TABLE[mirror_position];
        break;
    case 'n':
        base_value = KNIGHT_VALUE;
        position_value = KNIGHT_SQUARE_TABLE[mirror_position];
        break;
    case 'b':
        base_value = BISHOP_VALUE;
        position_value = BISHOP_SQUARE_TABLE[mirror_position];
        break;
    case 'r':
        base_value = ROOK_VALUE;
        position_value = ROOK_SQUARE_TABLE[mirror_position];
        break;
    case 'q':
        base_value = QUEEN_VALUE;
        position_value = QUEEN_SQUARE_TABLE[mirror_position];
        break;
    case 'k':
        base_value = KING_VALUE;
        position_value = KING_SQUARE_TABLE[mirror_position];
        break;
    default:
        return 0;
    }
    return base_value + position_value;
}