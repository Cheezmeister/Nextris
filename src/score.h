#ifndef SCORE_H
#define SCORE_H

#include <SDL.h>
#include <sstream>

#define SCORE_X FIELD_WIDTH
#define SCORE_Y 7
#define SCORE_WIDTH FIELD_WIDTH


typedef enum {
	M_CHAIN,
	M_EXTRA,
	M_DOUBLE
} Multiplier;

typedef enum {
	B_ROWCLEAR,
	B_COLORCLEAR,
	B_DOUBLECLEAR,
} Bonus;

class Score
	{
	private:
		unsigned long total; //grand total
		unsigned long curr; //current running
		unsigned int mlt; //current multiplier
		SDL_Surface* screen;
		Score();
	public:
		friend Score& playerScore(bool reset);
		void addMultiplier(Multiplier mult, unsigned int arg);
		void addBonus(Bonus bonus, unsigned int arg);
		void calc();
		unsigned long getTotal();
		void display(SDL_Surface* Screen) const;
	};

Score& playerScore(bool reset = false);

#endif // SCORE_H
