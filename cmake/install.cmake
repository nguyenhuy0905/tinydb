if(CMAKE_SKIP_INSTALL_RULES)
  return()
endif()
if(template_MODULE)
  set(template_INSTALLS template_lib_module template_compile_options)
else()
  set(template_INSTALLS template_lib template_compile_options)
endif()
include(GNUInstallDirs)
install(TARGETS ${template_INSTALLS}
  EXPORT templateTargets
  FILE_SET CXX_MODULES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/template
  FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/template
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/template
)
if(EXISTS "${PROJECT_BINARY_DIR}/template_export.h")
  install(FILES
    ${PROJECT_BINARY_DIR}/template_export.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/template
  )
endif()
install(EXPORT templateTargets
  FILE templateTargets.cmake
  NAMESPACE template::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/template
)
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/templateConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

install(FILES "${CMAKE_CURRENT_LIST_DIR}/install-config.cmake"
  RENAME "templateConfig.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/template
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/templateConfigVersion.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/template)
