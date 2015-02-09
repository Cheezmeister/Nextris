enum {
    SND_ASPLOADED,
    SND_ROTATELEFT,
    SND_ROTATERIGHT,
    SND_SHIFTLEFT,
    SND_SHIFTRIGHT
};

namespace nextris
{
namespace audio
{
void init(int channels);
void enable(bool on = true);
float noise(float mean = 0.0, float variance = 1.0);
void play_sound(int color, int column, int row, int what);
void update_bassline(unsigned long score);
}
}
