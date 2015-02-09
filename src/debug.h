#ifndef DEBUG_H
#define DEBUG_H

#define DEBUG

#ifdef DEBUG
#	define cdebug std::cout
#else
#	define cdebug if(0)std::cout
#endif


#endif
