#ifndef DISPLAY_H
#define DISPLAY_H

#include <list>
#include "field.h"
#include "score.h"

#define FPS 24
#define PARTICLINESS 30

void draw(SDL_Surface* screen);

class Particle
{
	private:
		int x;
		int y;
		int xvel;
		int yvel;
                int xaccel;
                int yaccel;
                int width;
                int height;
		int life;
                int maxlife;
		SDL_Color color;
                Uint8 flags;

                const static int ppoolSize = PARTICLINESS * 100;
                static Particle ppool[];
		static Particle* particlePool();

		Particle(int X, int Y, SDL_Color Color);
	public:
                Particle() : life(0) { }
		~Particle();
		void display(SDL_Surface* screen);
		static void createParticles(int num, int x, int y, unsigned char color);
		static void createBouncyParticles(int num, int x, int y, unsigned char color);
                static void displayAll(SDL_Surface* screen);

                int getLife() { return life; }
		
};


#endif // DISPLAY_H
