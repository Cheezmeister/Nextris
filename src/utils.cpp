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

Scintillator::Scintillator(int frequency) : 
	time(SDL_GetTicks())
	{
	setFrequency(frequency);
	}
void Scintillator::setFrequency(int frequency)
	{
	currentop = 0;
	currentindex = 1;
	freq = frequency;
	rgb[0] = 255;
	rgb[1] = 0;
	rgb[2] = 255;
	}

SDL_Color Scintillator::color()
	{
	
	SDL_Color ret;
	
	
	if (SDL_GetTicks() - time > 1000 / freq)
		{
		time = SDL_GetTicks();
		//change direction if we need to
		if (rgb[0] + rgb[1] + rgb[2] == 510)
			{
			currentop = -5;
			for (currentindex = rand() % 3; rgb[currentindex] == 0; currentindex = rand() % 3);
			}
		else if (rgb[0] + rgb[1] + rgb[2] == 255)
			{
			currentop = 5;
			for (currentindex = rand() % 3; rgb[currentindex] == 255; currentindex = rand() % 3);
			}
		//increment our current element in the right direction
		rgb[currentindex] += currentop;
		}

	ret.r = rgb[0];
	ret.g = rgb[1];
	ret.b = rgb[2];
	return ret;	
	}


