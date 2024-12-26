#include "a_header.h"

Game *initGame()
{
    Game *game = (Game *)malloc(sizeof(Game));
    if (game == NULL)
    {
        printf("ERROR: Failed to create game\n");
        return NULL;
    }
    game->move_history = createMoveList();
    game->possible_moves = createMoveList();

    game->isCheck = 0;
    game->white_king = -1;
    game->black_king = -1;
    game->currentPlayer = 1;
    game->human_color = -1;
    game->numPlayer = -1;
    game->selected_position = -1;
    game->is_white_turn = 1;

    return game;
}

void toggle_turn(Game *game)
{
    game->is_white_turn = game->is_white_turn == 1 ? 0 : 1;
}

void print_board(ChessBoard *board)
{
    char board_array[64] = {0};

    // Fill the board array
    for (int position = 0; position < 64; position++)
    {
        Bitboard mask = position_to_Bitboard(position);

        if (board->white_pawns & mask)
            board_array[position] = 'P';
        else if (board->white_knights & mask)
            board_array[position] = 'N';
        else if (board->white_bishops & mask)
            board_array[position] = 'B';
        else if (board->white_rooks & mask)
            board_array[position] = 'R';
        else if (board->white_queens & mask)
            board_array[position] = 'Q';
        else if (board->white_king & mask)
            board_array[position] = 'K';
        else if (board->black_pawns & mask)
            board_array[position] = 'p';
        else if (board->black_knights & mask)
            board_array[position] = 'n';
        else if (board->black_bishops & mask)
            board_array[position] = 'b';
        else if (board->black_rooks & mask)
            board_array[position] = 'r';
        else if (board->black_queens & mask)
            board_array[position] = 'q';
        else if (board->black_king & mask)
            board_array[position] = 'k';
        else
            board_array[position] = '.';
    }

    // Print the board
    printf("   a b c d e f g h\n");
    printf("\n");
    for (int rank = 0; rank < BOARD_SIZE; rank++)
    {
        printf("%d  ", 8 - rank);
        for (int file = 0; file < BOARD_SIZE; file++)
        {
            int position = rank * BOARD_SIZE + file;
            printf("%c ", board_array[position]);
        }
        printf("\n");
    }
    printf("\n");
}

void move(Game *game, int start_position, int end_position)
{
    if (game == NULL)
    {
        printf("ERROR: Game is NULL\n");
        return;
    }
    if (0 > start_position || start_position >= 64 || 0 > end_position || end_position >= 64)
    {
        printf("ERROR: Invalid move: %d -> %d\n", start_position, end_position);
        return;
    }

    // Pre-calculate the bitboards for positions
    Bitboard start_bb = position_to_Bitboard(start_position);
    Bitboard end_bb = position_to_Bitboard(end_position);

    char piece = get_piece_at_position(&game->board, start_position);
    char captured_piece = get_piece_at_position(&game->board, end_position);

    Bitboard *bitboards[] = {
        &game->board.white_pawns, &game->board.white_knights, &game->board.white_bishops,
        &game->board.white_rooks, &game->board.white_queens, &game->board.white_king,
        &game->board.black_pawns, &game->board.black_knights, &game->board.black_bishops,
        &game->board.black_rooks, &game->board.black_queens, &game->board.black_king};
    const char pieces[] = "PNBRQKpnbrqk";

    // // Remove captured piece if any
    if (captured_piece != '.')
    {
        for (int i = 0; i < 12; i++)
        {
            if (captured_piece == pieces[i])
            {
                *bitboards[i] &= ~end_bb;
                break;
            }
        }
    }

    // Remove piece from old position
    for (int i = 0; i < 12; i++)
    {
        if (piece == pieces[i])
        {
            *bitboards[i] &= ~start_bb;
            break;
        }
    }

    // Place the piece at its new position
    for (int i = 0; i < 12; i++)
    {
        if (piece == pieces[i])
        {
            *bitboards[i] |= end_bb;
            break;
        }
    }

    update_threat_map(game);

    // en passant capture handling
    int board_direction = (game->human_color == 1) ? -1 : 1;
    int piece_direction = (game->is_white_turn == 1) ? 1 : -1;

    int pawn_step = board_direction * piece_direction;
    // Add en passant capture handling
    if (game->board.last_move_double_pawn_push == 1 &&
        (piece == 'P' || piece == 'p') &&
        end_position == game->board.last_move_double_pawn_push_tile + (8 * pawn_step))
    {
        if (abs((end_position % 8) - (start_position % 8)) == 1)
        {
            // Remove the captured pawn
            if (piece == 'P')
            {
                game->board.black_pawns &= ~position_to_Bitboard(game->board.last_move_double_pawn_push_tile);
            }
            else
            {
                game->board.white_pawns &= ~position_to_Bitboard(game->board.last_move_double_pawn_push_tile);
            }
        }
    }

    // check for promotion / double pawn push
    if (piece == 'P' || piece == 'p')
    {
        // check for double pawn push
        if ((piece == 'P' && start_position >= 48 && start_position < 56 && end_position == start_position - 16) ||
            (piece == 'p' && start_position >= 8 && start_position < 16 && end_position == start_position + 16))
        {
            game->board.last_move_double_pawn_push = 1;
            game->board.last_move_double_pawn_push_tile = end_position;
        }
        else
        {
            game->board.last_move_double_pawn_push = 0;
            game->board.last_move_double_pawn_push_tile = -1;
        }

        // check for promotion
        if (end_position >= 56 || end_position <= 7)
        {
            game->board.promotion_tile = end_position;
        }
        else
        {
            game->board.promotion_tile = -1;
        }
    }
    else
    {
        game->board.promotion_tile = -1;
    }

    handle_castling(game, piece, start_position, end_position);

    // Track rook moves
    if (piece == 'R')
    {
        if (start_position == 56)
            game->board.rook_on56_moved = 1;
        else if (start_position == 63)
            game->board.rook_on63_moved = 1;
    }
    else if (piece == 'r')
    {
        if (start_position == 0)
            game->board.rook_on0_moved = 1;
        else if (start_position == 7)
            game->board.rook_on7_moved = 1;
    }
}

int handle_promotion(ChessBoard *board, char promotion_piece)
{
    int promotion_tile = board->promotion_tile;
    if (promotion_tile == -1)
    {
        printf("No promotion needed.\n");
        return 1;
    }

    Bitboard promotion_bb = position_to_Bitboard(promotion_tile);

    // Remove the piece at the promotion tile
    Bitboard *bitboards[] = {
        &board->white_pawns, &board->white_knights, &board->white_bishops,
        &board->white_rooks, &board->white_queens, &board->white_king,
        &board->black_pawns, &board->black_knights, &board->black_bishops,
        &board->black_rooks, &board->black_queens, &board->black_king};
    const char pieces[] = "PNBRQKpnbrqk";

    for (int i = 0; i < 12; i++)
    {
        *bitboards[i] &= ~promotion_bb;
    }

    // Set the new piece at the promotion tile
    for (int i = 0; i < 12; i++)
    {
        if (promotion_piece == pieces[i])
        {
            *bitboards[i] |= promotion_bb;
            break;
        }
    }

    return 3;
}

// 0 if no castling, 1 if castling is possible
int handle_castling(Game *game, char piece, int start_position, int end_position)
{
    // Track king moves
    if (game->human_color == 1)
    {
        if (piece == 'K')
        {
            game->board.white_king_moved = 1;

            // Handle castling moves
            if (start_position == 60) // e1
            {
                if (end_position == 62) // g1 - kingside
                {
                    // Move the rook from h1 to f1
                    game->board.white_rooks &= ~(position_to_Bitboard(63)); // Remove from h1
                    game->board.white_rooks |= position_to_Bitboard(61);    // Add to f1
                    return 1;
                }
                else if (end_position == 58) // c1 - queenside
                {
                    // Move the rook from a1 to d1
                    game->board.white_rooks &= ~(position_to_Bitboard(56)); // Remove from a1
                    game->board.white_rooks |= position_to_Bitboard(59);    // Add to d1
                    return 1;
                }
                game->white_king = end_position;
            }
        }
        else if (piece == 'k')
        {
            game->board.black_king_moved = 1;

            // Handle castling moves
            if (start_position == 4) // e8
            {
                if (end_position == 6) // g8 - kingside
                {
                    // Move the rook from h8 to f8
                    game->board.black_rooks &= ~(position_to_Bitboard(7)); // Remove from h8
                    game->board.black_rooks |= position_to_Bitboard(5);    // Add to f8
                    return 1;
                }
                else if (end_position == 2) // c8 - queenside
                {
                    // Move the rook from a8 to d8
                    game->board.black_rooks &= ~(position_to_Bitboard(0)); // Remove from a8
                    game->board.black_rooks |= position_to_Bitboard(3);    // Add to d8
                    return 1;
                }
                game->black_king = end_position;
            }
        }
    }
    else
    {
        if (piece == 'K')
        {
            game->board.white_king_moved = 1;

            // Handle castling moves
            if (start_position == 3)
            {
                if (end_position == 1) //  kingside
                {
                    // Move the rook from h1 to f1
                    game->board.white_rooks &= ~(position_to_Bitboard(0));
                    game->board.white_rooks |= position_to_Bitboard(2);
                    return 1;
                }
                else if (end_position == 5) // queenside
                {
                    // Move the rook from a1 to d1
                    game->board.white_rooks &= ~(position_to_Bitboard(7));
                    game->board.white_rooks |= position_to_Bitboard(4);
                    return 1;
                }
            }
            game->white_king = end_position;
        }
        else if (piece == 'k')
        {
            game->board.black_king_moved = 1;

            // Handle castling moves
            if (start_position == 59)
            {
                if (end_position == 57) // kingside
                {
                    // Move the rook from h8 to f8
                    game->board.black_rooks &= ~(position_to_Bitboard(56));
                    game->board.black_rooks |= position_to_Bitboard(58);
                    return 1;
                }
                else if (end_position == 61) // queenside
                {
                    // Move the rook from a8 to d8
                    game->board.black_rooks &= ~(position_to_Bitboard(63));
                    game->board.black_rooks |= position_to_Bitboard(60);
                    return 1;
                }
            }
            game->black_king = end_position;
        }
    }
    return 0;
}

void check_check(Game *game)
{
    update_threat_map(game);

    int white_in_check = (position_to_Bitboard(game->white_king) & game->board.black_threat_map) != 0;
    int black_in_check = (position_to_Bitboard(game->black_king) & game->board.white_threat_map) != 0;

    // Combine conditions into a single assignment
    game->isCheck = white_in_check && black_in_check ? 10 // Both in check
                    : white_in_check                 ? 1  // White in check
                    : black_in_check                 ? 2  // Black in check
                                                     : -1;                // No check
}

void check_checkmate(Game *game)
{
    // No need to update threat map again as check_check already did
    if (game->possible_moves->size > 0)
    {
        return; // Early return if moves exist
    }

    switch (game->isCheck)
    {
    case 1:
        game->isCheck = 3;
        break; // White checkmate
    case 2:
        game->isCheck = 4;
        break; // Black checkmate
    default:
        game->isCheck = 0;
        break; // Stalemate
    }
}