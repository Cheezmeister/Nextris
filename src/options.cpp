#include "options.h"

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
    return string(getenv("HOME")) + '/' + basename;
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
    _options.game.colorclear=true;
    _options.game.colorthreshold= 3;
    _options.game.lookahead= 2;

    _options.graphics.windowed = true;
    _options.graphics.maxParticles = 100;
    _options.graphics.mult = 3;
}

void doline(const char* line) 
{
    string key = line;
    char* eq = strchr(line, '=');
    key = key.substr(0, eq - line);
    cout << " key = " << key;
    string value = (eq + 1);
    cout << " value = " << value;
}

bool init()
{

    defaults();

    string filename = find_file(".nextris.conf");
    ifstream infile(filename);

    if (!infile) return false;

    cout << "Reading options from " << filename << endl;

    while (infile)
    {
        string key, eq, value;

        infile >> key;
        infile >> eq;
        infile >> value;

        #define READ_OPTION(name, expr) if (key == #name) _options.name = (expr)
        READ_OPTION(keys.left, value[0]);
        READ_OPTION(keys.right, value[0]);
        READ_OPTION(keys.up, value[0]);
        READ_OPTION(keys.down, value[0]);
        READ_OPTION(keys.rotleft, value[0]);
        READ_OPTION(keys.rotright, value[0]);
        READ_OPTION(keys.pause, value[0]);

        READ_OPTION(game.speed, stoi(value));
        READ_OPTION(game.colorthreshold, stoi(value));
        READ_OPTION(game.lookahead, stoi(value));
        READ_OPTION(game.instadrop, "true" == value);
        READ_OPTION(game.lineclear, "true" == value);
        READ_OPTION(game.colorclear,"true" == value);
        #undef READ_OPTION

        std::cerr << key << "=" << value << endl;
    }

    infile.close();

    return true;
}
}
}
