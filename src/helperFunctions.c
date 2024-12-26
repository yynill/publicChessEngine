#include "a_header.h"

int Bitboard_to_position(Bitboard bb)
{
    return __builtin_ctzll(bb);
}

Bitboard position_to_Bitboard(int position)
{
    return 1ULL << position;
}

// reminder: bitboards atr from top left to bottom right
void print_bitboard(Bitboard bb)
{
    for (int rank = 0; rank < 8; rank++)
    {
        printf("%d  ", 8 - rank); // Print rank numbers
        for (int file = 0; file < 8; file++)
        {
            int i = rank * 8 + file;
            printf("%d ", (bb & (1ULL << i)) ? 1 : 0);
        }
        printf("\n");
    }
    printf("\n   a b c d e f g h\n"); // Print file letters
}

void position_to_notation(int position, char *notation)
{
    int rank = 8 - (position / 8);
    char file = 'a' + (position % 8);
    sprintf(notation, "%c%d", file, rank);
}

int get_and_clear_LSB(Bitboard *bb)
{
    int pos = __builtin_ctzll(*bb); // Get position of least significant bit
    *bb &= (*bb - 1);               // Clear the least significant bit
    return pos;                     // Return the position of the LSB
}

MoveList *calculate_all_moves(Game *game, int color)
{
    MoveList *legal_moves = createMoveList();
    Bitboard piece_bitboard = color ? game->board.white_pieces : game->board.black_pieces;

    while (piece_bitboard)
    {
        int i = get_and_clear_LSB(&piece_bitboard);
        char piece = get_piece_at_position(&game->board, i);
        Bitboard temporary = 0;

        if (piece == 'P' || piece == 'p')
        {
            temporary = calculate_pawn_moves(game, i, 0);
        }
        else if (piece == 'r' || piece == 'R')
        {
            temporary = calculate_rook_moves(game, i, 0);
        }
        else if (piece == 'b' || piece == 'B')
        {
            temporary = calculate_bishop_moves(game, i, 0);
        }
        else if (piece == 'q' || piece == 'Q')
        {
            temporary = calculate_queen_moves(game, i, 0);
        }
        else if (piece == 'n' || piece == 'N')
        {
            temporary = calculate_knight_moves(game, i, 0);
        }
        else if (piece == 'k' || piece == 'K')
        {
            temporary = calculate_king_moves(game, i);
        }

        while (temporary)
        {
            int end_position = get_and_clear_LSB(&temporary);

            // Check for pawn promotion
            if ((piece == 'P' && end_position <= 7 && game->human_color == 1) ||
                (piece == 'p' && end_position >= 56 && game->human_color == 1) ||
                (piece == 'P' && end_position >= 56 && game->human_color == 0) ||
                (piece == 'p' && end_position <= 7 && game->human_color == 0))
            {
                // Add moves for each promotion piece
                char promotion_pieces[] = {'Q', 'R', 'B', 'N'};
                // Convert to lowercase for black pieces
                for (int j = 0; j < 4; j++)
                {
                    promotion_pieces[j] = piece == 'p' ? tolower(promotion_pieces[j]) : promotion_pieces[j];
                }
                for (int j = 0; j < 4; j++)
                {
                    if (is_move_legal(game, i, end_position))
                    {
                        addMove(legal_moves, i, end_position, get_piece_at_position(&game->board, end_position), promotion_pieces[j]);
                    }
                }
            }
            else
            {
                if (is_move_legal(game, i, end_position))
                {
                    addMove(legal_moves, i, end_position, get_piece_at_position(&game->board, end_position), '.');
                }
            }
        }
    }

    return legal_moves;
}

int is_move_legal(Game *game, int start_position, int end_position)
{
    Game temp_game;
    memcpy(&temp_game, game, sizeof(Game));

    move(&temp_game, start_position, end_position);
    update_threat_map(&temp_game);
    check_check(&temp_game);

    if (temp_game.isCheck == 10)
        return 0;

    if (temp_game.is_white_turn)
    {
        if (temp_game.isCheck == 1 || temp_game.isCheck == 3)
            return 0;
    }
    else
    {
        if (temp_game.isCheck == 2 || temp_game.isCheck == 4)
            return 0;
    }
    return 1;
}

int max(int a, int b)
{
    return (a > b) ? a : b;
}

int min(int a, int b)
{
    return (a < b) ? a : b;
}

void print_piece_values_board(Game *game)
{
    printf("\nPiece Values Board:\n");
    printf("    a    b    c    d    e    f    g    h\n");
    printf("  +----+----+----+----+----+----+----+----+\n");
    for (int rank = 0; rank < 8; rank++)
    {
        printf("%d |", rank + 1);
        for (int file = 0; file < 8; file++)
        {
            int position = rank * 8 + file;
            printf("%4d|", piece_value(get_piece_at_position(&game->board, position), position));
        }
        printf(" %d\n", rank + 1);
        printf("  +----+----+----+----+----+----+----+----+\n");
    }
    printf("    a    b    c    d    e    f    g    h\n");
}