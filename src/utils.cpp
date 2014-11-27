#include "utils.h"

void vertical_gradient(SDL_Surface* screen, SDL_Rect& loc, SDL_Color bottom)
{
    int lines = (loc.h < 255 ? loc.h : 255);
    Uint8 r, g, b;
    SDL_Rect dest = loc;
    for (int i = 0; i < lines; ++i)
    {
        float t = (i / (float) lines);
        dest.h = lerp(loc.h, (Uint16)0, t);
        r = lerp(bottom.r, (Uint8)0, t);
        g = lerp(bottom.g, (Uint8)0, t);
        b = lerp(bottom.b, (Uint8)0, t);

        SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format, r, g, b));
    }
}

void fill_gradient(SDL_Surface* screen, SDL_Rect& loc, SDL_Color cent)
{
    Uint32 shade;
    Uint8 r, g, b;
    for (int i = 0; i <= GRADIENT; ++i)
    {
        r = cent.r - cent.r * i / GRADIENT;
        g = cent.g - cent.g * i / GRADIENT;
        b = cent.b - cent.b * i / GRADIENT;
        shade = SDL_MapRGB(screen->format, r, g, b);//i * 255 / GRADIENT);
        SDL_FillRect(screen, &loc, shade);
        if (loc.w > 1)
        {
            loc.x += 1;
            loc.w -= 2;
        }
        if (loc.h > 1)
        {
            loc.y += 1;
            loc.h -= 2;
        }
    }
    SDL_FillRect(screen, &loc, 0);
}

Scintillator::Scintillator(double frequency) :
    time(SDL_GetTicks())
{
    setFrequency(frequency);
}
void Scintillator::setHue(double hue)
{
    this->hue = hue;
}

void Scintillator::setFrequency(double frequency)
{
    freq = frequency;
    hue = 0;
}

SDL_Color Scintillator::color()
{
    Uint32 ticks = SDL_GetTicks();
    if (ticks == time) return h11_to_rgb(hue);

    hue += (ticks - time) * freq / 1000.0;

    if (hue >= 1.0)
    {
        hue -= 1.0;
    }


    SDL_Color ret = h11_to_rgb(hue);
    time = ticks;
    return ret;
}


