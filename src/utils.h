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
		Scintillator(int frequency);
		SDL_Color color();
	};

#endif //utils.h

