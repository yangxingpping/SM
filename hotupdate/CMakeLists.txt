
PROJECT(hotupdate)

set(CMAKE_CXX_STANDARD 20)

FILE(GLOB_RECURSE MY_HEADERS "./*.h")
source_group("Headers" FILES ${MY_HEADERS})

FILE(GLOB_RECURSE MY_INTERFACEHEADERS "../inc/networks/*.h")
source_group("Headers" FILES ${MY_INTERFACEHEADERS})

include_directories(../inc ../Utils ../database ../configs)

AUX_SOURCE_DIRECTORY(. A)

IF(CMAKE_SYSTEM_NAME MATCHES "Windows")
    ADD_LIBRARY(hotupdate SHARED  ${A})
    set_target_properties(hotupdate  PROPERTIES VS_GLOBAL_VcpkgEnabled true)
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Linux")
    add_compile_options(-fcoroutines)
    ADD_LIBRARY(hotupdate  ${A} )
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    ADD_LIBRARY(hotupdate  ${A} )
ENDIF(CMAKE_SYSTEM_NAME MATCHES "Windows")

target_link_libraries(hotupdate    PRIVATE Utils PRIVATE configs PRIVATE spdlog::spdlog PRIVATE efsw::efsw)

generate_export_header(hotupdate EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/../inc/os/${CMAKE_SYSTEM_NAME}/export_flags/${PROJECT_NAME}Export.h)

