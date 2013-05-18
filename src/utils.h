#ifndef UTILS_H
#define UTILS_H

#include <SDL.h>
#include <iostream>

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

template<typename T>
T lerp(T a, T b, float t)
{
  return a + (b - a) * t;
}

static SDL_Color fade_color(const SDL_Color& color, int fade_factor = 8)
{
  SDL_Color ret = {
    color.r / fade_factor,
    color.g / fade_factor,
    color.b / fade_factor,
    color.unused / fade_factor
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
		unsigned int freq;
		Uint8 rgb[3];
		unsigned long time;
		int currentop;
		int currentindex;
	
	public:
		Scintillator() { }
		Scintillator(int frequency);
		void setFrequency(int frequency);
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
				colors[i].setFrequency(15 + rand() % 20);
			}
		ScintillatingPalette(int frequency)
			{
			for (int i = 0; i < Size; ++i)
				colors[i].setFrequency(frequency);
			}
		SDL_Color operator[] (const int index)
			{
			if (index > Size)
				{
				SDL_Color col = {128, 128, 128, 128};
				return col;
				}
                        // std::cerr << colors[index].color();
			return colors[index].color();
			}
	};
#endif //utils.h

