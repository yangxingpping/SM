
PROJECT(Utils)

set(CMAKE_CXX_STANDARD 20)

FILE(GLOB_RECURSE MY_HEADERS "./*.h")
source_group("Headers" FILES ${MY_HEADERS})

FILE(GLOB_RECURSE MY_INTERFACEHEADERS "../inc/networks/*.h")
source_group("Headers" FILES ${MY_INTERFACEHEADERS})

add_definitions(-DBUILD_SHARED_LIBS -DUtils_EXPORTS)

include_directories(../inc)

AUX_SOURCE_DIRECTORY(. A)

IF(CMAKE_SYSTEM_NAME MATCHES "Windows")
	link_libraries(Ws2_32)
    add_compile_options(/Zc:__cplusplus /Wv:19.27)
    ADD_LIBRARY(Utils SHARED ${A})
    set_target_properties(Utils  PROPERTIES VS_GLOBAL_VcpkgEnabled true)
ELSE()
    ADD_LIBRARY(Utils  ${A})
ENDIF(CMAKE_SYSTEM_NAME MATCHES "Windows")

include(GenerateExportHeader)
generate_export_header(Utils EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/../inc/os/${CMAKE_SYSTEM_NAME}/export_flags/${PROJECT_NAME}Export.h)

target_include_directories(
    Utils SYSTEM
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/export>"
)

target_link_libraries(Utils PRIVATE spdlog::spdlog PRIVATE date::date date::date-tz PRIVATE msgpack-c)



