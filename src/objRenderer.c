#include "a_header.h"

Button *createButton(int x, int y, const char *text, void (*onClick)(void))
{
    Button *button = (Button *)malloc(sizeof(Button));
    button->rect = (SDL_Rect){x, y, BUTTON_WIDTH, BUTTON_HEIGHT};
    button->color = (SDL_Color){100, 150, 200, 255};
    button->textColor = (SDL_Color){0, 0, 0, 255};
    button->text = strdup(text); // Make a copy of the text
    button->onClick = onClick;
    return button;
}

void destroyButton(Button *button)
{
    if (button)
    {
        free(button->text);
        free(button);
    }
}

void drawButton(SDL_Renderer *renderer, Button *button, TTF_Font *font)
{
    // Draw button background
    SDL_Color currentColor = button->color;

    SDL_SetRenderDrawColor(renderer, currentColor.r, currentColor.g,
                           currentColor.b, currentColor.a);
    SDL_RenderFillRect(renderer, &button->rect);

    // Render text
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, button->text,
                                                    button->textColor);
    if (textSurface)
    {
        SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer,
                                                                textSurface);
        if (textTexture)
        {
            SDL_Rect textRect = button->rect;
            // Center the text
            textRect.x += (button->rect.w - textSurface->w) / 2;
            textRect.y += (button->rect.h - textSurface->h) / 2;
            textRect.w = textSurface->w;
            textRect.h = textSurface->h;

            SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
            SDL_DestroyTexture(textTexture);
        }
        SDL_FreeSurface(textSurface);
    }
}

int isMouseOverButton(Button *button, int mouseX, int mouseY)
{
    return mouseX >= button->rect.x && mouseX <= button->rect.x + button->rect.w &&
           mouseY >= button->rect.y && mouseY <= button->rect.y + button->rect.h;
}

void renderText(SDL_Renderer *renderer, TTF_Font *font, const char *text,
                int x, int y, SDL_Color color)
{
    SDL_Surface *surface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dest = {x, y, surface->w, surface->h};

    SDL_RenderCopy(renderer, texture, NULL, &dest);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}