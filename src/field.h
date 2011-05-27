#ifndef FIELD_H
#define FIELD_H

#include <map>
#include "SDL.h"
#include "debug.h"
#include "controls.h"
#include "block.h"
#include "score.h"
#include "utils.h"

#define FIELD_HEIGHT 20
#define FIELD_WIDTH 10
#define DEFAULT_SKILL LEVEL_MEDIUM
#define DEFAULT_INTERVAL 20
#define SPEEDUP_DELAY 500

class Field
	{
	private:
		unsigned long frame;
		unsigned int stepInterval;
		unsigned char skillLevel;
		bool paused;     //did the player pause?
		bool harddrop;   //is the player forcing blocks to fall?
		bool pushBlocks; //do we need to deploy the next tetra?
		bool fail;       //did the player lose?
		bool clearing;   //are we in the process of clearing blocks?
		bool cascading;  //are blocks cascading?
		bool chunking;   //are blocks falling in chunks (regions)?
		int chain;       //are we in the middle of a chain?
		//SDL_Surface* screen;
		Quad* activeQuad, *nextQuad;
		Block*** grid;
		std::vector<Region> falling;

		Field();
		void identifyRegion(std::vector<Coord>& region, Coord orig, unsigned char color = 255);
		void rowclear();
		void colorclear();
		
	public:
		~Field();
		friend Field& playField(bool reset);
		void handleEvent(SDL_Event* evt);
		void handleInput(Uint8* keystate);
		void step();
		void displayNextQuad(SDL_Surface* nextDisp);
		void display(SDL_Surface* screen);
		void getFPS();
		Quad* createQuad();
		void clear();
		int chunkFall();
		int cascade(int from);
		bool rowIsEmpty(int row);
		bool gameOver();
		void speedUp();
		bool chaining();
	};

Field& playField(bool reset = false);

#endif
