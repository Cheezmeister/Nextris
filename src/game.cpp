#include "game.h"
#include "audio.h"

#ifdef EMSCRIPTEN
#  include <emscripten.h>
#endif

using namespace std;

int update();//SDL_Surface* screen);
int setup();//SDL_Surface*& outScreen);
int cleanup();

void js_update()
	{
	update();
	}

static SDL_Surface* screen = NULL;

int nextris_run()
	{
	while(1)
		{
		Game game;
		switch (game.run() )
			{
			case NORMAL_EXIT:
				return 0;
			case ERROR_EXIT:
				return 1;
			case RESTART:
				;//reloop
			}
		}

	}

int Game::run()
	{
        cerr << "Running...\n";
        int result = setup();
        if (result != NO_EXIT) return result;

#ifdef EMSCRIPTEN
        emscripten_set_main_loop(js_update, FPS, false);
        return NORMAL_EXIT;
#endif

	//MAIN LOOP
	Uint32 nextFrame = SDL_GetTicks() + 1000 / FPS;

	cerr << "Entering main loop.\n";
	for (;;)
		{
		if ((result = update()) != NO_EXIT)
			break;

		//regulate framerate
		int pauseSecs = nextFrame - SDL_GetTicks();
		if (pauseSecs > 0)
			{
			cdebug << "Waiting " << pauseSecs << " ms...\n";
			SDL_Delay(pauseSecs);
			}
		nextFrame += 1000 / FPS;

		} // end main loop

        return result | cleanup();
	}

int setup()
	{
	srand(time(NULL) );

	nextris::audio::init();

	//init video
	if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
		{
		cout << "Unable to init SDL: " << SDL_GetError() << "\n";
		return ERROR_EXIT;
		}
	cout << "SDL initialized fine.\n";

	// make sure SDL cleans up before exit
	atexit(SDL_Quit);

	// create a new window
	screen = SDL_SetVideoMode(
		FIELD_WIDTH * BLOCK_WIDTH + SCORE_WIDTH * BLOCK_WIDTH,
		FIELD_HEIGHT * BLOCK_WIDTH,
		16,
		SDL_HWSURFACE|SDL_DOUBLEBUF);

	if ( !screen )
		{
		cout << "Unable to set video: " << SDL_GetError() << "\n";
		return ERROR_EXIT;
		}
        return NO_EXIT;
        }
int cleanup()
	{
	cout << "Exited cleanly\n";
	return NORMAL_EXIT;
	}
int update()
	{
	static Uint32 nextFrame = 0;
	SDL_Event event;
	Uint8* ks;
	ks = SDL_GetKeyboardState(NULL);
	playField().handleInput(ks);
	while (SDL_PollEvent(&event))
		{
		playField().handleEvent(&event);
		switch (event.type)
			{
			case SDL_QUIT:
				return NORMAL_EXIT;
				break;

			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE)
					return NORMAL_EXIT;
				if (event.key.keysym.sym == CONFIG_KEYS)
					reconfig_controls();
				break;
			} // end switch
		} // end of message processing


	playField().step();
	playField().clear();
	cdebug << "Checking gameover...\n";
	if (playField().gameOver() )
		{
		playField(true);
		playerScore(true);
		return RESTART;
		}

	draw(screen);

	nextris::audio::update_bassline(playerScore().getTotal());


        return NO_EXIT;
	}
