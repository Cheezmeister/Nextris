#include "game.h"
#include "audio.h"
#include "options.h"

#ifdef EMSCRIPTEN
#  include <emscripten.h>
#endif

using namespace std;

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

    //MAIN LOOP
    Uint32 nextFrame = SDL_GetTicks() + 1000 / FPS;

    cerr << "Entering main loop.\n";
    for (;;)
    {
        result = update();
        if (result != NO_EXIT)
            break;

        draw(screen);

        /* nextris::audio::update_bassline(playerScore().getTotal()); */


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

    nextris::options::init();
    nextris::audio::init(nextris::options::get_options().game.width);

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
    int score_width = abs((FIELD_HEIGHT) * 4 / 3 - (FIELD_WIDTH));
    int width = FIELD_WIDTH * BLOCK_WIDTH + score_width * BLOCK_WIDTH;
    int height = FIELD_HEIGHT * BLOCK_WIDTH;

    screen = SDL_SetVideoMode(
                 width,
                 height,
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

    return NO_EXIT;
}
