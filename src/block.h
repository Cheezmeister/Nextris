#ifndef BLOCK_H
#define BLOCK_H

#include <iostream>
#include <vector>
#include <cstdlib>
#include <SDL.h>
#include "utils.h"

#define BLOCK_WIDTH 32

enum {
	SHAPE_I, 
	SHAPE_L, 
	SHAPE_J, 
	SHAPE_T, 
	SHAPE_O, 
	SHAPE_Z, 
	SHAPE_S,
	N_SHAPES
};

enum {
	LEVEL_EASY = 4,
	LEVEL_MEDIUM,
	LEVEL_HARD
};

enum {
	FELL_OKAY,
	FELL_AND_SET,
	NO_ROOM
};

typedef struct _Coord
	{
	int x;
	int y;
	bool operator==(const _Coord& rhs)
		{ return rhs.x == x && rhs.y == y; }
	} Coord;

const SDL_Color COLORS[] =	{
							{255, 0, 0}, //red
							{0, 255, 0}, //green
							{0, 0, 255}, //blue
							{255, 255, 0}, //yellow
							{0, 255, 255}, //cyan
							{255, 0, 255} //magenta
							};
class Block
    {
	private:
		SDL_Rect mask; //the rectangle the block occupies
		bool active; //is it still falling?
		bool condemned; //is it on the way out?
		unsigned char color; //red, blue, yellow, green, [cyan, magenta]

	public:
		//default ctor, should not be used
		Block();
		//ctor
		Block(int X, int Y, unsigned char color);
		//dtor
		~Block();
		//is the block in an invalid position? (outside grid, on another block)
		int blocked(const Block*** grid);
		//fall one notch and return whether we're blocked
		int fall(Block*** grid);
		//explode with pretty particles, deleting self in the process
		void aspload();
		//update grid with current location
		void updateGrid(Block*** grid);
		//return to the position above
		void recede();
		//draw the block on an SDL_Surface
		void display(SDL_Surface* screen);
		//return color
		unsigned char getColor();
		//signify the block is no longer moving
		void deactivate();
		//return the x location, independent of width in pixels
		int getX();
		//return the y location, independent of width in pixels
		int getY();
		//set x, independent of width
		void setX(int x);
		//set y, independent of width
		void setY(int y);
		//rotate around a given center, clockwise if passed a positive int, counter otherwise
		void rotateAround(Coord center, int clockwise);
		//return whether the block is at this coordinate
		int isAt(Coord loc);
		//signify that this Block is about to be cleared
		void condemn();
    };

class Quad
	{
	private:
		const unsigned char shape;
		Block* master;
		Block* slave1;
		Block* slave2;
		Block* slave3;
		Block*** grid;
	public:
		Quad(int X, int Y, Block*** Grid, unsigned char level);
		~Quad();
		int fall();
		void rotateLeft();
		void rotateRight();
		void moveLeft();
		void moveRight();
		void goTo(int X, int Y);
		void display(SDL_Surface* screen, bool dropshadow = false);
	};


class Region
	{
	private:
		std::vector<Block*> blocks;
		Block*** grid;

	public:
	    //ctor
		Region(Block*** Grid);
		//dtor
		~Region();
		//return number of blocks encompassed
		unsigned int size();
		//reset to an empty region
		void reset();
		//remove references to this region's blocks from the grid
		void yank();
		//clear all blocks in this region
		void clear();
		//add a block to the region
		void addBlock(Block* block);
		//fall one block
		int fall();
		//check that all pointers are valid
		int valid();
		//draw all blocks on an SDL_Surface
		void display(SDL_Surface* screen);
		//return blocks contained in this Region
		const std::vector<Block*>& getBlocks();
		
		//recursively identify a contiguous region of the same color, or any color
		//contiguous here means all blocks are connected by an edge
		static void identifyRegion(Region& region, Coord orig, unsigned char color);
	};

#endif
