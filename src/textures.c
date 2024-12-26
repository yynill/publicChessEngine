#include "a_header.h"

void load_piece_textures(SDL_Renderer *renderer)
{
    const char *piece_files[] = {"wp.svg", "wn.svg", "wb.svg", "wr.svg", "wq.svg", "wk.svg", "bp.svg", "bn.svg", "bb.svg", "br.svg", "bq.svg", "bk.svg"};

    for (int i = 0; i < 12; i++)
    {
        char path[100];
        snprintf(path, sizeof(path), "assets/piece/maestro/%s", piece_files[i]);
        SDL_Surface *surface = IMG_Load(path);
        if (!surface)
        {
            fprintf(stderr, "Failed to load image %s: %s\n", path, IMG_GetError());
            continue;
        }
        piece_textures[i] = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);

        if (!piece_textures[i])
        {
            fprintf(stderr, "Failed to create texture for %s\n", piece_files[i]);
        }
    }
}

void cleanup_piece_textures()
{
    for (int i = 0; i < 12; i++)
    {
        if (piece_textures[i])
        {
            SDL_DestroyTexture(piece_textures[i]);
        }
    }
}