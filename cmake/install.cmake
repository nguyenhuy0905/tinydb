if(CMAKE_SKIP_INSTALL_RULES)
  return()
endif()
if(tinydb_MODULE)
  set(tinydb_INSTALLS tinydb_lib_module tinydb_compile_options)
else()
  set(tinydb_INSTALLS tinydb_lib tinydb_compile_options)
endif()
include(GNUInstallDirs)
install(TARGETS ${tinydb_INSTALLS}
  EXPORT tinydbTargets
  FILE_SET CXX_MODULES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tinydb
  FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tinydb
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tinydb
  PRIVATE_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tinydb/internal
)
if(EXISTS "${PROJECT_BINARY_DIR}/tinydb_export.h")
  install(FILES
    ${PROJECT_BINARY_DIR}/tinydb_export.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tinydb
  )
endif()
install(EXPORT tinydbTargets
  FILE tinydbTargets.cmake
  NAMESPACE tinydb::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tinydb
)
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/tinydbConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

install(FILES "${CMAKE_CURRENT_LIST_DIR}/install-config.cmake"
  RENAME "tinydbConfig.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tinydb
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/tinydbConfigVersion.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tinydb)
