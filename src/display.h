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
	protected:
		int x;
		int y;
		int xvel;
		int yvel;
		SDL_Color color;
		int life;
		Particle(int X, int Y, unsigned char Color);
                const static int ppoolSize = PARTICLINESS * 100;
                static Particle ppool[];
		static Particle* particlePool();
	public:
                Particle() : life(0) { }
		virtual ~Particle();
		virtual void display(SDL_Surface* screen);
		static void createParticle(int x, int y, unsigned char color);
		static void createParticles(int num, int x, int y, unsigned char color);
                static void displayAll(SDL_Surface* screen);

                int getLife() { return life; }
		
};

class BouncyParticle : public Particle
{
	private:
		BouncyParticle(int X, int Y, unsigned char Color);
	public:
		virtual ~BouncyParticle();
		virtual void display(SDL_Surface* screen);
		static void createBouncyParticle(int x, int y, unsigned char color);
};

#endif // DISPLAY_H
