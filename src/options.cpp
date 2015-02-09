#include "options.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

namespace nextris
{
namespace options
{

Options _options;

Options& get_options()
{
    return _options;
}

string find_file(const char* basename)
{
    // TODO Windows-ify
    #ifdef __WIN32__
    const char* home = getenv("HOME");
    if (home) return string(home) + '/' + basename;
    return string(basename);
    #else
    return string(getenv("HOME")) + '/' + basename;
    #endif
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

    string filename = find_file(".nextris.conf");
    ifstream infile(filename.c_str());

    if (!infile) return false;

    cout << "Reading options from " << filename << endl;

    while (infile)
    {
        string key, eq, value;

        infile >> key;
        infile >> eq;
        infile >> value;

        char c = value[0];
        char* temp;
        long i = strtol(value.c_str(), &temp, 10);

        #define READ_OPTION(name, expr) if (key == #name) {_options.name = (expr); continue; }
        READ_OPTION(keys.left, c);
        READ_OPTION(keys.right, c);
        READ_OPTION(keys.up, c);
        READ_OPTION(keys.down, c);
        READ_OPTION(keys.rotleft, c);
        READ_OPTION(keys.rotright, c);
        READ_OPTION(keys.pause, c);

        READ_OPTION(game.width, i);
        READ_OPTION(game.height, i);

        READ_OPTION(game.speed, i);
        READ_OPTION(game.colorthreshold, i);
        READ_OPTION(game.lookahead, i);
        READ_OPTION(game.instadrop, "true" == value);
        READ_OPTION(game.lineclear, "true" == value);
        READ_OPTION(game.colorclear,"true" == value);
        #undef READ_OPTION

        std::cerr << "Ignoring '" << key << "=" << value << "'" << endl;
    }

    infile.close();

    return true;
}
}
}
