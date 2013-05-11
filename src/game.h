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

extern int update();
extern int setup();
extern int cleanup();

extern void js_update();
extern void js_setup();
extern void js_cleanup();

#endif
