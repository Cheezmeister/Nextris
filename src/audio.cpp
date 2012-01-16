#include "audio.h"

#ifdef HAVE_PORTAUDIO
#include <cmath>
#include "SDL.h"
#include "field.h"
#include "portaudio.h"
#include "block.h"

namespace nextris
{
    namespace audio 
    {
        using namespace std;

        typedef const float Chord[6];

#define CHORD_ARR_i   {5.0 / 6.0, 1.0, 1.25, 5.0 / 3.0, 2.0, 2.5}
#define CHORD_ARR_VI  {5.0 / 6.0, 2.0 / 3.0, 1.0, 4.0 / 3.0, 5.0 / 3.0, 2.0}
#define CHORD_ARR_III {0.75, 1.0, 1.25, 1.5, 2.0, 2.5}
#define CHORD_ARR_VII {1.125, 1.5, 1.875, 2.25, 3.0, 3.75}

        static Chord invRock[] = { CHORD_ARR_i, CHORD_ARR_VI, CHORD_ARR_III, CHORD_ARR_VII };
        static Chord epic3[] = { CHORD_ARR_VI, CHORD_ARR_VII, CHORD_ARR_i, CHORD_ARR_i };

#undef CHORD_ARR_i 
#undef CHORD_ARR_VI 
#undef CHORD_ARR_III
#undef CHORD_ARR_VII

        static const float (*chordProg)[6] = epic3;

        enum ChannelEnum {
            CHAN_FIRSTCOLUMN,
            CHAN_LASTCOLUMN = CHAN_FIRSTCOLUMN + FIELD_WIDTH - 1,
            CHAN_BASS,
            CHAN_PERC,
            CHANNELS,
        };

//        static const int CHANNELS = FIELD_WIDTH + 1;
        const float TONIC_FREQUENCY = 440.0F;
        static const int BPM = 160;


        const int SAMPLE_RATE = 44100;
        typedef enum _WaveType {
            WT_SIN,
            WT_SQUARE,
            WT_TRIANGLE,
            WT_SAWTOOTH,
            WT_NOISE,
        } WaveType;
        typedef enum _Decay {
            DECAY_NONE,
            DECAY_LINEAR,
            //DECAY_EXP,
            //DECAY_REVERSE_LINEAR,
            //DECAY_REVERSE_EXP,
        } Decay;

        typedef struct _ToneInfo
        {
            WaveType type;
            Decay decay;
            float time;
            float freq;
            float amp;
            float duration;
            float phase;
        } ToneInfo;

        typedef struct _StereoToneInfo
        {
            ToneInfo left;
            ToneInfo right;
        } StereoToneInfo;

        //housekeeping
        static PaError paerr = paNoError;
        static PaStream* stream = NULL;
        static bool inited = false;

        static StereoToneInfo toneInfo[CHANNELS]; //wave info
        static int chordi = 0; //chord index
        static Uint16 rhythm = 0x0000; //bitstring for bass rhythm

        //clamp a value between two extremes
        template<typename T>
        inline T clamp(T value, T min, T max)
        {
            return value < min ? min : value > max ? max : value;
        }

        float wave_value(const ToneInfo& tone)
        {
            if (tone.type == WT_SIN)
                return tone.amp * sin(tone.phase) / (float)CHANNELS;
            if (tone.type == WT_SAWTOOTH)
                return tone.amp * ((tone.phase-M_PI) / M_PI - 1) / 2.0f;
            if (tone.type == WT_TRIANGLE)
                return tone.amp * (4 * tone.phase / (M_PI*2))
                            - 4 * (clamp<float>(tone.phase, M_PI/2.0f, 3.0f*M_PI/2.0f) - M_PI/2.0f);
            if (tone.type == WT_SQUARE)
                return tone.amp * (tone.phase < M_PI ? 1 : -1)  / 2.0f;
            return 0;
            if (tone.type == WT_NOISE)
                return tone.amp * noise(0.0, 1.0);
        }

        static int nextris_pa_callback( const void *inputBuffer, void *outputBuffer,
                                   unsigned long framesPerBuffer,
                                   const PaStreamCallbackTimeInfo* timeInfo,
                                   PaStreamCallbackFlags statusFlags,
                                   void *userData )
        {
            /* Cast data passed through stream to our structure. */
            StereoToneInfo *toneArr = (StereoToneInfo*)userData; 
            float *out = (float*)outputBuffer;
            unsigned int i;
            (void) inputBuffer; /* Prevent unused variable warning. */

            if (paerr != paNoError)
            {
                cerr << "PortAudio error: %s\n" << Pa_GetErrorText( paerr );
                return 0;
            }


    
            for( i=0; i<framesPerBuffer; i++ )
            {
                out[0] = out[1] = 0.0;
                for (int j = 0; j < CHANNELS; ++j)
                {
                    StereoToneInfo* data = toneArr + j;
                    if (data->left.amp + data->right.amp == 0)
                        continue;

                    out[0] += wave_value(data->left); 
                    out[1] += wave_value(data->right);
            
                    data->left.time += 1.0 / SAMPLE_RATE;
                    data->right.time += 1.0 / SAMPLE_RATE;


                    // update amplitude
                    if (data->left.decay == DECAY_NONE)
                    {
                        if (data->left.time > data->left.duration)
                        {
                            data->left.amp = 0;
                            data->left.time = 0;
                        }
                    }
                    else if (data->left.decay == DECAY_LINEAR)
                    {
                        data->left.amp -= .9 / SAMPLE_RATE;
                        if (data->left.amp < 0)
                        {
                            data->left.amp = 0;
                            data->left.time = 0;
                        }
                    }
                    if (data->right.decay == DECAY_NONE)
                    {
                        if (data->right.time > data->right.duration)
                        {
                            data->right.amp = 0;
                            data->right.time = 0;
                        }
                    }
                    else if (data->right.decay == DECAY_LINEAR)
                    {
                        data->right.amp -= .9 / SAMPLE_RATE;
                        if (data->right.amp < 0)
                        {
                            data->right.amp = 0;
                            data->right.time = 0;
                        }
                    }
            
                    // update phase
                    data->left.phase += M_PI * 2 * data->left.freq / SAMPLE_RATE;
                    data->right.phase += M_PI * 2 * data->right.freq / SAMPLE_RATE;
    
                    if( data->left.phase >= M_PI * 2) data->left.phase -= M_PI * 2;
                    if( data->right.phase >= M_PI * 2) data->right.phase -= M_PI * 2;

            
                }
            ++++out;

            }
            return 0;
        }




        void cleanup()
        {
            paerr = Pa_StopStream( stream );
            stream = NULL;
        }
        void init()
        {
            if (inited)
                return;

            PaError paerr = Pa_Initialize();
            if (paerr != paNoError)
            {
                cerr << "Couldn't init! PortAudio error: " << Pa_GetErrorText( paerr );
                return;
            }
    
            // make sure PA cleans up before exit
            atexit(cleanup);

            paerr = Pa_OpenDefaultStream(
                &stream,
                0,
                2,
                paFloat32,
                SAMPLE_RATE,
                256,
                nextris_pa_callback,
                toneInfo
                );
            if( paerr != paNoError )
            {
                cerr << "Couldn't open audio! PortAudio error: " << Pa_GetErrorText( paerr );
                return;
            }

            paerr = Pa_StartStream( stream );
            if( paerr != paNoError )
            {
                cerr << "Couldn't start audio! PortAudio error:  " << Pa_GetErrorText( paerr );
                return;
            }
            inited = true;
        }

        //stolen from http://www.dspguru.com/dsp/howtos/how-to-generate-white-gaussian-noise
        float noise(float mean, float variance)
        {
            float U1, U2, V1, V2, S, X, Y;
            do
            {
                U1 = fabs((float)rand() / (float)0x7fff);            /* U1=[0,1] */
                U2 = fabs((float)rand() / (float)0x7fff);            /* U2=[0,1] */
                V1 = 2 * U1 - 1;           /* V1=[-1,1] */
                V2 = 2 * U2 - 1;           /* V2=[-1,1] */
                 S = V1 * V1 + V2 * V2;
            } while (S >= 1);
    
           X = sqrt(-2 * log(S) / S) * V1;
           //Y = sqrt(-2 * log(S) / S) * V2;

           return mean + sqrt(variance) * X;
        }











        void update_percs(unsigned long score)
        {
            toneInfo[CHAN_PERC].left.freq = noise(0, 1);
            toneInfo[CHAN_PERC].right.freq = noise(0, 1);
            toneInfo[CHAN_PERC].left.amp = 0.1;
            toneInfo[CHAN_PERC].right.amp = 0.1;
        }

        void update_bassline(unsigned long score)
        {
            if (!inited || paerr != paNoError)
                return;

            //update_percs(score);
            Uint32 ticks = SDL_GetTicks();

            chordProg = epic3;


            const Uint16 barspermin = BPM / 4; 
            const Uint16 ticksperbar = 60 * 1000 / barspermin;
            const Uint16 ticksperbeat = ticksperbar / 16;

            Uint16 beat = (ticks / ticksperbeat) % 16;;
            if (rhythm & (1 << beat) )
            {
                toneInfo[CHAN_BASS].right.amp = 0.1;
            }
            else
            {
                toneInfo[CHAN_BASS].right.amp = 0;
            }
            if (beat == 0)
            {
                toneInfo[CHAN_BASS].left.amp = 0.8;
            }

            int temp = (ticks / ticksperbar) % 4;
            if (chordi == temp) return;
            chordi = temp;
    
            toneInfo[CHAN_BASS].left.freq = TONIC_FREQUENCY * chordProg[chordi][rand() % 6] / 4;
            toneInfo[CHAN_BASS].right.freq = TONIC_FREQUENCY * chordProg[chordi][rand() % 6] / 4;

            toneInfo[CHAN_BASS].left.type = WT_SIN;
            toneInfo[CHAN_BASS].right.type = WT_SQUARE;

            toneInfo[CHAN_BASS].left.duration = ticksperbar;
            toneInfo[CHAN_BASS].right.duration = ticksperbeat;
            toneInfo[CHAN_BASS].left.decay = DECAY_LINEAR;
            toneInfo[CHAN_BASS].right.decay = DECAY_LINEAR;


            if (chordi == 0)
                {
                unsigned long temp = score;
                rhythm = temp ^ (temp << 7) ^ (temp << 14) ^ (temp << 24);
                rhythm |= 0x5555; //always play a note on the first of every four beats
                }

        }


        void play_sound(int color, int column, int row, int what) 
        { 
            if (!inited)
                init();
            if (paerr != paNoError)
                return;


            int i = chordi;

            if (what == SND_SHIFTLEFT || what == SND_SHIFTRIGHT)
            {
                toneInfo[column].left.type = WT_SQUARE;
                toneInfo[column].right.type = WT_SQUARE;

                toneInfo[column].left.freq = TONIC_FREQUENCY * chordProg[i][0] / 2;
                toneInfo[column].right.freq = TONIC_FREQUENCY * chordProg[i][5] / 2;
                
                toneInfo[column].left.duration = 0.05;
                toneInfo[column].right.duration = 0.05;
                toneInfo[column].right.decay = DECAY_NONE;
                toneInfo[column].left.decay = DECAY_NONE;
                toneInfo[column].right.amp = (float)column / (float)FIELD_WIDTH / 10.0;
                toneInfo[column].left.amp = 0.1 - toneInfo[column].left.amp;
            }
            else if (what == SND_ASPLOADED)
            {
                toneInfo[column].left.type = WT_SIN;
                toneInfo[column].right.type = WT_SIN;

                toneInfo[column].left.freq = TONIC_FREQUENCY * chordProg[i][color];
                toneInfo[column].right.freq = TONIC_FREQUENCY * chordProg[i][color];
                toneInfo[column].left.duration = 1000;
                toneInfo[column].right.duration = 1000;
                toneInfo[column].right.decay = DECAY_LINEAR;
                toneInfo[column].left.decay = DECAY_LINEAR;
                toneInfo[column].right.amp = (float)column / (float)FIELD_WIDTH;
                toneInfo[column].left.amp = 1.0 - toneInfo[column].left.amp;
            }
            else
                return;

        }
    }
}
#else
namespace nextris
{
    namespace audio
    {
        void init() { }
        void play_sound(int color, int column, int row, int what) { }
        void update_bassline(unsigned long score) { }
    }
}
#endif
