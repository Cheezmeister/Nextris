
namespace nextris
{
    namespace audio
    {
        void init();
        float noise();
        void play_sound(int color, int column, int row, int what);
        void update_bassline(unsigned long score);
    }
}
