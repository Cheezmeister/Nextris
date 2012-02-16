#include "field.h"

Field::Field()
	{
	//screen = Screen;
	stepInterval = DEFAULT_INTERVAL;
	paused = false;
	harddrop = false;
	warpdrop = false;
	pushBlocks = true;
	fail = false;
	clearing = false;
	cascading = false;
	chunking = false;
	skillLevel = DEFAULT_SKILL;
	frame = 0;

	//ablsolutely disgusting allocation.
	grid = new Block**[FIELD_WIDTH];
	for (int i = 0; i < FIELD_WIDTH; ++i)
		{
		grid[i] = new Block*[FIELD_HEIGHT];
		for (int j = 0; j < FIELD_HEIGHT; ++j)
			grid[i][j] = NULL;
		}
	nextQuad = createQuad();
	activeQuad = NULL;

	}

Field::~Field()
	{
	if (activeQuad)
		delete activeQuad;

	for (int i = 0; i < FIELD_WIDTH; ++i)
		{
		for (int j = 0; j < FIELD_HEIGHT; ++j)
			{
			if (grid[i][j])
				delete grid[i][j];
			}
		cdebug << ">>>";		
		delete[] grid[i];
		}
	delete[] grid;
	}


//it's tough to decide whether to handle left/right motion as an event or a state.
//one seems too fast, the other too slow.
void Field::handleEvent(SDL_Event* evt)
	{
	//pause game
	if (evt->type == SDL_KEYDOWN && evt->key.keysym.sym == USRKEY_PAUSE)
		paused = !paused;
	if (paused)
		return;
	//rotate right
	if (evt->type == SDL_KEYDOWN && evt->key.keysym.sym == USRKEY_B)
		if (activeQuad)
			activeQuad->rotateLeft();
	//rotate left
	if (evt->type == SDL_KEYDOWN && evt->key.keysym.sym == USRKEY_A)
		if (activeQuad)
			activeQuad->rotateRight();
	//move left
	if (evt->type == SDL_KEYDOWN && evt->key.keysym.sym == USRKEY_LEFT)
		if (activeQuad)
			activeQuad->moveLeft();
	//move right
	if (evt->type == SDL_KEYDOWN && evt->key.keysym.sym == USRKEY_RIGHT)
		if (activeQuad)
			activeQuad->moveRight();
	//immediate drop
	if (evt->type == SDL_KEYDOWN && evt->key.keysym.sym == SDLK_w)
		warpdrop = true;
	}

void Field::handleInput(Uint8* keystate)
	{
	if (paused)
		return;
	if (keystate[USRKEY_UP])
		{
		}
	if (keystate[USRKEY_DOWN])
		{
		harddrop = true;
		}
	}

void Field::step()
	{
	cdebug << "Entering Field::step()\n";

	if (paused)
		return;

	if (++frame % (SPEEDUP_DELAY * DEFAULT_INTERVAL / stepInterval) == 0) //if this many frames have passed
		{
		if (!harddrop) //prevent getting here several times in the same interval
			{
			std::cout << "Speed increased! (" << stepInterval << ")\n";
			speedUp();		
			}
		}
	
	//only force falling once an interval
	if ((!harddrop && !warpdrop && (frame % stepInterval)) || paused || clearing)
		{
		cdebug << "Skipping Field::step()\n";
		return;
		}


	//deploy the next tetra
	if (pushBlocks)
		{
		cdebug << "Deactivating old Quad\n";
		delete activeQuad;
		activeQuad = nextQuad;
		activeQuad->goTo(FIELD_WIDTH / 2, 0);
		cdebug << "Creating new Quad\n";
		nextQuad = createQuad();
		pushBlocks = false;
		}

	//drop the current tetra
	do
		{
		int result = activeQuad->fall();
		if (result)
			{
			if (result == NO_ROOM)
				fail = true;
			pushBlocks = true;
			activeQuad = NULL;
			warpdrop = false;
			}
		} while (warpdrop);
	harddrop = warpdrop = false;
	

	cdebug << "Exiting Field::step()\n";
	}

void Field::displayNextQuad(SDL_Surface* nextDisp)
	{
	SDL_FillRect(nextDisp, NULL, 0);
	nextQuad->display(nextDisp);
	}

void Field::display(SDL_Surface* screen)
	{
	cdebug << "Entering Field::display()\n";

	if (activeQuad)
		activeQuad->display(screen, true);

	
	for (int j = 0; j < FIELD_HEIGHT; ++j)
		{
		for (int i = 0; i < FIELD_WIDTH; ++i)
			{
			if (grid[i][j])
				{
#ifdef DEBUG
				SDL_Rect rect = {i * BLOCK_WIDTH, j * BLOCK_WIDTH, BLOCK_WIDTH, BLOCK_WIDTH};
				SDL_FillRect(screen, &rect, 0xffffffff);
#endif
				grid[i][j]->display(screen);
				}
			}
		}

	
	if (chunking)
		for (unsigned int i = 0; i < falling.size(); ++i)
			falling[i].display(screen);

	cdebug << "Exiting Field::display()\n";
	}

Quad* Field::createQuad()
	{
	return new Quad(1, 1, grid, skillLevel);
	}

int Field::chunkFall()
	{
	Coord here = {0, 0};

	static bool identifying = true; //are we still identifying chunks?

	bool taken[FIELD_WIDTH][FIELD_HEIGHT];
	for (int i = 0; i < FIELD_WIDTH; ++i)
		for (int j = 0; j < FIELD_HEIGHT; ++j)
			taken[i][j] = false;

	if (identifying)
		{
		//identify "chunks" of blocks
		for (int j = FIELD_HEIGHT - 1; j >= 0; --j)
			{
			for (int i = 0; i < FIELD_WIDTH; ++i)
				{
				if (taken[i][j] || grid[i][j] == NULL)
					continue;

				here.x = i;
				here.y = j;
				Region region(grid);
				Region::identifyRegion(region, here, -1);
				region.yank();
				falling.push_back(region);
				}
			}
		identifying = false;
		//return 0;
		}

	cdebug << "Chunking...\n";

	//drop each chunk
	for (unsigned int i = 0; i < falling.size(); ++i)
		{
		if (falling[i].fall() )
			{
			std::vector<Region>::iterator it = falling.begin();
			for (unsigned int j = 0; j < i; ++j)
				++it;
			falling.erase(it);
			}

		}

	if (falling.empty() )
		{
		chunking = false;
		identifying = true;
		return 1;
		}
		
	return 0;

	}

int Field::cascade(int from = FIELD_HEIGHT - 1)
	{
	static int i = 0;
	static int j = from;

	cdebug << "Entering Field::cascade() ";

	while (1) //find next block
		{
		if (j <= 0)
			{
			++i;
			j = from;
			}
		if (i >= FIELD_WIDTH)
			{
			i = 0;
			j = from;
			return 1;
			}

		if (grid[i][j])
			break;

		--j;

		}
	cdebug << "(" << i << " " << j << ")\n";
	grid[i][j]->fall(grid);
	if (grid[i][j]->blocked((const Block***)grid) ) //hit bottom
		{
		grid[i][j]->recede();
		cdebug << "\n";
		Block* temp = grid[i][j]; //update grid
		grid[i][j] = NULL;
		grid[i][temp->getY()] = temp;
		--j;
		return cascade(from);
		}

	cdebug << "Exiting Field::cascade()\n";
	return 0;
	}

void Field::clear()
	{
	rowclear();
	colorclear();	

	if (!(chunking || clearing) )
		{
		if (chain) //if we were chaining, we're finished now
			playerScore().calc();
		cdebug << "Chain: " << chain << "\n";
		chain = 0;
		}
	else if (chunking)
		{
		chunkFall();
		}
	}

bool Field::rowIsEmpty(int index)
	{
	cdebug << "Entering Field::rowIsEmpty(" << index << ")\n";
	if (grid[index] == NULL)
		{
		cdebug << "Exiting Field::rowIsEmpty() (NULL!)\n";
		return true;
		}
	for (int i = 0; i < FIELD_WIDTH; ++i)
		{
		if (grid[i][index] != NULL)
			{
			cdebug << "Exiting Field::rowIsEmpty() (" << i << ")\n";
			return false;
			}
		}
	cdebug << "Exiting Field::rowIsEmpty()\n";
	return true;
	}

bool Field::gameOver()
	{
	return fail;
	}

void Field::speedUp()
	{
	if (stepInterval > 1)
		--stepInterval;
	}



/////////////////
//private methods
/////////////////

void Field::rowclear()
	{
	static bool row[FIELD_HEIGHT];
	static int col = 0;
	Region temp(grid);
	
	cdebug << "Entering Field::rowclear()\n";

	if (cascading || chunking)
		return;
		
	else if (clearing) //if we're clearing, do the next column
		{
		for (int i = 0; i < FIELD_HEIGHT; ++i)
			{
			if (row[i]) //if this row is being cleared
				{
				Coord here = {col, i};
				if (grid[col][i])
					{
					Region::identifyRegion(temp, here, grid[col][i]->getColor() );
					if (temp.size() >= skillLevel)
						{
						playerScore().addBonus(B_COLORCLEAR, temp.size() );
						playerScore().addMultiplier(M_EXTRA, temp.size() - skillLevel);
						temp.clear();
						}
					else
						{
						temp.reset(); 
						grid[col][i]->aspload();
						}
					}
				//either way there's no more block here
				grid[col][i] = NULL;
				}
			}
		if (++col >= FIELD_WIDTH) //if we're finished clearing
			{
			int rowscleared = 0;

			for (int i = 0; i < FIELD_HEIGHT; ++i)
				{
				if (row[i])
					{
					++rowscleared;
					row[i] = false;
					}
				}

			playerScore().addBonus(B_ROWCLEAR, rowscleared);
//			std::cout << "Updating score...\n";
			col = 0;
			chunking = true;
			clearing = false;
			playerScore().addMultiplier(M_CHAIN, chain++);
			}
		}


	else //else see if we need to clear
		{
		for (int i = 0; i < FIELD_HEIGHT; ++i) //for each row
			{
			bool needsclearing = true;
			for (int j = 0; j < FIELD_WIDTH; ++j) //see if this row is full
				{
				if (!grid[j][i])
					{
					needsclearing = false;
					break; //this row doesn't need clearing
					}
				}
			if (needsclearing)
				{
				clearing = true;
				row[i] = true;
				for (int j = 0; j < FIELD_WIDTH; ++j)
					{
					grid[j][i]->condemn();
					}
				}
			}
		if (!clearing) //nothing needs to be cleared
			{
			col = 0;
			for (int i = 0; i < FIELD_HEIGHT; ++i)
				row[i] = false;
			}
		}
	cdebug << "Exiting Field::rowclear()\n";
	}

void Field::colorclear()
	{
	Region region(grid);
	Coord here;
	std::vector<Coord> done;
	
	cdebug << "Entering Field::colorclear()\n";

	if (clearing || cascading || chunking)
		return;

	for (unsigned int i = 0; i < FIELD_WIDTH; ++i)
		{
		for (unsigned int j = 0; j < FIELD_HEIGHT; ++j)
			{
			bool skip = false;
			if (grid[i][j] == NULL)
				continue;

			region.reset();
			here.x = i; here.y = j;
			for (unsigned int k = 0; k < done.size(); ++k)
				if (done[k] == here)
					skip = true;
			if (skip)
				continue;
			
			Region::identifyRegion(region, here, grid[i][j]->getColor() );
			const std::vector<Block*>& cur = region.getBlocks();
			for (unsigned int i = 0 ; i < cur.size(); ++i)
				{
				Coord c = {cur[i]->getX(), cur[i]->getY() };
				done.push_back( c );
				}
			
			if (region.size() >= skillLevel)
				{
				playerScore().addBonus(B_COLORCLEAR, region.size() );
				playerScore().addMultiplier(M_EXTRA, region.size() - skillLevel);

				chunking = true;

				region.clear();
				playerScore().addMultiplier(M_CHAIN, chain++);
				}
			}
		}

	cdebug << "Exiting Field::colorclear()\n";
	}



Field& playField(bool reset)
	{
	static Field* f = new Field();
	if (reset)
		{
		delete f;
		f = new Field();
		}
	return *f;
	}

bool Field::chaining()
	{
	return chain;
	}
