#include "tinydb_export.h"

// simply change TINYDB_EXPORT into the export keyword.

#if defined TINYDB_EXPORT
#if defined ENABLE_MODULE
#undef TINYDB_EXPORT
#define TINYDB_EXPORT export
#endif // ENABLE_MODULE
#endif // TINYDB_EXPORT
