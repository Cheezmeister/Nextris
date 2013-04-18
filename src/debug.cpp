#include "debug.h"
#ifdef DEBUG
#	ifdef DEBUG_TO_FILE
		std::ofstream mydebugfile("cdebug.txt");
#	endif
#endif
