#ifndef UTILS_H
#define UTILS_H

#include <SDL.h>
#include <iostream>
#include <cmath>

// Support old-school SDL
#if ((SDL_MAJOR_VERSION) == 1 && (SDL_MINOR_VERSION) == 2)
#  define SDL_GetKeyboardState SDL_GetKeyState
#endif

#define SDLtoU32(x) (SDL_MapRGB(screen->format, (x).r, (x).g, (x).b) )
#define FADE_COLOR(x) ((x) & 0x08080808)

enum Gradient {
    GT_RADIAL,
    GT_VERTICAL,
};

#define GRADIENT 20
void fill_gradient(SDL_Surface* screen, SDL_Rect& loc, SDL_Color cent);
void vertical_gradient(SDL_Surface* screen, SDL_Rect& loc, SDL_Color bottom);

template<typename T, typename U>
T lerp(T a, U b, float t)
{
    return a + (b - a) * t;
}

static SDL_Color h11_to_rgb(float hue)
{
    hue *= 6.0;
    Uint8 chroma = 255; // S and V are both 1
    float fmf = (hue < 2.0 ? hue : hue < 4.0 ? hue - 2.0 : hue - 4.0);
    float fbs = fmf - 1 > 0 ? fmf - 1 : 1 - fmf;
    Uint8 X = (Uint8)(255 * (1.0 - fbs));

    SDL_Color r = {0, 0, 0, 0};
    if (hue < 1.0)
    {
        SDL_Color c = {chroma, X, 0, 0};
        return c;
    }
    if (hue < 2.0)
    {
        SDL_Color c = {X, chroma, 0, 0};
        return c;
    }
    if (hue < 3.0)
    {
        SDL_Color c = {0, chroma, X, 0};
        return c;
    }
    if (hue < 4.0)
    {
        SDL_Color c = {0, X, chroma, 0};
        return c;
    }
    if (hue < 5.0)
    {
        SDL_Color c = {X, 0, chroma, 0};
        return c;
    }
    if (hue < 6.0)
    {
        SDL_Color c = {chroma, 0, X, 0};
        return c;
    }
    return r;
}
static SDL_Color fade_color(const SDL_Color& color, int fade_factor = 8)
{
    SDL_Color ret = {
        color.r / fade_factor,
        color.g / fade_factor,
        color.b / fade_factor,
        //color.unused / fade_factor
    };
    return ret;
}
static Uint32 sdlc_to_u32(const SDL_Color& color, const SDL_Surface* surf)
{
    return SDL_MapRGB(surf->format, color.r, color.g, color.b);
}


class Scintillator
{
private:
    double freq;
    unsigned long time;
    double hue; // from 0 to 1.0

public:
    Scintillator() { }
    Scintillator(double frequency);
    void setFrequency(double frequency);
    void setHue(double hue);
    SDL_Color color();
};

template <unsigned int Size>
class ScintillatingPalette
{
private:
    Scintillator colors[Size];

public:
    ScintillatingPalette()
    {
        for (int i = 0; i < Size; ++i)
        {
            colors[i].setFrequency(.15 + .1 * (rand() % 20));
            colors[i].setHue(1.0 * (double)i / Size);
        }
    }
    ScintillatingPalette(double frequency)
    {
        for (int i = 0; i < Size; ++i)
        {
            colors[i].setFrequency(frequency);
            colors[i].setHue(1.0 * (double)i / Size);
        }
    }
    SDL_Color operator[] (const int index)
    {
        if (index > Size)
        {
            SDL_Color col = {128, 128, 128, 128};
            return col;
        }
        return colors[index].color();
    }
};
#endif //utils.h

