#ifndef OPTIONS_H
#define OPTIONS_H

namespace nextris
{
namespace options
{
/**
 *
 */
struct Options
{
    struct KeysOpts
    {
        char left;
        char right;
        char up;
        char down;
        char rotleft;
        char rotright;
        char pause;
        // TODO quit
    } keys;

    struct GameOpts
    {
        short width; // blocks
        short height; // blocks
        short speed;  // hertz (TODO)
        short colorthreshold;
        short lookahead;
        bool instadrop; // press up to drop fully
        bool lineclear;
        bool colorclear;
    } game;

    struct GraphicsOpts
    {
        bool windowed;
        int maxParticles;
        short mult;
    } graphics;
};

bool init();
Options& get_options();

}
}
#endif
