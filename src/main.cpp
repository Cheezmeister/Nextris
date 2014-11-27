#include "game.h"

#include "audio.h"


int parse_args(int argc, char** argv)
{
    for (int i = 0; i < argc; ++i)
    {
        if (!strcmp(argv[i], "-s") || !strcmp(argv[i], "--nosound"))
            nextris::audio::enable(false);
    }

    return 0;
}
int main ( int argc, char** argv )
{
    int ret = parse_args(argc, argv);
    if (ret) return ret;
    return nextris_run();
}

