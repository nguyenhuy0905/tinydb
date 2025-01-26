add_library(tinydb_utest)
target_sources(tinydb_utest PUBLIC FILE_SET
    CXX_MODULES FILES
    utest.cxx)
