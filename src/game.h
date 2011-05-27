#ifndef GAME_H
#define GAME_H

#include <cstdlib>
#include <ctime>
#include <SDL/SDL.h>
#include "debug.h"
#include "block.h"
#include "field.h"
#include "score.h"
#include "display.h"

enum {
	NORMAL_EXIT,
	ERROR_EXIT,
	RESTART
};


class Game
	{
	public:
		int run();
	};


#endif
