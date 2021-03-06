#include "field.h"
#include "block.h"
#include "options.h"

using namespace std;

Score::Score()
{
    total = 0;
}
void Score::addMultiplier(Multiplier mult, unsigned int arg)
{
    if (mult == M_CHAIN)
        mlt += arg * 2;
    if (mult == M_EXTRA || mult == M_DOUBLE)
        mlt += arg;
    else
        cdebug << "Unknown multiplier, value: " << mult << "\n";
}
void Score::addBonus(Bonus bonus, unsigned int arg)
{
    cdebug << "Entering Score::addBonus()\n";
    if (bonus == B_ROWCLEAR)
        if (arg == 1)
            curr += 40;
        else if (arg == 2)
            curr += 100;
        else if (arg == 3)
            curr += 300;
        else if (arg == 4)
            curr += 1200;
        else
            curr += 1000 * arg;

    else if (bonus == B_COLORCLEAR)
        curr += 20 * arg;

    else if (bonus == B_DOUBLECLEAR)
    {
        addBonus(B_ROWCLEAR, arg);
        addBonus(B_COLORCLEAR, arg * nextris::options::get_options().game.width);
        addMultiplier(M_DOUBLE, arg);
    }
    cdebug << "Exiting Score::addBonus()\n";
}

//add running to total & reset running
void Score::calc()
{
    cout << curr * mlt << " points!\n";
    total += curr * mlt;
    mlt = 1;
    curr = 0;
}
unsigned long Score::getTotal()
{
    return total;
}

void Score::display(SDL_Surface* screen) const
{

    static unsigned long temp = 0;
    cdebug << "Entering Score::display()\n";


    //get decimal digits
    char scorecharstar[10]; // yes, I'm horrible.
    sprintf(scorecharstar, "%d", total);

    for (int i = 0; scorecharstar[i]; ++i)
    {
        for (int j = scorecharstar[i] - '0'; j >= 0; --j) //work from the bottom up
        {
            //draw rect where it goes
            if (i < 6) //scores up to one million display like this
            {
                Block dblock(i, 9 - j, i);
                dblock.display(screen);
            }
        }
    }
    cdebug << "Exiting Score::display()\n";
}

Score& playerScore(bool reset)
{
    static Score sc;
    if (reset)
    {
        sc.curr = sc.total = 0;
        sc.mlt = 1;
    }
    return sc;
}
