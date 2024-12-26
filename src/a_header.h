#ifndef A_Header
#define A_Header

#include <SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include </opt/homebrew/Cellar/sdl2_image/2.8.2_2/include/SDL2/SDL_image.h>
#include </opt/homebrew/opt/sdl2/include/SDL2/SDL.h>
#include </opt/homebrew/opt/sdl2_ttf/include/SDL2/SDL_ttf.h>

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768
#define BOARD_WIDTH 768
#define BOARD_HEIGHT 768
#define BOARD_SIZE 8
#define SQUARE_SIZE (BOARD_WIDTH / BOARD_SIZE)

#define BUTTON_WIDTH 200
#define BUTTON_HEIGHT 50

// 64-bit unsigned integer to represent a bitboard
typedef uint64_t Bitboard;
#define MAX_REACHABLE_POSITIONS 64

typedef struct
{
    Bitboard white_pawns;
    Bitboard white_knights;
    Bitboard white_bishops;
    Bitboard white_rooks;
    Bitboard white_queens;
    Bitboard white_king;
    Bitboard black_pawns;
    Bitboard black_knights;
    Bitboard black_bishops;
    Bitboard black_rooks;
    Bitboard black_queens;
    Bitboard black_king;

    Bitboard white_pieces;
    Bitboard black_pieces;

    Bitboard reachable_positions;
    Bitboard white_threat_map;
    Bitboard black_threat_map;

    int last_move_double_pawn_push;
    int last_move_double_pawn_push_tile;

    int promotion_tile; // tile where promotion is happening (last rank)

    // Castling rights flags
    int white_king_moved;
    int black_king_moved;
    int rook_on0_moved;
    int rook_on7_moved;
    int rook_on56_moved;
    int rook_on63_moved;
} ChessBoard;

// Structure for a single move
typedef struct Move
{
    int origin;           // Origin position (0-63)
    int target;           // Target position (0-63)
    char captured;        // Captured piece (or '.' if none)
    char promotion_piece; // Promotion piece (or '.' if none)
    struct Move *next;    // Pointer to next move
    struct Move *prev;    // Pointer to previous move
} Move;

// Structure for the move list
typedef struct MoveList
{
    Move *head; // First move in the list
    Move *tail; // Last move in the list
    int size;   // Number of moves in the list
} MoveList;

typedef struct
{
    int score;        // Final position score
    Move moves[4096]; // Array to store the moves in the line
    int length;       // Number of moves in the line
} SearchResult;

// the structure that holds all the chess game information
typedef struct
{
    ChessBoard board;           // game board
    MoveList *move_history;     // move history
    MoveList *possible_moves;   // all possible moves
    int numPlayer;              // indicates the number of humans playing.
    int human_color;            // 1- white 0- black
    int currentPlayer;          // 1-white 2-black
    int white_king, black_king; // both kings positions are kept here.
    int isCheck;                // -1: no check, 0: Stalemate, 1: white check, 2: black check, 3: white checkmate, 4: black checkmate, 10: both in check
    int selected_position;      // selected position
    int is_white_turn;          // 1-white 0-black
} Game;

typedef struct
{
    SDL_Rect rect;         // Position and size
    SDL_Color color;       // Button color
    SDL_Color textColor;   // Text color
    char *text;            // Button text
    int isHovered;         // Hover state
    void (*onClick)(void); // Click callback function
} Button;

#define MAX_MOVES 256

extern SDL_Renderer *renderer;
extern ChessBoard global_board;
// Function prototypes

// objRenderer.c
Button *createButton(int x, int y, const char *text, void (*onClick)(void));
void destroyButton(Button *button);
void drawButton(SDL_Renderer *renderer, Button *button, TTF_Font *font);
int isMouseOverButton(Button *button, int mouseX, int mouseY);
void renderText(SDL_Renderer *renderer, TTF_Font *font, const char *text,
                int x, int y, SDL_Color color);

// mainAux.c
int mainAuxRunGameGUI(void);

// gui.c
int runMainMenu(SDL_Window **window, Game **game);
int runGameWindow(SDL_Window **window, Game **game);
char runPromotionWindow(SDL_Window **window, Game **game);

// textures.c
SDL_Texture *piece_textures[12];
void load_piece_textures(SDL_Renderer *renderer);
void cleanup_piece_textures();

// game.c
Game *initGame();
void print_board(ChessBoard *board);
void move(Game *game, int start_position, int end_position);
int handle_promotion(ChessBoard *board, char promotion_piece);
int handle_castling(Game *game, char piece, int start_position, int end_position);
void toggle_turn(Game *game);
/*
Game state values:
-1: No check/normal play
 0: Stalemate
 1: White in check
 2: Black in check
 3: White is checkmated
 4: Black is checkmated
*/
void check_check(Game *game);
void check_checkmate(Game *game);

// helperFunctions.c
void print_bitboard(Bitboard bb);
MoveList *calculate_all_moves(Game *game, int color);
int Bitboard_to_position(Bitboard bb);
Bitboard position_to_Bitboard(int position);
void position_to_notation(int position, char *notation);
int get_and_clear_LSB(Bitboard *bb);
int is_move_legal(Game *game, int start_position, int end_position);
int max(int a, int b);
int min(int a, int b);
void print_piece_values_board(Game *game);

// bitboard.c
int initialize_board(Game *game);
void calcReachablePositions(Game *game);
char get_piece_at_position(const ChessBoard *board, int position);
void update_threat_map(Game *game);

Bitboard calculate_pawn_moves(Game *game, int start_position, int attacksOnly);
Bitboard calculate_rook_moves(Game *game, int start_position, int include_friendly);
Bitboard calculate_bishop_moves(Game *game, int start_position, int include_friendly);
Bitboard calculate_knight_moves(Game *game, int start_position, int include_friendly);
Bitboard calculate_queen_moves(Game *game, int start_position, int include_friendly);
Bitboard calculate_king_moves(Game *game, int start_position);

// moveList.c
MoveList *createMoveList();
void free_move_list(MoveList *list);
int addMove(MoveList *list, int origin, int target, char captured, char promotion_piece);
Move *removeLastMove(MoveList *list);
void printMoves(MoveList *list);
void clearMoveList(MoveList *list);

// chessEngine.c
int engine_move(Game *game);
int evaluate_board(Game *game);
int piece_value(char piece, int position);
SearchResult minimax(Game *game, int depth, int alpha, int beta, long long *positions_counted);

#endif // A_Header
