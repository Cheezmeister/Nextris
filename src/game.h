#ifndef GAME_H
#define GAME_H

#include <cstdlib>
#include <ctime>
#include <SDL.h>
#include "debug.h"
#include "block.h"
#include "field.h"
#include "score.h"
#include "display.h"

enum {
	NORMAL_EXIT,
	ERROR_EXIT,
	NO_EXIT,
	RESTART
};


class Game
	{
	public:
		int run();
	};

int nextris_run();

int update();
int setup();
int cleanup();

void js_update() { update(); }
void js_setup() { setup(); }
void js_cleanup() { cleanup(); }

#endif
