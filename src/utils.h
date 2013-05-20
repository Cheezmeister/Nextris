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

  if (hue < 1.0)
  {
    SDL_Color c = {chroma, X, 0, 0}; return c;
  }
  if (hue < 2.0)
  {
    SDL_Color c = {X, chroma, 0, 0}; return c;
  }
  if (hue < 3.0)
  {
    SDL_Color c = {0, chroma, X, 0}; return c;
  }
  if (hue < 4.0)
  {
    SDL_Color c = {0, X, chroma, 0}; return c;
  }
  if (hue < 5.0)
  {
    SDL_Color c = {X, 0, chroma, 0}; return c;
  }
  if (hue < 6.0)
  {
    SDL_Color c = {chroma, 0, X, 0}; return c;
  }
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

template <unsigned int N>
class StochasticScintillator
{

  private: double _frequency;
           unsigned long _time;
           double _hues[N];
           double _hueVelocities[N];

           static const double SCALE = 360;

           Uint32 lastTicks;

  public:
           StochasticScintillator() { }
           StochasticScintillator(double frequency) 
             : _frequency(frequency) 
           { 
             double velSum = 0.0;
             for (int i = 0; i < N; ++i)
             {
               _hues[i] = rand() % (int)SCALE;
               _hueVelocities[i] = SCALE * frequency / N - (double)(rand() % 10);
               velSum += _hueVelocities[i];
             }
           }

           SDL_Color color(int index)
           {

             Uint32 ticks = SDL_GetTicks();
             Uint32 last = lastTicks;
             

             Uint32 diff = ticks - last;

             if (diff < 100)
             {
               update(diff);
             }

             lastTicks = ticks;

             SDL_Color retVal = h11_to_rgb(_hues[index] / SCALE);
             return retVal;
           }

           void update(int diff)
           {
             for (int i = 0; i < N; ++i)
             {
               _hues[i] += _hueVelocities[i] * diff / 1000;
               if (_hues[i] > SCALE)
               {
                 _hues[i] -= SCALE;
               }
               else if (_hues[i] < 0)
               {
                 _hues[i] += SCALE;
               }
               for (int j = 0; j < N; ++j)
               {
                 if (i == j) continue;

                 double distance = fabs(_hues[i] - _hues[j]);

                 if (distance > (SCALE / N)) continue;
                 if (distance < (SCALE / 1000)) continue;

                 double acceleration = SCALE / 5.0f / (distance * distance);
                 _hueVelocities[i] += acceleration * diff / 1000;
                 _hueVelocities[j] -= acceleration * diff / 1000;
               }
             }
           }

           SDL_Color operator[] (const int index)
           {
             SDL_Color ret = color(index);
             return ret;
           }
};

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

