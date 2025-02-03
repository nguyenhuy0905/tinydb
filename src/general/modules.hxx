#ifndef TINYDB_MODULES_HXX
#define TINYDB_MODULES_HXX

#include "tinydb_export.h"

#undef TINYDB_EXPORT
#ifdef ENABLE_MODULES
#define TINYDB_EXPORT export
#else
#define TINYDB_EXPORT
#endif // ENABLE_MODULES

#endif // !TINYDB_MODULES_HXX
