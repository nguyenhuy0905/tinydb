#ifndef TINYDB_MODULE_CFG_H
#define TINYDB_MODULE_CFG_H

#include "tinydb_export.h" // NOLINT

/*
 * Simply switch TINYDB_EXPORT into export (module)
 * */
    #ifdef ENABLE_MODULE
        #ifdef TINYDB_EXPORT
            #undef TINYDB_EXPORT
            #define TINYDB_EXPORT export
        #endif
    #endif
#endif
