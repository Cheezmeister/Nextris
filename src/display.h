#ifndef DISPLAY_H
#define DISPLAY_H

#include <list>
#include "field.h"
#include "score.h"

#define FPS 20
#define PARTICLINESS 30

void draw(SDL_Surface* screen);

class Particle
{
	private:
		int x;
		int y;
		int xvel;
		int yvel;
		SDL_Color color;
		int life;
		Particle(int X, int Y, unsigned char Color);
	public:
		~Particle();
		void display(SDL_Surface* screen);
		static std::list<Particle*>& particlePool();
		static void createParticle(int x, int y, unsigned char color);
		
};

#endif // DISPLAY_H
