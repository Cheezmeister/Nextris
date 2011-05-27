#include "game.h"

int main ( int argc, char** argv )
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
