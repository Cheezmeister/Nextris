#include <map>
#include "block.h"
#include "field.h"
#include "display.h"
#include "audio.h"


static StochasticScintillator<6> scintColors(0.1);


//Block
Block::Block()
{
    cdebug << "Wrong!\n";
}
Block::Block(int X, int Y, unsigned char Color)
{
    mask.x = X * BLOCK_WIDTH;
    mask.y = Y * BLOCK_WIDTH;
    mask.w = mask.h = BLOCK_WIDTH;
    color = Color;
    active = true;
    condemned = false;
}

Block::~Block()
{

}
int Block::blocked(const Block*** grid)
{
    if (getX() < 0)
    {
        cdebug << "side\n";
        return 2;
    }
    if (getX() >= FIELD_WIDTH) //hit side
    {
        cdebug << "right\n";
        return 3;
    }
    if (getY() >= FIELD_HEIGHT) //hit bottom
    {
        cdebug << "bottom\n";
        return 1;
    }
    if (grid[getX()][getY()] != NULL) //hit block
    {
        cdebug << "blocked at " << getX() << getY() << "!\n";
        return 1;
    }
    return 0;
}

int Block::fall(Block*** grid)
{
    cdebug << "Entering Block::fall()\n";
    mask.y += BLOCK_WIDTH;

    if (!blocked((const Block***)grid) )
    {
        return 0;
    }
    cdebug << "Blocked!\n";
    return 1;
}
void Block::aspload()
{
    nextris::audio::play_sound(color, getX(), getY(), SND_ASPLOADED);
    Particle::createParticles(rand() % PARTICLINESS, mask.x, mask.y, color);
    delete this;
}
void Block::recede()
{
    mask.y -= BLOCK_WIDTH;
}
void Block::updateGrid(Block*** grid)
{
    grid[getX()][getY()] = this;
}
void Block::display(SDL_Surface* screen)
{
    SDL_Rect bmask = mask;

    SDL_Color base = BASE_COLORS[color];
    SDL_Color sdlcolor = scintColors[color];
    if (condemned)
    {
        Uint32 col = sdlc_to_u32(sdlcolor, screen);
        SDL_FillRect(screen, &mask, col);
    }
    while (bmask.w > 0 && bmask.h > 0)
    {
        ++bmask.x;
        ++bmask.y;
        bmask.w -= 2;
        bmask.h -= 2;

        float t = (mask.w - bmask.w) / (float)mask.w;
        Uint8 br = sdlcolor.r; //lerp(sdlcolor.r, base.r, t);
        Uint8 bg = sdlcolor.g; //lerp(sdlcolor.g, base.g, t);
        Uint8 bb = sdlcolor.b; //lerp(sdlcolor.b, base.b, t);

        br = lerp((Uint8)0, br, t);
        bg = lerp((Uint8)0, bg, t);
        bb = lerp((Uint8)0, bb, t);

        Uint32 col = SDL_MapRGB(screen->format, br, bg, bb);
        SDL_FillRect(screen, &bmask, col);
    }

}

unsigned char Block::getColor()
{
    return color;
}

void Block::deactivate()
{
    active = false;
}
void Block::poomp()
{
    Particle::createBouncyParticles(PARTICLINESS, mask.x + BLOCK_WIDTH / 2, mask.y + BLOCK_WIDTH / 2, color);
}

int Block::getX()
{
    return mask.x / BLOCK_WIDTH;
}
int Block::getY()
{
    return mask.y / BLOCK_WIDTH;
}
void Block::setX(int x)
{
    mask.x = x * BLOCK_WIDTH;
}
void Block::setY(int y)
{
    mask.y = y * BLOCK_WIDTH;
}
void Block::rotateAround(Coord center, int clockwise)
{
    int xd = getX() - center.x;
    int yd = getY() - center.y;

    if (!clockwise)
        clockwise = -1;
    else
        clockwise = 1;

    setX(center.x + yd * clockwise);
    setY(center.y - xd * clockwise);
}
int Block::isAt(Coord loc)
{
    return getX() == loc.x && getY() == loc.y;
}
void Block::condemn()
{
    condemned = true;
}

//Quad
Quad::Quad(int X, int Y, Block*** Grid, unsigned char level) : shape(rand() % N_SHAPES)
{
    //shape = rand() % 7;
    master = new Block(X, Y, rand() % level);
    grid = Grid;
    cdebug << "Quad created at " << X << "," << Y << "\n";
    switch (shape)
    {
    case SHAPE_I:
        slave1 = new Block(X, Y - 1, rand() % level);
        slave2 = new Block(X, Y + 1, rand() % level);
        slave3 = new Block(X, Y + 2, rand() % level);
        break;
    case SHAPE_L:
        slave1 = new Block(X, Y - 1, rand() % level);
        slave2 = new Block(X, Y + 1, rand() % level);
        slave3 = new Block(X + 1, Y + 1, rand() % level);
        break;
    case SHAPE_J:
        slave1 = new Block(X, Y - 1, rand() % level);
        slave2 = new Block(X, Y + 1, rand() % level);
        slave3 = new Block(X - 1, Y + 1, rand() % level);
        break;
    case SHAPE_T:
        slave1 = new Block(X - 1, Y, rand() % level);
        slave2 = new Block(X + 1, Y, rand() % level);
        slave3 = new Block(X, Y + 1, rand() % level);
        break;
    case SHAPE_O:
        slave1 = new Block(X + 1, Y, rand() % level);
        slave2 = new Block(X + 1, Y + 1, rand() % level);
        slave3 = new Block(X, Y + 1, rand() % level);
        break;
    case SHAPE_Z:
        slave1 = new Block(X, Y - 1, rand() % level);
        slave2 = new Block(X - 1, Y, rand() % level);
        slave3 = new Block(X - 1, Y + 1, rand() % level);
        break;
    case SHAPE_S:
        slave1 = new Block(X, Y - 1, rand() % level);
        slave2 = new Block(X + 1, Y, rand() % level);
        slave3 = new Block(X + 1, Y + 1, rand() % level);
        break;
    }
}

Quad::~Quad()
{
    //?
}

int Quad::fall()
{
    cdebug << "Entering Quad::fall()\n";

    bool blocked = master->fall(grid) +
                   slave1->fall(grid) +
                   slave2->fall(grid) +
                   slave3->fall(grid);

    if (blocked)
    {
        master->recede();
        slave1->recede();
        slave2->recede();
        slave3->recede();

        master->deactivate();
        slave1->deactivate();
        slave2->deactivate();
        slave3->deactivate();

        if (master->getY() < 0 ||
                slave1->getY() < 0 ||
                slave2->getY() < 0 ||
                slave3->getY() < 0)
        {
            return NO_ROOM;
        }
        grid[master->getX()][master->getY()] = master;
        grid[slave1->getX()][slave1->getY()] = slave1;
        grid[slave2->getX()][slave2->getY()] = slave2;
        grid[slave3->getX()][slave3->getY()] = slave3;

        cdebug << "Exiting Quad::fall()\n";
        return FELL_AND_SET;
    }


    cdebug << "Exiting Quad::fall()\n";
    return FELL_OKAY;
}

void Quad::poomp()
{
    master->poomp();
    slave1->poomp();
    slave2->poomp();
    slave3->poomp();
}
void Quad::rotateLeft()
{
    if (shape == SHAPE_O)
    {
        Coord temp = {master->getX(), master->getY() };
        master->setX(slave1->getX() );
        master->setY(slave1->getY() );
        slave1->setX(slave2->getX() );
        slave1->setY(slave2->getY() );
        slave2->setX(slave3->getX() );
        slave2->setY(slave3->getY() );
        slave3->setX(temp.x);
        slave3->setY(temp.y);
        return; //no need to check for changed position
    }

    Coord center = {master->getX(), master->getY() };
    slave1->rotateAround(center, 0);
    slave2->rotateAround(center, 0);
    slave3->rotateAround(center, 0);

    //first check if we've rotated off the left
    for (int i = 0; i < 4 &&
            (master->blocked((const Block***)grid) == 2 ||
             slave1->blocked((const Block***)grid) == 2 ||
             slave2->blocked((const Block***)grid) == 2 ||
             slave3->blocked((const Block***)grid) == 2);
            ++i)
        moveRight();

    //then right
    for (int i = 0; i < 4 &&
            (master->blocked((const Block***)grid) == 3 ||
             slave1->blocked((const Block***)grid) == 3 ||
             slave2->blocked((const Block***)grid) == 3 ||
             slave3->blocked((const Block***)grid) == 3);
            ++i)
        moveLeft();

    if (slave1->blocked((const Block***)grid) ||
            slave3->blocked((const Block***)grid) ||
            slave2->blocked((const Block***)grid) )
    {
        slave1->rotateAround(center, 1);
        slave2->rotateAround(center, 1);
        slave3->rotateAround(center, 1);
    }
    else
        nextris::audio::play_sound(master->getColor(), master->getX(), master->getY(), SND_ROTATELEFT);
}
void Quad::rotateRight()
{
    if (shape == SHAPE_O)
    {
        Coord temp = {master->getX(), master->getY() };
        master->setX(slave3->getX() );
        master->setY(slave3->getY() );
        slave3->setX(slave2->getX() );
        slave3->setY(slave2->getY() );
        slave2->setX(slave1->getX() );
        slave2->setY(slave1->getY() );
        slave1->setX(temp.x);
        slave1->setY(temp.y);
        return; //no need to check for changed position
    }

    Coord center = {master->getX(), master->getY() };
    slave1->rotateAround(center, 1);
    slave2->rotateAround(center, 1);
    slave3->rotateAround(center, 1);

    //first check if we've rotated off the left
    for (int i = 0; i < 4 &&
            (master->blocked((const Block***)grid) == 2 ||
             slave1->blocked((const Block***)grid) == 2 ||
             slave2->blocked((const Block***)grid) == 2 ||
             slave3->blocked((const Block***)grid) == 2);
            ++i)
        moveRight();

    //then right
    for (int i = 0; i < 4 &&
            (master->blocked((const Block***)grid) == 3 ||
             slave1->blocked((const Block***)grid) == 3 ||
             slave2->blocked((const Block***)grid) == 3 ||
             slave3->blocked((const Block***)grid) == 3);
            ++i)
        moveLeft();

    if (slave1->blocked((const Block***)grid) ||
            slave3->blocked((const Block***)grid) ||
            slave2->blocked((const Block***)grid) )
    {
        slave1->rotateAround(center, 0);
        slave2->rotateAround(center, 0);
        slave3->rotateAround(center, 0);
    }
    else
        nextris::audio::play_sound(master->getColor(), master->getX(), master->getY(), SND_ROTATERIGHT);
}
void Quad::moveLeft()
{
    master->setX(master->getX() - 1);
    slave1->setX(slave1->getX() - 1);
    slave2->setX(slave2->getX() - 1);
    slave3->setX(slave3->getX() - 1);


    if (master->blocked((const Block***)grid) ||
            slave1->blocked((const Block***)grid) ||
            slave2->blocked((const Block***)grid) ||
            slave3->blocked((const Block***)grid) )
    {
        master->setX(master->getX() + 1);
        slave1->setX(slave1->getX() + 1);
        slave2->setX(slave2->getX() + 1);
        slave3->setX(slave3->getX() + 1);
    }

    else
        nextris::audio::play_sound(master->getColor(), master->getX(), master->getY(), SND_SHIFTLEFT);
}
void Quad::moveRight()
{
    master->setX(master->getX() + 1);
    slave1->setX(slave1->getX() + 1);
    slave2->setX(slave2->getX() + 1);
    slave3->setX(slave3->getX() + 1);

    if (master->blocked((const Block***)grid) ||
            slave1->blocked((const Block***)grid) ||
            slave2->blocked((const Block***)grid) ||
            slave3->blocked((const Block***)grid) )
    {
        master->setX(master->getX() - 1);
        slave1->setX(slave1->getX() - 1);
        slave2->setX(slave2->getX() - 1);
        slave3->setX(slave3->getX() - 1);
    }

    else
        nextris::audio::play_sound(master->getColor(), master->getX(), master->getY(), SND_SHIFTRIGHT);
}

void Quad::goTo(int X, int Y)
{
    slave1->setX(slave1->getX() + X - master->getX() );
    slave2->setX(slave2->getX() + X - master->getX() );
    slave3->setX(slave3->getX() + X - master->getX() );
    master->setX(X);

    slave1->setY(slave1->getY() + Y - master->getY());
    slave2->setY(slave2->getY() + Y - master->getY());
    slave3->setY(slave3->getY() + Y - master->getY());
    master->setY(Y);
}

void shadow_helper(Block* b, SDL_Surface* s)
{
    SDL_Rect shadow;
    shadow.x = b->getX() * BLOCK_WIDTH;
    shadow.y = b->getY() * BLOCK_WIDTH;
    shadow.w = BLOCK_WIDTH;
    shadow.h = FIELD_HEIGHT * BLOCK_WIDTH;
    SDL_Color color = scintColors[b->getColor()];
    //SDL_FillRect(s, &shadow, );
    vertical_gradient(s, shadow, fade_color(scintColors[b->getColor()], 8));
}

void Quad::display(SDL_Surface* screen, bool dropshadow)
{
    cdebug << "Entering Quad::display()\n";

    if (dropshadow)
    {
        shadow_helper(master, screen);
        shadow_helper(slave1, screen);
        shadow_helper(slave2, screen);
        shadow_helper(slave3, screen);
    }

    master->display(screen);
    slave1->display(screen);
    slave2->display(screen);
    slave3->display(screen);

    cdebug << "Exiting Quad::display()\n";
}

//Region

Region::Region(Block*** Grid)
{
    grid = Grid;
}

Region::~Region()
{

}

unsigned int Region::size()
{
    return blocks.size();
}

void Region::reset()
{
    blocks.clear();
}

void Region::yank()
{
    for (unsigned int i = 0; i < blocks.size(); ++i)
        if (blocks[i] == grid[blocks[i]->getX()][blocks[i]->getY()] )
            grid[blocks[i]->getX()][blocks[i]->getY()] = NULL;
        else
            cdebug << "uh-oh\n";
}
void Region::clear()
{
    int X, Y;
    for (unsigned int i = 0; i < blocks.size(); ++i)
    {
        X = blocks[i]->getX();
        Y = blocks[i]->getY();
        grid[X][Y] = NULL;
        blocks[i]->aspload();
    }
    reset();
}

void Region::addBlock(Block* block)
{
    blocks.push_back(block);
}

int Region::fall()
{
    bool hit = false;

    cdebug << "Entering Region::fall()\n";

    if (!valid() )
    {
        cdebug << "wtf mite! exiting!\n";
        return 1;
    }

    //fall a notch
    for (unsigned int i = 0; i < blocks.size(); ++i)
        if (blocks[i]->fall(grid) )
            hit = true;
//	//see if one of the blocks has hit
//	for (int i = 0; i < blocks.size(); ++i)
//		if (blocks[i]->blocked((const Block***)grid) )
//			hit = true;

    if (hit)
    {
        //clean up when we've hit.
        cdebug << "Region hit!\n";
        for (unsigned int i = 0; i < blocks.size(); ++i)
        {
            if (!valid() )
                cdebug << "haHAH!\n";
            blocks[i]->recede();
        }
        for (unsigned int i = 0; i < blocks.size(); ++i)
        {
            blocks[i]->updateGrid(grid);
        }
        cdebug << "Exiting Region::fall()\n";
        return 1;
    }

    cdebug << "Exiting Region::fall()\n";
    return 0;
}

int Region::valid()
{
    for (unsigned int i = 0; i < blocks.size(); ++i)
        if (!blocks[i] || blocks[i]->getX() > 10 || blocks[i]->getY() > 20)
            return 0;
    return 1;
}

void Region::display(SDL_Surface* screen)
{
    for (unsigned int i = 0; i < blocks.size(); ++i)
        blocks[i]->display(screen);
}
const std::vector<Block*>& Region::getBlocks()
{
    return blocks;
}
//recursively identify a contiguous region of the same color, or any color
//contiguous here means all blocks are connected by an edge
void Region::identifyRegion(Region& region, Coord orig, unsigned char color)
{
    if (orig.x >= FIELD_WIDTH || orig.y >= FIELD_HEIGHT ||
            orig.x < 0 || orig.y < 0) //edge!
        return;
    if (region.grid[orig.x][orig.y] == NULL) //no block
        return;
    if (region.grid[orig.x][orig.y]->getColor() != color && color != 255) //wrong color
        return;

    cdebug << "Entering Region::identifyRegion()\n";

    //prevent duplicate adds
    for (unsigned int i = 0; i < region.blocks.size(); ++i)
        if (region.blocks[i]->isAt(orig) )
            return;
        else if (region.blocks[i] == region.grid[orig.x][orig.y])
            return;

    //add this block
    cdebug << "Adding " << orig.x << "," << orig.y << "\n";
    if (region.grid[orig.x][orig.y] == NULL)
        cdebug << "BUT IT'S NULL!!1????\n";
    region.addBlock(region.grid[orig.x][orig.y]);

    //recurse for neighbors
    Coord c1 = {orig.x, orig.y + 1},
          c2 = {orig.x, orig.y - 1},
          c3 = {orig.x - 1, orig.y},
          c4 = {orig.x + 1, orig.y};

    identifyRegion(region, c1, color);
    identifyRegion(region, c2, color);
    identifyRegion(region, c3, color);
    identifyRegion(region, c4, color);

    cdebug << "Exiting Region::identifyRegion()\n";
}
