#include "a_header.h"

// Initialize a new move list
MoveList *createMoveList()
{
    MoveList *list = (MoveList *)malloc(sizeof(MoveList));
    if (list == NULL)
    {
        printf("ERROR: Failed to create move list\n");
        return NULL;
    }

    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    return list;
}

// Add a move to the end of the list
int addMove(MoveList *list, int origin, int target, char captured, char promotion_piece)
{
    if (list == NULL)
    {
        printf("ERROR: addMove -Move list is NULL\n");
        return 0;
    }

    Move *newMove = (Move *)malloc(sizeof(Move));
    if (newMove == NULL)
    {
        return 0; // Failed to allocate memory
    }

    newMove->origin = origin;
    newMove->target = target;
    newMove->captured = captured;
    newMove->promotion_piece = promotion_piece;
    newMove->next = NULL;
    newMove->prev = list->tail;

    if (list->tail != NULL)
    {
        list->tail->next = newMove;
    }
    list->tail = newMove;

    if (list->head == NULL)
    {
        list->head = newMove;
    }

    list->size++;
    return 1; // Success
}

// Remove the last move from the list
Move *removeLastMove(MoveList *list)
{
    if (list->tail == NULL)
    {
        return NULL;
    }

    Move *lastMove = list->tail;
    list->tail = lastMove->prev;

    if (list->tail != NULL)
    {
        list->tail->next = NULL;
    }
    else
    {
        list->head = NULL; // List is now empty
    }

    list->size--;
    return lastMove; // Caller is responsible for freeing this memory
}

// Clear all moves from the list but keep the list structure intact
void clearMoveList(MoveList *list)
{
    if (list == NULL)
    {
        printf("ERROR: Move list is NULL\n");
        return;
    }

    Move *current = list->head;
    while (current != NULL)
    {
        Move *next = current->next;
        free(current);
        current = next;
    }

    // Reset list properties
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

void free_move_list(MoveList *list)
{
    if (list == NULL)
        return;

    Move *current = list->head;
    while (current != NULL)
    {
        Move *next = current->next;
        free(current);
        current = next;
    }
    free(list);
}

// Print all moves in the list
void printMoves(MoveList *list)
{
    if (list == NULL)
    {
        printf("ERROR: Move list is NULL\n");
        return;
    }

    if (list->size == 0)
    {
        printf("No moves in list\n");
        return;
    }

    Move *current = list->head;
    int moveNumber = 1;

    printf("Move List (total moves: %d):\n", list->size);
    while (current != NULL)
    {
        // Convert positions to chess notation (e.g., e2 to e4)
        char originFile = 'a' + (current->origin % 8);
        int originRank = 8 - (current->origin / 8);
        char targetFile = 'a' + (current->target % 8);
        int targetRank = 8 - (current->target / 8);

        printf("Move %d: %c%d -> %c%d",
               moveNumber,
               originFile, originRank,
               targetFile, targetRank);

        if (current->captured != '.')
        {
            printf(" (captured: %c)", current->captured);
        }
        if (current->promotion_piece != '.')
        {
            printf(" (promoted to: %c)", current->promotion_piece);
        }
        printf("\n");

        current = current->next;
        moveNumber++;
    }
}
