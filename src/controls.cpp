#include "controls.h"
#include <iostream>
#include <fstream>
#include <string>
#include <map>

std::map<std::string, SDLKey> keys;

void defaults() 
	{
	//fallback defaults
	keys["UP"] = SDLK_w;
	keys["DOWN"] = SDLK_s;
	keys["LEFT"] = SDLK_a;
	keys["RIGHT"] = SDLK_d;
	keys["A"] = SDLK_l;
	keys["B"] = SDLK_p;
	keys["PAUSE"] = SDLK_SPACE;
	}

void read_controls() { defaults(); }
void write_controls() { }

std::string names[] = {
	"LEFT",
	"RIGHT",
	"UP",
	"DOWN",
	"A",
	"B",
	"PAUSE"
};

SDLKey get_userkey(UserKey key)
	{
	static bool initialized = false;
	
	if (!initialized)
		{
		read_controls();
		initialized = true;
		}
		
	if (key < N_USERKEYS)
		return keys[names[key]];
	return SDLK_UNKNOWN;	
	}

void reconfig_controls()
	{
	bool waiting;
	SDL_Event event;
	
	for (int i = 0; i < N_USERKEYS; ++i)
		{
		std::cerr << "Enter key for " << names[i] << ": \n";
		waiting = true;
		while (waiting)
			{
			while (SDL_PollEvent(&event) )
				{
				if (event.type == SDL_KEYDOWN)
					{
					keys[names[i]] = event.key.keysym.sym;
					waiting = false;
					}
				if (event.type == SDL_QUIT)
					{
					std::cout << "Never mind!";
					return;
					}
				}
			}
		}
	write_controls();
	}
	
//DevTeam
//letmein35
