#pragma once

#ifdef SPECULOS
#define DEBUG(_s) debug_write(_s)
#else
#define DEBUG(_s)
#endif