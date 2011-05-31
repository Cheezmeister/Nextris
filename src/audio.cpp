#include "audio.h"

#ifdef HAVE_PORTAUDIO
#include <cmath>
#include "SDL.h"
#include "field.h"
#include "portaudio.h"

using namespace std;

static const float CHORDS[][6] = {
	{5.0f / 6.0f, 1.0, 1.25, 5.0f / 3.0f, 2.0, 2.5},         // i
	{5.0f / 6.0f, 2.0f / 3.0f, 1.0, 4.0f / 3.0f, 5.0f / 3.0f, 2.0}, // VI
	{0.75, 1.0, 1.25, 1.5, 2.0, 2.5},                        // III
	{1.5, 1.875, 2.25, 3.0, 3.75, 4.5}                      // VII
};

static const int CHANNELS = FIELD_WIDTH + 1;
const float TONIC_FREQUENCY = 440.0F;
static const int BPM = 160;

typedef struct
{
    float left_phase;
    float right_phase;
    float left_freq;
    float right_freq;
    float left_amp;
    float right_amp;
}   
ToneInfo;

//housekeeping
static PaError paerr = paNoError;
static PaStream* stream = NULL;
static bool inited = false;

static ToneInfo toneInfo[CHANNELS]; //wave info
static int chordi = 0; //chord index
static Uint16 rhythm = 0x1111; //bitstring for bass rhythm


static int patestCallback( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData )
{
    /* Cast data passed through stream to our structure. */
    ToneInfo *toneArr = (ToneInfo*)userData; 
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
            ToneInfo* data = toneArr + j;
            if (true)
            {

                out[0] += data->left_amp * sin(data->left_phase) / (float)CHANNELS;
                out[1] += data->right_amp * cos(data->right_phase) / (float)CHANNELS;
    
                // update phase
                data->left_phase += M_PI * 2 * data->left_freq / 44100;
                data->right_phase += M_PI * 2 * data->right_freq / 44100;
    
                if( data->left_phase >= M_PI * 2) data->left_phase -= M_PI * 2;
                if( data->right_phase >= M_PI * 2) data->right_phase -= M_PI * 2;

                data->left_amp -= .9 /* / secs */ / 44100;
                if (data->left_amp < 0)
                  data->left_amp = 0;
           
                data->right_amp -= .9 /* / secs */ / 44100;
                if (data->right_amp < 0)
                  data->right_amp = 0;
            }
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
	PaError paerr = Pa_Initialize();
	if (paerr != paNoError)
		cerr << "PortAudio error: %s\n" << Pa_GetErrorText( paerr );
	else
		{
		// make sure PA cleans up before exit
		atexit(cleanup);

		paerr = Pa_OpenDefaultStream(
			&stream,
			0,
			2,
			paFloat32,
			44100,
			256,
			patestCallback,
			toneInfo
			);

		paerr = Pa_StartStream( stream );
		if( paerr != paNoError )
			cerr << "PortAudio error: %s\n" << Pa_GetErrorText( paerr );
		}
	inited = true;
	}

void update_bassline(unsigned long score)
	{
	if (!inited || paerr != paNoError)
		return;

	Uint32 ticks = SDL_GetTicks();

	const Uint16 barspermin = BPM / 4; 
	const Uint16 ticksperbar = 60 * 1000 / barspermin;

	Uint16 beat = (ticks / (ticksperbar / 16)) % 16;;
	if (rhythm & (1   << beat))
		{
		toneInfo[CHANNELS - 1].left_amp = 0.5;
		}
	if (rhythm & (256 << beat))
		{
		toneInfo[CHANNELS - 1].right_amp = 0.5;
		}

	int temp = (ticks / ticksperbar) % 4;
	if (chordi == temp) return;
	chordi = temp;
	
	toneInfo[CHANNELS - 1].left_freq = TONIC_FREQUENCY * CHORDS[chordi][rand() % 6] / 4;
	toneInfo[CHANNELS - 1].right_freq = TONIC_FREQUENCY * CHORDS[chordi][rand() % 6] / 4;
	

	if (chordi == 0)
		{
		unsigned long temp = score;
		rhythm = temp ^ (temp << 7) ^ (temp << 14) ^ (temp << 24);
		}

	}


void play_sound(int color, int column, int row, int what) 
	{ 
	if (!inited)
		init();
	if (paerr != paNoError)
		return;


//        // play different pitches depending on color, forming a dim7 chord
//	toneInfo.left_freq = TONIC_FREQUENCY * pow(6.0 / 5.0, color);
//	toneInfo.right_freq = TONIC_FREQUENCY * pow(6.0 / 5.0, color);

	int i = chordi;
	
	cout << "Chord " << i << "\n";

	toneInfo[column].left_freq = TONIC_FREQUENCY * CHORDS[i][color];
	toneInfo[column].right_freq = TONIC_FREQUENCY * CHORDS[i][color];

	toneInfo[column].right_amp = (float)column / (float)FIELD_WIDTH;
	toneInfo[column].left_amp = 1.0 - toneInfo[column].left_amp;

	}
#else
void play_sound(int color, int column, int row, int what) { }
void update_bassline(unsigned long score) { }
#endif
