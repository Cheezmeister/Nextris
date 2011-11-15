#include "display.h"

static std::list<Particle*> ppool;
		
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
	
	std::list<Particle*> temppool(Particle::particlePool() );
		
	for (std::list<Particle*>::iterator it = temppool.begin(); it != temppool.end(); ++it)
		{
		(*it)->display(screen);
		}
	cdebug << "Updating screen\n";

	SDL_Flip(screen);
	}


std::list<Particle*>& Particle::particlePool()
	{
	return ppool;
	}
	
void Particle::createParticle(int x, int y, unsigned char color)
	{
	ppool.push_back(new Particle(x, y, color) );
	}
	
Particle::Particle(int X, int Y, unsigned char Color) : x(X), y(Y)
	{
	life = 100;
	color = COLORS[Color];
	xvel = rand() % 30 - 15;
	yvel = rand() % 20 - 15;
	}

Particle::~Particle()
	{
	cdebug << "Destructing!\n";
	ppool.remove(this);
	}

void Particle::display(SDL_Surface* screen)
	{
	if (--life < 0 || x < 0 || y < 0 || x > screen->w || y > screen->h)
		{
		delete this;
		return;
		}
		
	x += xvel; //update location
	y += ++yvel;
	SDL_Rect rect = {x, y, life % 4 + 1, (life+2) % 4 + 1};
	
	color.r = color.r * 15 / 16;
	color.g = color.g * 15 / 16;
	color.b = color.b * 15 / 16;
	SDL_FillRect(screen, &rect, SDLtoU32(color) ); //draw
	}

BouncyParticle::BouncyParticle(int X, int Y, unsigned char Color) : Particle(X, Y, Color)
	{
	life = 100;
	color = COLORS[Color];
	double angle = (double)rand() / RAND_MAX * 2 * M_PI;
	xvel = (rand() % 15) * cos(angle);
	yvel = (rand() % 15) * sin(angle);
	}

BouncyParticle::~BouncyParticle()
	{
	}

void BouncyParticle::createBouncyParticle(int x, int y, unsigned char color)
	{
	ppool.push_back(new BouncyParticle(x, y, color) );
	}
void BouncyParticle::display(SDL_Surface* screen)
	{
	if (--life < 0 || x < 0 || y < 0 || x > screen->w || y > screen->h)
		{
		delete this;
		return;
		}
		
	x += xvel; //update location
	y += yvel;
	if (y >= FIELD_HEIGHT * BLOCK_WIDTH || y <= 0)
		yvel = -yvel;
	if (x >= FIELD_WIDTH * BLOCK_WIDTH || x <= 0)
		xvel = -xvel;

	SDL_Rect rect = {x, y, 1, 1};
	color.r = color.r * 15 / 16;
	color.g = color.g * 15 / 16;
	color.b = color.b * 15 / 16;
	
	SDL_FillRect(screen, &rect, SDL_MapRGBA(screen->format, color.r, color.g, color.b, life * 2)); //draw
	}