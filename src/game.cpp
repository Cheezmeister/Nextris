#include "game.h"
#include "audio.h"

using namespace std;

int Game::run()
	{
	int pauseSecs;
	srand(time(NULL) );

	nextris::audio::init();

	//init video
	if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
		{
		cout << "Unable to init SDL: " << SDL_GetError() << "\n";
		return 1;
		}
	cout << "SDL initialized fine.\n";

	// make sure SDL cleans up before exit
	atexit(SDL_Quit);

	// create a new window
	SDL_Surface* screen = SDL_SetVideoMode(
		FIELD_WIDTH * BLOCK_WIDTH + SCORE_WIDTH * BLOCK_WIDTH,
		FIELD_HEIGHT * BLOCK_WIDTH,
		16,
		SDL_HWSURFACE|SDL_DOUBLEBUF);

	if ( !screen )
		{
		cout << "Unable to set video: " << SDL_GetError() << "\n";
		return 1;
		}

	//MAIN LOOP
	bool done = false;
	Uint32 nextFrame = SDL_GetTicks() + 1000 / FPS;

	cout << "Entering main loop.\n";
	while (!done)
		{
		SDL_Event event;
		Uint8* ks;
		ks = SDL_GetKeyState(NULL);
		playField().handleInput(ks);
		while (SDL_PollEvent(&event))
			{
			playField().handleEvent(&event);
			switch (event.type)
				{
				case SDL_QUIT:
					done = true;
					break;

				case SDL_KEYDOWN:
					if (event.key.keysym.sym == SDLK_ESCAPE)
						done = true;
					else if (event.key.keysym.sym == CONFIG_KEYS)
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

		//regulate framerate
		pauseSecs = nextFrame - SDL_GetTicks();
		if (pauseSecs > 0)
			{
			cdebug << "Waiting " << pauseSecs << " ms...\n";
			SDL_Delay(pauseSecs);
			}
		nextFrame = SDL_GetTicks() + 1000 / FPS;
		} // end main loop

	cout << "Exited cleanly\n";
	return NORMAL_EXIT;
	}
