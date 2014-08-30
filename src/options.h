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
        /* char quit; */
    } keys;

    struct GameOpts
    {
        short width; // blocks
        short height; // blocks
        short speed;  // hertz
        bool instadrop;
        bool lineclear;
        bool colorclear;
        short colorthreshold;
        short lookahead;
    } game;

    struct GraphicsOpts
    {
        bool windowed;
        int maxParticles;
        short mult;
    } graphics;
};

bool init();

Options get_options();
}
}
#endif
