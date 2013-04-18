#ifndef UTILS_H
#define UTILS_H

#include <SDL.h>
#include <iostream>


#define SDLtoU32(x) (SDL_MapRGB(screen->format, (x).r, (x).g, (x).b) )
#define FADE_COLOR(x) ((x) & 0x08080808)

#define GRADIENT 20
void fill_gradient(SDL_Surface* screen, SDL_Rect& loc, SDL_Color cent);

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
			return colors[index].color();
			}
	};
#endif //utils.h

