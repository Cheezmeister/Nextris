#include "options.h"

#include <fstream>

namespace nextris
{
namespace options
{
Options _options;

Options get_options()
{
    return _options;
}

const char* find_file(const char* basename)
{
    return basename;
}

void defaults()
{
    _options.keys.left            = 'a';
    _options.keys.right           = 'd';
    _options.keys.up              = 'w';
    _options.keys.down            = 's';
    _options.keys.rotleft         = 'l';
    _options.keys.rotright        = 'p';
    _options.keys.pause           = ' '; // Set the pause button | default spacebar
    /* _options.keys.quit            = ''; */

    _options.game.width=10;
    _options.game.height=20;
    _options.game.speed=20;
    _options.game.instadrop=true;
    _options.game.lineclear=true;
    _options.game.colorclear=false;
    _options.game.colorthreshold= 4;
    _options.game.lookahead= 1;

    _options.graphics.windowed = true;
    _options.graphics.maxParticles = 100;
    _options.graphics.mult = 3;
}

bool init()
{

    defaults();

    const char* filename = find_file(".nextris.conf");
    std::ifstream infile(filename);

    if (!infile) return false;

    char line [100];
    while (infile)
    {
        infile.getline(line, 100);
    }

    infile.close();

    return true;
}
}
}
