namespace nextris
{
    namespace audio
    {
        void init();
        float noise(float mean = 0.0, float variance = 1.0);
        void play_sound(int color, int column, int row, int what);
        void update_bassline(unsigned long score);
    }
}
