#include "game.h"

#include "audio.h"
int main ( int argc, char** argv )
{
    /*for (int i = 0; i < 100; ++i)
    {
        std::cout << nextris::audio::noise() << "\n";
    }
    return 0;*/

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
