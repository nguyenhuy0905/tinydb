target_sources(tinydb_test
    PRIVATE
    dbfile_test.cxx
)
target_link_libraries(tinydb_test
    PRIVATE
    tinydb_dbfile
)
