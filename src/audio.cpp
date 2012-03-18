#include "audio.h"

#ifndef HAVE_PORTAUDIO
namespace nextris
{
    namespace audio
    {
        void init() { }
        void play_sound(int color, int column, int row, int what) { }
        void update_bassline(unsigned long score) { }
    }
}
#else // HAVE_PORTAUDIO

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

        static const float (*chordProg)[6] = invRock;

        enum ChannelEnum {
            CHAN_FIRSTCOLUMN,
            CHAN_LASTCOLUMN = CHAN_FIRSTCOLUMN + FIELD_WIDTH - 1,
            CHAN_PAD,
            CHAN_PERC,
            CHAN_BASS,
            CHANNELS,
        };

//        static const int CHANNELS = FIELD_WIDTH + 1;
        const float TONIC_FREQUENCY = 440.0F;
        static const int BPM = 160;
        static const Uint16 barspermin = BPM / 4; 
        static const Uint16 ticksperbar = 60 * 1000 / barspermin;
        static const Uint16 ticksperbeat = ticksperbar / 16;



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
            DECAY_PARABOLIC,
            //DECAY_EXP,
            //DECAY_REVERSE_LINEAR,
            //DECAY_REVERSE_EXP,
        } Decay;

        typedef struct _ToneInfo
        {
            // "public" specifications
            WaveType type;
            Decay decay;
            float freq;
            float amp;
            float duration;
            // "private" time-dependent fields
            float phase;
            float current_amp;
            float time;
        } ToneInfo;

        typedef union _StereoToneInfo
        {
            ToneInfo arr[2];
            struct 
            {
                ToneInfo left;
                ToneInfo right;
            } channel;
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
                return tone.current_amp * sin(tone.phase) / (float)CHANNELS;
            if (tone.type == WT_SAWTOOTH)
                return tone.current_amp * ((tone.phase-M_PI) / M_PI - 1) / 2.0f;
            if (tone.type == WT_TRIANGLE)
                return tone.current_amp * (4 * tone.phase / (M_PI*2))
                            - 4 * (clamp<float>(tone.phase, M_PI/2.0f, 3.0f*M_PI/2.0f) - M_PI/2.0f);
            if (tone.type == WT_SQUARE)
                return tone.current_amp * (tone.phase < M_PI ? 1 : -1)  / 2.0f;
            return 0;
            if (tone.type == WT_NOISE)
                return tone.current_amp * noise(0.0, 1.0);
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
                    if (data->channel.left.amp + data->channel.right.amp == 0)
                        continue;

                    out[0] += wave_value(data->channel.left); 
                    out[1] += wave_value(data->channel.right);
            
                    data->channel.left.time += 1.0 / SAMPLE_RATE;
                    data->channel.right.time += 1.0 / SAMPLE_RATE;


                    // update amplitude
                    for (int i = 0; i < 2; ++i)
                    {
                        ToneInfo& ti = data->arr[i];
                        if (ti.decay == DECAY_NONE)
                        {
                            ti.current_amp = ti.amp;
                            if (ti.time > ti.duration)
                            {
                                ti.amp = 0;
                                ti.time = 0;
                            }
                        }
                        else if (ti.decay == DECAY_LINEAR)
                        {
                            ti.current_amp = ti.amp * (1 - ti.time / ti.duration);
                            if (ti.current_amp < 0)
                            {
                                ti.amp = 0;
                                ti.time = 0;
                            }
                        }
                        else if (ti.decay == DECAY_PARABOLIC)
                        {
                            float x2 = (ti.time - ti.duration / 2 ) * (ti.time - ti.duration / 2);
                            float par = 1 - x2 * 4 / (ti.duration * ti.duration);
                            ti.current_amp = ti.amp * par;
                        }

                        if (ti.current_amp < 0)
                        {
                            ti.amp = 0;
                            ti.time = 0;
                        }
                    }
                    
            
                    // update phase
                    data->channel.left.phase += M_PI * 2 * data->channel.left.freq / SAMPLE_RATE;
                    data->channel.right.phase += M_PI * 2 * data->channel.right.freq / SAMPLE_RATE;
    
                    if( data->channel.left.phase >= M_PI * 2) data->channel.left.phase -= M_PI * 2;
                    if( data->channel.right.phase >= M_PI * 2) data->channel.right.phase -= M_PI * 2;

            
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









        void update_pads(unsigned long score)
        {
            int i = 3;
            toneInfo[CHAN_PAD].channel.left.type = WT_SIN;
            toneInfo[CHAN_PAD].channel.right.type = WT_SIN;
            toneInfo[CHAN_PAD].channel.left.freq = TONIC_FREQUENCY * chordProg[chordi][i];
            toneInfo[CHAN_PAD].channel.right.freq = TONIC_FREQUENCY * chordProg[chordi][i] * 1.01;
            toneInfo[CHAN_PAD].channel.left.amp = 0.4;
            toneInfo[CHAN_PAD].channel.right.amp = 0.4;

            toneInfo[CHAN_PAD].channel.left.duration = ticksperbar / 1000.0;
            toneInfo[CHAN_PAD].channel.right.duration = ticksperbar / 1000.0;
            toneInfo[CHAN_PAD].channel.left.decay = DECAY_PARABOLIC;
            toneInfo[CHAN_PAD].channel.right.decay = DECAY_PARABOLIC;

        }

        void update_percs(unsigned long score)
        {
            toneInfo[CHAN_PERC].channel.left.freq = noise(0, 1);
            toneInfo[CHAN_PERC].channel.right.freq = noise(0, 1);
            toneInfo[CHAN_PERC].channel.left.amp = 0.1;
            toneInfo[CHAN_PERC].channel.right.amp = 0.1;
        }

        void update_bassline(unsigned long score)
        {
            if (!inited || paerr != paNoError)
                return;

            update_pads(score);
            //update_percs(score);
            Uint32 ticks = SDL_GetTicks();

            Uint16 beat = (ticks / ticksperbeat) % 16;;
            if (rhythm & (1 << beat) )
            {
                toneInfo[CHAN_BASS].channel.right.amp = 0.1;
            }
            else
            {
                toneInfo[CHAN_BASS].channel.right.amp = 0;
            }
            if (beat == 0)
            {
                toneInfo[CHAN_BASS].channel.left.amp = 0.8;
            }

            int temp = (ticks / ticksperbar) % 4;
            if (chordi == temp) return;
            chordi = temp;
    
            toneInfo[CHAN_BASS].channel.left.freq = TONIC_FREQUENCY * chordProg[chordi][rand() % 6] / 4;
            toneInfo[CHAN_BASS].channel.right.freq = TONIC_FREQUENCY * chordProg[chordi][rand() % 6] / 4;

            toneInfo[CHAN_BASS].channel.left.type = WT_SIN;
            toneInfo[CHAN_BASS].channel.right.type = WT_SQUARE;

            toneInfo[CHAN_BASS].channel.left.duration = ticksperbar;
            toneInfo[CHAN_BASS].channel.right.duration = ticksperbeat;
            toneInfo[CHAN_BASS].channel.left.decay = DECAY_LINEAR;
            toneInfo[CHAN_BASS].channel.right.decay = DECAY_LINEAR;


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
                toneInfo[column].channel.left.type = WT_SQUARE;
                toneInfo[column].channel.right.type = WT_SQUARE;

                toneInfo[column].channel.left.freq = TONIC_FREQUENCY * chordProg[i][0] / 2;
                toneInfo[column].channel.right.freq = TONIC_FREQUENCY * chordProg[i][5] / 2;
                
                toneInfo[column].channel.left.duration = 0.05;
                toneInfo[column].channel.right.duration = 0.05;
                toneInfo[column].channel.right.decay = DECAY_NONE;
                toneInfo[column].channel.left.decay = DECAY_NONE;
                toneInfo[column].channel.right.amp = (float)column / (float)FIELD_WIDTH / 10.0;
                toneInfo[column].channel.left.amp = 0.1 - toneInfo[column].channel.right.amp;
            }
            else if (what == SND_ASPLOADED)
            {
                toneInfo[column].channel.left.type = WT_SIN;
                toneInfo[column].channel.right.type = WT_SIN;

                toneInfo[column].channel.left.freq = TONIC_FREQUENCY * chordProg[i][color];
                toneInfo[column].channel.right.freq = TONIC_FREQUENCY * chordProg[i][color];
                toneInfo[column].channel.left.duration = 1.0;
                toneInfo[column].channel.right.duration = 1.0;
                toneInfo[column].channel.right.decay = DECAY_LINEAR;
                toneInfo[column].channel.left.decay = DECAY_LINEAR;
                toneInfo[column].channel.right.amp = (float)column / (float)FIELD_WIDTH;
                toneInfo[column].channel.left.amp = 1.0 - toneInfo[column].channel.left.amp;
            }
            else
                return;

        }
    }
}
#endif
