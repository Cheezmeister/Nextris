#include <cmath>
#include "display.h"

enum {
  PF_NONE   = 0,
  PF_BOUNCY = (1 << 0),
  PF_SPINNY = (1 << 1),
  PF_ALL    = ~0,
};
const SDL_Color COLORS[] = {
	{255, 0, 0}, //red
	{0, 255, 0}, //green
	{0, 0, 255}, //blue
	{255, 255, 0}, //yellow
	{0, 255, 255}, //cyan
	{255, 0, 255} //magenta
};

Particle Particle::ppool[Particle::ppoolSize];
		
void draw(SDL_Surface* screen)
	{
	//color scintillator for the right panel
	static Scintillator scint(FPS);
	//where we draw the field
	static SDL_Rect fieldRect = {0, 0, 1, 1};
	static SDL_Surface* fieldDisp = SDL_CreateRGBSurface(
								SDL_SWSURFACE,
								FIELD_WIDTH  * BLOCK_WIDTH,
								FIELD_HEIGHT * BLOCK_WIDTH,
								32,
								0, 0, 0, 0
								);
	//where we draw the next tetra to fall
	static SDL_Rect nextRect = {SCORE_X * BLOCK_WIDTH + GRADIENT, GRADIENT, 1, 1};
	static SDL_Surface* nextDisp = SDL_CreateRGBSurface(
								SDL_SWSURFACE,
								3 * BLOCK_WIDTH,
								4 * BLOCK_WIDTH,
								32,
								0, 0, 0, 0
								);
	//where we draw the score
	static SDL_Rect scoreRect = {(FIELD_WIDTH + 1) * BLOCK_WIDTH, nextRect.y + nextDisp->h + BLOCK_WIDTH, 1, 1};
	static SDL_Surface* scoreDisp = SDL_CreateRGBSurface(
								SDL_SWSURFACE,
								LEVEL_HARD * BLOCK_WIDTH,
								9 * BLOCK_WIDTH,
								32,
								0, 0, 0, 0
								);
	//where we draw the chain indicator
	SDL_Rect chainRect = {scoreRect.x + scoreDisp->w, scoreRect.y, 4, scoreDisp->h};
	Uint8 chain = playField().chaining() * 255;
	
	//the area of the right panel i.e. the non-field part of the display
	SDL_Rect rpanel = {SCORE_X * BLOCK_WIDTH,
					   0,
					   SCORE_WIDTH * BLOCK_WIDTH,
					   FIELD_HEIGHT * BLOCK_WIDTH
					   };

	SDL_FillRect(fieldDisp, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
	SDL_FillRect(nextDisp,  NULL, SDL_MapRGB(screen->format, 0, 0, 0));
	SDL_FillRect(scoreDisp, NULL, SDL_MapRGB(screen->format, 0, 0, 0));

	fill_gradient(screen, rpanel, scint.color() );
	
	if (chain)
		fill_gradient(screen, chainRect, scint.color() );
	
	
	playField().display(fieldDisp);
	playField().displayNextQuad(nextDisp);
	playerScore().display(scoreDisp);

	SDL_BlitSurface(fieldDisp, NULL, screen, &fieldRect);
	SDL_BlitSurface(nextDisp, NULL, screen, &nextRect);
	SDL_BlitSurface(scoreDisp, NULL, screen, &scoreRect);

	cdebug << "Score is " << playerScore().getTotal() << "\n";
	

        Particle::displayAll(screen);

	cdebug << "Updating screen\n";

	SDL_Flip(screen);
	}


Particle* Particle::particlePool()
	{
	return ppool;
	}
	
void Particle::displayAll(SDL_Surface* screen)
	{
	for (int i = 0; i < ppoolSize; ++i)
		{
		ppool[i].display(screen);
		}
	}
void Particle::createBouncyParticles(int num, int x, int y, unsigned char color)
{
	for (int i = 0, made = 0; i < ppoolSize && made < num; ++i)
        {
        	if (ppool[i].life <= 0)
                {
                	ppool[i] = Particle(x, y, COLORS[color]);
                        ppool[i].xaccel  = 0;
                        ppool[i].yaccel  = 0;
                        ppool[i].maxlife = 200;
                        ppool[i].life    = 200;
                        ppool[i].flags  |= PF_BOUNCY;
                        ++made;
                }
        }
}
void Particle::createParticles(int num, int x, int y, unsigned char color)
{
	for (int i = 0, made = 0; i < ppoolSize && made < num; ++i)
        {
        	if (ppool[i].life <= 0)
                {
                	ppool[i] = Particle(x, y, COLORS[color]);
                        ppool[i].xaccel = 0;
                        ppool[i].yaccel = 1;
                        ppool[i].flags |= PF_SPINNY;
                        ++made;
                }
        }
}
	
Particle::Particle(int X, int Y, SDL_Color Color) : x(X), y(Y)
	{
	color = Color;
	xvel = rand() % 30 - 15;
	yvel = rand() % 20 - 15;
        xaccel = 0;
        yaccel = 0;
        width = 1;
        height = 1;
	life = 100;
	maxlife = 100;
	}

Particle::~Particle()
	{
	}

void Particle::display(SDL_Surface* screen)
	{
	if (--life < 0 || x < 0 || y < 0 || x > screen->w || y > screen->h)
		{
		life = 0;
		return;
		}
		
	x += xvel; //update location
	y += yvel;

        xvel += xaccel;
        yvel += yaccel;

	SDL_Rect rect = {x, y, width, height};
        if (flags & PF_SPINNY)
        {
	  SDL_Rect temp = {x, y, life % 4 + width, (life+2) % 4 + height};
          rect = temp;
        }

        if (flags & PF_BOUNCY)
		{
	 	if (y >= FIELD_HEIGHT * BLOCK_WIDTH || y <= 0)
	 		yvel = -yvel;
	 	if (x >= FIELD_WIDTH * BLOCK_WIDTH || x <= 0)
			xvel = -xvel;
		}
	
	color.r = color.r * 15 / 16;
	color.g = color.g * 15 / 16;
	color.b = color.b * 15 / 16;
        SDL_Color out = {
          color.r * life / maxlife, 
          color.g * life / maxlife, 
          color.b * life / maxlife, 
          color.unused * life / maxlife, 
        };
	SDL_FillRect(screen, &rect, SDLtoU32(color) ); //draw
	}
