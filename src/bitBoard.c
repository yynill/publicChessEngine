#include "a_header.h"

// Define the global variables
ChessBoard global_board;

// Example usage
int initialize_board(Game *game)
{
    if (game == NULL)
    {
        printf("ERROR: Game is NULL\n");
        return 0;
    }

    if (game->human_color == 1) // 1 for white, 0 for black
    {
        // Set starting bits for white pieces
        game->board.white_pawns = 0x00FF000000000000;
        game->board.white_knights = 0x4200000000000000;
        game->board.white_bishops = 0x2400000000000000;
        game->board.white_rooks = 0x8100000000000000;
        game->board.white_queens = 0x0800000000000000;
        game->board.white_king = 0x1000000000000000;

        // Set starting bits for black pieces
        game->board.black_pawns = 0xff00;
        game->board.black_bishops = 0x0024;
        game->board.black_knights = 0x0042;
        game->board.black_rooks = 0x0081;
        game->board.black_queens = 0x0008;
        game->board.black_king = 0x0010;
    }
    else if (game->human_color == 0)
    {
        // Set starting bits for black pieces (mirrored)
        game->board.white_pawns = 0xff00;
        game->board.white_bishops = 0x0024;
        game->board.white_knights = 0x0042;
        game->board.white_rooks = 0x0081;
        game->board.white_queens = 0x0010;
        game->board.white_king = 0x0008;

        // Set starting bits for white pieces
        game->board.black_pawns = 0x00FF000000000000;
        game->board.black_knights = 0x4200000000000000;
        game->board.black_bishops = 0x2400000000000000;
        game->board.black_rooks = 0x8100000000000000;
        game->board.black_queens = 0x1000000000000000;
        game->board.black_king = 0x0800000000000000;
    }

    // Initialize castling flags
    game->board.white_king_moved = 0;
    game->board.black_king_moved = 0;
    game->board.rook_on0_moved = 0;
    game->board.rook_on7_moved = 0;
    game->board.rook_on56_moved = 0;
    game->board.rook_on63_moved = 0;

    game->isCheck = -1;
    if (game->human_color == 1)
    {
        game->white_king = 60;
        game->black_king = 4;
    }
    else
    {
        game->white_king = 3;
        game->black_king = 59;
    }
    // global_board.is_game_over = 0;

    update_threat_map(game);
    return 3;
}

void calcReachablePositions(Game *game)
{
    int start_position = game->selected_position;

    game->board.reachable_positions = 0;

    if (game->possible_moves == NULL)
    {
        printf("Error: possible_moves is NULL\n");
        return;
    }

    if (game->possible_moves->head == NULL)
    {
        printf("Error: possible_moves->head is NULL\n");
        return;
    }

    Move *current_move = game->possible_moves->head;
    while (current_move != NULL)
    {
        if (current_move->origin == start_position)
        {
            game->board.reachable_positions |= position_to_Bitboard(current_move->target);
        }
        current_move = current_move->next;
    }
}

char get_piece_at_position(const ChessBoard *board, int position)
{
    Bitboard mask = 1ULL << position;

    // Quick check if square is empty
    if (!(mask & (board->white_pieces | board->black_pieces)))
        return '.';

    // Check white pieces
    if (mask & board->white_pieces)
    {
        if (mask & board->white_pawns)
            return 'P';
        if (mask & board->white_knights)
            return 'N';
        if (mask & board->white_bishops)
            return 'B';
        if (mask & board->white_rooks)
            return 'R';
        if (mask & board->white_queens)
            return 'Q';
        return 'K'; // Must be king if it's a white piece
    }

    // Check black pieces
    if (mask & board->black_pawns)
        return 'p';
    if (mask & board->black_knights)
        return 'n';
    if (mask & board->black_bishops)
        return 'b';
    if (mask & board->black_rooks)
        return 'r';
    if (mask & board->black_queens)
        return 'q';
    return 'k'; // Must be king if it's a black piece
}

Bitboard calculate_pawn_moves(Game *game, int start_position, int attacksOnly)
{
    // These determine how pawns move based on player color and board orientation
    int board_direction = (game->human_color == 1) ? -1 : 1;   // Direction from player's perspective
    int piece_direction = (game->is_white_turn == 1) ? 1 : -1; // Direction pieces should move
    int pawn_step = board_direction * piece_direction;         // Final movement direction for pawns

    Bitboard friendly_pieces = (game->is_white_turn == 1) ? game->board.white_pieces : game->board.black_pieces;
    Bitboard hostile_pieces = (game->is_white_turn == 1) ? game->board.black_pieces : game->board.white_pieces;

    Bitboard pawn_moves = 0;

    // Move forward
    int step1 = start_position + (8 * pawn_step);     // move one step forward
    int step2 = start_position + (2 * 8 * pawn_step); // move two steps forward

    // Check single step forward
    if (attacksOnly == 0)
    {
        if (step1 >= 0 && step1 < 64)
        {
            // Only allow move if square is empty (no friendly or hostile pieces)
            if (get_piece_at_position(&game->board, step1) == '.')
            {
                pawn_moves |= position_to_Bitboard(step1);

                // Only check double step if single step is possible
                if (step2 >= 0 && step2 < 64)
                {
                    // Check if pawn is on starting rank
                    if (game->is_white_turn)
                    {
                        // White pawns' starting rank
                        if ((game->human_color == 0 && 8 <= start_position && start_position <= 15) ||
                            (game->human_color == 1 && 48 <= start_position && start_position <= 55))
                        {
                            // Check if second square is also empty
                            if (get_piece_at_position(&game->board, step2) == '.')
                            {
                                pawn_moves |= position_to_Bitboard(step2);
                            }
                        }
                    }
                    else
                    {
                        // Black pawns' starting rank
                        if ((game->human_color == 0 && 48 <= start_position && start_position <= 55) ||
                            (game->human_color == 1 && 8 <= start_position && start_position <= 15))
                        {
                            // Check if second square is also empty
                            if (get_piece_at_position(&game->board, step2) == '.')
                            {
                                pawn_moves |= position_to_Bitboard(step2);
                            }
                        }
                    }
                }
            }
        }
    }

    // Calculate diagonal captures/threats
    int topLeft = start_position + (7 * pawn_step);
    int topRight = start_position + (9 * pawn_step);

    // Check if positions are within bounds
    if (topLeft >= 0 && topLeft < 64 && abs((topLeft % 8) - (start_position % 8)) == 1)
    {
        if (attacksOnly)
        {
            // For threat map, add diagonal regardless of piece presence
            pawn_moves |= position_to_Bitboard(topLeft);
        }
        else if (position_to_Bitboard(topLeft) & hostile_pieces)
        {
            // For actual moves, only add if enemy piece present
            pawn_moves |= position_to_Bitboard(topLeft);
        }
    }

    if (topRight >= 0 && topRight < 64 && abs((topRight % 8) - (start_position % 8)) == 1)
    {
        if (attacksOnly)
        {
            // For threat map, add diagonal regardless of piece presence
            pawn_moves |= position_to_Bitboard(topRight);
        }
        else if (position_to_Bitboard(topRight) & hostile_pieces)
        {
            // For actual moves, only add if enemy piece present
            pawn_moves |= position_to_Bitboard(topRight);
        }
    }

    // en passant
    if (game->board.last_move_double_pawn_push == 1)
    {
        int file_diff = abs((game->board.last_move_double_pawn_push_tile % 8) - (start_position % 8));
        int rank_diff = abs((game->board.last_move_double_pawn_push_tile / 8) - (start_position / 8));

        // Check if our pawn is adjacent to the double-pushed pawn
        if (file_diff == 1 && rank_diff == 0)
        {
            // Calculate the en passant capture square
            int capture_square = game->board.last_move_double_pawn_push_tile + (8 * pawn_step);

            // Add the en passant move
            if (capture_square >= 0 && capture_square < 64)
            {
                pawn_moves |= position_to_Bitboard(capture_square);
            }
        }
    }

    return pawn_moves;
}

Bitboard calculate_rook_moves(Game *game, int start_position, int include_friendly)
{
    int x = start_position % 8;
    int y = start_position / 8;
    int directions[4][2] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};

    Bitboard rook_moves = 0;
    Bitboard friendly_pieces = (game->is_white_turn == 1) ? game->board.white_pieces : game->board.black_pieces;
    Bitboard hostile_pieces = (game->is_white_turn == 1) ? game->board.black_pieces : game->board.white_pieces;

    for (int d = 0; d < 4; d++)
    {
        int new_x = x;
        int new_y = y;
        while (1)
        {
            new_x += directions[d][0];
            new_y += directions[d][1];

            // Check if the new position is within bounds
            if (new_x < 0 || new_x >= 8 || new_y < 0 || new_y >= 8)
            {
                break;
            }

            int new_position = new_y * 8 + new_x;
            Bitboard mask = 1ULL << new_position;

            // For threat map, include squares with friendly pieces
            if (include_friendly)
            {
                rook_moves |= mask;
                // Stop after hitting any piece (friendly or hostile)
                if ((mask & friendly_pieces) || (mask & hostile_pieces))
                {
                    break;
                }
            }
            else
            {
                // Original move calculation logic
                if (mask & friendly_pieces)
                {
                    break;
                }
                rook_moves |= mask;
                if (mask & hostile_pieces)
                {
                    break;
                }
            }
        }
    }

    return rook_moves;
}

Bitboard calculate_bishop_moves(Game *game, int start_position, int include_friendly)
{
    static const uint64_t not_a_file = 0xfefefefefefefefe;
    static const uint64_t not_h_file = 0x7f7f7f7f7f7f7f7f;

    Bitboard bishop = 1ULL << start_position;
    Bitboard occupied = game->board.white_pieces | game->board.black_pieces;
    Bitboard moves = 0;

    // Northwest
    Bitboard nw = bishop;
    while (nw & not_a_file)
    {
        nw = (nw << 7);
        moves |= nw;
        if (nw & occupied)
            break; // Stop after hitting any piece
    }

    // Northeast
    Bitboard ne = bishop;
    while (ne & not_h_file)
    {
        ne = (ne << 9);
        moves |= ne;
        if (ne & occupied)
            break; // Stop after hitting any piece
    }

    // Southwest
    Bitboard sw = bishop;
    while (sw & not_a_file)
    {
        sw = (sw >> 9);
        moves |= sw;
        if (sw & occupied)
            break; // Stop after hitting any piece
    }

    // Southeast
    Bitboard se = bishop;
    while (se & not_h_file)
    {
        se = (se >> 7);
        moves |= se;
        if (se & occupied)
            break; // Stop after hitting any piece
    }

    // If including friendly pieces for threat map
    if (include_friendly)
    {
        Bitboard friendly = game->is_white_turn ? game->board.white_pieces : game->board.black_pieces;
        moves |= (moves & friendly);
    }
    else
    {
        // Remove friendly pieces from possible moves
        Bitboard friendly = game->is_white_turn ? game->board.white_pieces : game->board.black_pieces;
        moves &= ~friendly;
    }

    return moves;
}

Bitboard calculate_queen_moves(Game *game, int start_position, int include_friendly)
{
    Bitboard queen_moves = 0;
    queen_moves |= calculate_rook_moves(game, start_position, include_friendly);
    queen_moves |= calculate_bishop_moves(game, start_position, include_friendly);
    return queen_moves;
}

Bitboard calculate_knight_moves(Game *game, int start_position, int include_friendly)
{
    // Pre-calculated knight move patterns for each square
    static const uint64_t knight_patterns[64] = {
        0x0000000000020400ULL, 0x0000000000050800ULL, 0x00000000000A1100ULL, 0x0000000000142200ULL,
        0x0000000000284400ULL, 0x0000000000508800ULL, 0x0000000000A01000ULL, 0x0000000000402000ULL,
        0x0000000002040004ULL, 0x0000000005080008ULL, 0x000000000A110011ULL, 0x0000000014220022ULL,
        0x0000000028440044ULL, 0x0000000050880088ULL, 0x00000000A0100010ULL, 0x0000000040200020ULL,
        0x0000000204000402ULL, 0x0000000508000805ULL, 0x0000000A1100110AULL, 0x0000001422002214ULL,
        0x0000002844004428ULL, 0x0000005088008850ULL, 0x000000A0100010A0ULL, 0x0000004020002040ULL,
        0x0000020400040200ULL, 0x0000050800080500ULL, 0x00000A1100110A00ULL, 0x0000142200221400ULL,
        0x0000284400442800ULL, 0x0000508800885000ULL, 0x0000A0100010A000ULL, 0x0000402000204000ULL,
        0x0002040004020000ULL, 0x0005080008050000ULL, 0x000A1100110A0000ULL, 0x0014220022140000ULL,
        0x0028440044280000ULL, 0x0050880088500000ULL, 0x00A0100010A00000ULL, 0x0040200020400000ULL,
        0x0204000402000000ULL, 0x0508000805000000ULL, 0x0A1100110A000000ULL, 0x1422002214000000ULL,
        0x2844004428000000ULL, 0x5088008850000000ULL, 0xA0100010A0000000ULL, 0x4020002040000000ULL,
        0x0400040200000000ULL, 0x0800080500000000ULL, 0x1100110A00000000ULL, 0x2200221400000000ULL,
        0x4400442800000000ULL, 0x8800885000000000ULL, 0x100010A000000000ULL, 0x2000204000000000ULL,
        0x0004020000000000ULL, 0x0008050000000000ULL, 0x00110A0000000000ULL, 0x0022140000000000ULL,
        0x0044280000000000ULL, 0x0088500000000000ULL, 0x0010A00000000000ULL, 0x0020400000000000ULL};

    Bitboard moves = knight_patterns[start_position];

    // If we don't want to include friendly pieces, mask them out
    if (!include_friendly)
    {
        Bitboard friendly_pieces = game->is_white_turn ? game->board.white_pieces : game->board.black_pieces;
        moves &= ~friendly_pieces;
    }

    return moves;
}

Bitboard calculate_king_moves(Game *game, int start_position)
{
    // Calculate x,y coordinates from the start_position
    int x = start_position % 8;
    int y = start_position / 8;

    int king_moves[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};

    Bitboard king_move = 0;
    Bitboard friendly_pieces = game->is_white_turn ? game->board.white_pieces : game->board.black_pieces;

    // Get enemy king's position
    Bitboard enemy_king = game->is_white_turn ? game->board.black_king : game->board.white_king;
    int enemy_king_pos = Bitboard_to_position(enemy_king);
    int enemy_king_x = enemy_king_pos % 8;
    int enemy_king_y = enemy_king_pos / 8;

    // Check each possible move direction
    for (int i = 0; i < 8; i++)
    {
        int new_x = x + king_moves[i][0];
        int new_y = y + king_moves[i][1];

        // Check if the new position is within bounds
        if (new_x >= 0 && new_x < 8 && new_y >= 0 && new_y < 8)
        {
            // Check if move is at least one square away from enemy king
            int dx = abs(new_x - enemy_king_x);
            int dy = abs(new_y - enemy_king_y);

            // Skip if too close to enemy king (adjacent squares including diagonals)
            if (dx <= 1 && dy <= 1)
            {
                continue;
            }

            int new_position = new_y * 8 + new_x;
            Bitboard mask = 1ULL << new_position;

            // Only add the move if it's not occupied by a friendly piece
            if (!(mask & friendly_pieces))
            {
                king_move |= mask;
            }
        }
    }

    // castling
    if (game->isCheck == -1)
    {
        Bitboard squares_in_between = 0; // squares between king and rook
        if (game->human_color == 1)
        {
            if (game->board.white_king_moved == 0 && game->is_white_turn == 1)
            {
                squares_in_between = position_to_Bitboard(57) | position_to_Bitboard(58) | position_to_Bitboard(59);
                if (game->board.rook_on56_moved == 0 && (game->board.white_pieces & squares_in_between) == 0)
                {
                    king_move |= position_to_Bitboard(58); // c1
                }
                squares_in_between = position_to_Bitboard(61) | position_to_Bitboard(62);
                if (game->board.rook_on63_moved == 0 && (game->board.white_pieces & squares_in_between) == 0)
                {
                    king_move |= position_to_Bitboard(62); // g1
                }
            }
            if (game->board.black_king_moved == 0 && game->is_white_turn == 0)
            {
                squares_in_between = position_to_Bitboard(1) | position_to_Bitboard(2) | position_to_Bitboard(3);
                if (game->board.rook_on0_moved == 0 && (game->board.black_pieces & squares_in_between) == 0)
                {
                    king_move |= position_to_Bitboard(2); // c8
                }
                squares_in_between = position_to_Bitboard(5) | position_to_Bitboard(6);
                if (game->board.rook_on7_moved == 0 && (game->board.black_pieces & squares_in_between) == 0)
                {
                    king_move |= position_to_Bitboard(6); // g8
                }
            }
        }
        else if (game->human_color == 0)
        {
            if (game->board.white_king_moved == 0 && game->is_white_turn == 1)
            {
                squares_in_between = position_to_Bitboard(1) | position_to_Bitboard(2);
                if (game->board.rook_on0_moved == 0 && (game->board.white_pieces & squares_in_between) == 0)
                {
                    king_move |= position_to_Bitboard(1);
                }
                squares_in_between = position_to_Bitboard(4) | position_to_Bitboard(5) | position_to_Bitboard(6);
                if (game->board.rook_on7_moved == 0 && (game->board.white_pieces & squares_in_between) == 0)
                {
                    king_move |= position_to_Bitboard(5);
                }
            }
            else if (game->board.black_king_moved == 0 && game->is_white_turn == 0)
            {
                squares_in_between = position_to_Bitboard(57) | position_to_Bitboard(58);
                if (game->board.rook_on56_moved == 0 && (game->board.black_pieces & squares_in_between) == 0)
                {
                    king_move |= position_to_Bitboard(57);
                }
                squares_in_between = position_to_Bitboard(60) | position_to_Bitboard(61) | position_to_Bitboard(62);
                if (game->board.rook_on63_moved == 0 && (game->board.black_pieces & squares_in_between) == 0)
                {
                    king_move |= position_to_Bitboard(61);
                }
            }
        }
    }
    return king_move;
}

void update_threat_map(Game *game)
{
    // Update combined piece positions
    game->board.white_pieces = game->board.white_pawns | game->board.white_knights | game->board.white_bishops | game->board.white_rooks | game->board.white_queens | game->board.white_king;
    game->board.black_pieces = game->board.black_pawns | game->board.black_knights | game->board.black_bishops | game->board.black_rooks | game->board.black_queens | game->board.black_king;

    game->board.white_threat_map = 0;
    game->board.black_threat_map = 0;

    game->white_king = Bitboard_to_position(game->board.white_king);
    game->black_king = Bitboard_to_position(game->board.black_king);

    int original_turn = game->is_white_turn;

    // Calculate white threats
    game->is_white_turn = 1; // Set to black's turn to calculate white threats
    Bitboard piece_bb = game->board.white_pawns;
    while (piece_bb)
    {
        int pos = get_and_clear_LSB(&piece_bb);
        game->board.white_threat_map |= calculate_pawn_moves(game, pos, 1);
    }
    piece_bb = game->board.white_knights;
    while (piece_bb)
    {
        int pos = get_and_clear_LSB(&piece_bb);
        game->board.white_threat_map |= calculate_knight_moves(game, pos, 1);
    }
    piece_bb = game->board.white_bishops;
    while (piece_bb)
    {
        int pos = get_and_clear_LSB(&piece_bb);
        game->board.white_threat_map |= calculate_bishop_moves(game, pos, 1);
    }
    piece_bb = game->board.white_rooks;
    while (piece_bb)
    {
        int pos = get_and_clear_LSB(&piece_bb);
        game->board.white_threat_map |= calculate_rook_moves(game, pos, 1);
    }
    piece_bb = game->board.white_queens;
    while (piece_bb)
    {
        int pos = get_and_clear_LSB(&piece_bb);
        game->board.white_threat_map |= calculate_queen_moves(game, pos, 1);
    }
    piece_bb = game->board.white_king;
    while (piece_bb)
    {
        int pos = get_and_clear_LSB(&piece_bb);
        game->board.white_threat_map |= calculate_king_moves(game, pos);
    }

    // Calculate black threats
    game->is_white_turn = 0; // Set to white's turn to calculate black threats
    piece_bb = game->board.black_pawns;
    while (piece_bb)
    {
        int pos = get_and_clear_LSB(&piece_bb);
        game->board.black_threat_map |= calculate_pawn_moves(game, pos, 1);
    }
    piece_bb = game->board.black_knights;
    while (piece_bb)
    {
        int pos = get_and_clear_LSB(&piece_bb);
        game->board.black_threat_map |= calculate_knight_moves(game, pos, 1);
    }
    piece_bb = game->board.black_bishops;
    while (piece_bb)
    {
        int pos = get_and_clear_LSB(&piece_bb);
        game->board.black_threat_map |= calculate_bishop_moves(game, pos, 1);
    }
    piece_bb = game->board.black_rooks;
    while (piece_bb)
    {
        int pos = get_and_clear_LSB(&piece_bb);
        game->board.black_threat_map |= calculate_rook_moves(game, pos, 1);
    }
    piece_bb = game->board.black_queens;
    while (piece_bb)
    {
        int pos = get_and_clear_LSB(&piece_bb);
        game->board.black_threat_map |= calculate_queen_moves(game, pos, 1);
    }
    piece_bb = game->board.black_king;
    while (piece_bb)
    {
        int pos = get_and_clear_LSB(&piece_bb);
        game->board.black_threat_map |= calculate_king_moves(game, pos);
    }

    // Restore original turn
    game->is_white_turn = original_turn;
}
