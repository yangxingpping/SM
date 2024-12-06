
cmake_minimum_required(VERSION 3.25) # GENERATOR_IS_MULTI_CONFIG

project(Hotupdate VERSION 1.2.3)


if(MSVC)
  set(pdb_output_dir "${CMAKE_CURRENT_BINARY_DIR}/pdb-files")

  set(CMAKE_PDB_OUTPUT_DIRECTORY "${pdb_output_dir}")
  set(CMAKE_COMPILE_PDB_OUTPUT_DIRECTORY "${pdb_output_dir}")

  get_cmake_property(is_multi GENERATOR_IS_MULTI_CONFIG)
  if(is_multi)
    set(config_suffix "$<CONFIG>")
  else()
    set(config_suffix "")
  endif()

  include(GNUInstallDirs)

  if(BUILD_SHARED_LIBS)
    set(pdb_dst ${CMAKE_INSTALL_BINDIR})
  else()
    set(pdb_dst ${CMAKE_INSTALL_LIBDIR})
  endif()

  install(
      DIRECTORY "${pdb_output_dir}/${config_suffix}/"
      DESTINATION ${pdb_dst}
  )
endif()

IF(CMAKE_SYSTEM_NAME MATCHES "Windows")
	#link_libraries(Ws2_32)
    add_compile_options(/Zc:__cplusplus /Wv:19.27)
    #ADD_LIBRARY(Hotupdate SHARED ${A})
    #set_target_properties(Hotupdate  PROPERTIES VS_GLOBAL_VcpkgEnabled true)
ELSE()
    #ADD_LIBRARY(Hotupdate  ${A})
ENDIF(CMAKE_SYSTEM_NAME MATCHES "Windows")

####
# Create targets
aux_source_directory(./ A)
add_library(Hotupdate ${A})

target_link_libraries(Hotupdate PRIVATE spdlog::spdlog PRIVATE whereami PRIVATE efsw::efsw)

####
# Properties of targets

# Add definitions for targets
# Values:
#   * Debug: -DFOO_Hotupdate_DEBUG=1
#   * Release: -DFOO_Hotupdate_DEBUG=0
#   * other: -DFOO_Hotupdate_DEBUG=0
target_compile_definitions(Hotupdate PUBLIC "Hotupdate_DEBUG=$<CONFIG:Debug>")

set(generated_headers "${CMAKE_CURRENT_BINARY_DIR}/generated_headers")
set(Hotupdate_export "${generated_headers}/Hotupdate_EXPORT.h")

include(GenerateExportHeader)
generate_export_header(Hotupdate EXPORT_FILE_NAME ${Hotupdate_export})

target_include_directories(
    Hotupdate PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>"
    "$<BUILD_INTERFACE:${generated_headers}>"
)


include(GNUInstallDirs)

set(config_install_dir "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")

# Configuration
set(version_config "${generated_dir}/${PROJECT_NAME}ConfigVersion.cmake")
set(project_config "${generated_dir}/${PROJECT_NAME}Config.cmake")
set(TARGETS_EXPORT_NAME "${PROJECT_NAME}Targets")
#set(namespace "${PROJECT_NAME}::")

# Include module with fuction 'write_basic_package_version_file'
include(CMakePackageConfigHelpers)

# Configure '<PROJECT-NAME>ConfigVersion.cmake'
# Use:
#   * PROJECT_VERSION
write_basic_package_version_file(
    "${version_config}" COMPATIBILITY SameMajorVersion
)

# Configure '<PROJECT-NAME>Config.cmake'
# Use variables:
#   * TARGETS_EXPORT_NAME
#   * PROJECT_NAME
configure_package_config_file(
    "./cmake/Config.cmake.in"
    "${project_config}"
    INSTALL_DESTINATION "${config_install_dir}"
)

install(
    TARGETS Hotupdate
    EXPORT "${TARGETS_EXPORT_NAME}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

# Headers:
#   * Source/foo/Hotupdate.hpp -> <prefix>/include/foo/Hotupdate.hpp
#   * Source/foo/Baz.hpp -> <prefix>/include/foo/Baz.hpp
install(
    DIRECTORY "./"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/Hotupdate"
    FILES_MATCHING PATTERN "*.h"
)

# Export headers:
#   * ${CMAKE_CURRENT_BINARY_DIR}/.../Hotupdate_EXPORT.h -> <prefix>/include/Hotupdate_EXPORT.h
install(
    FILES "${Hotupdate_export}"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

# Config
#   * <prefix>/lib/cmake/Foo/FooConfig.cmake
#   * <prefix>/lib/cmake/Foo/FooConfigVersion.cmake
install(
    FILES "${project_config}" "${version_config}"
    DESTINATION "${config_install_dir}"
)

# Config
#   * <prefix>/lib/cmake/Foo/FooTargets.cmake
install(
    EXPORT "${TARGETS_EXPORT_NAME}"
    NAMESPACE "${namespace}"
    DESTINATION "${config_install_dir}"
)

# }
