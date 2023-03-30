
PROJECT(rayX)

set(CMAKE_CXX_STANDARD 20)

include_directories(../configs ../database ../Utils ../inc ../..  ../inc/networks  ./ )


FILE(GLOB_RECURSE CUR_HEADER "./*.h")
source_group("IncCur" FILES ${CUR_HEADER})

FILE(GLOB_RECURSE COMMN_HEADER "../inc/*.h")
source_group("IncComm" FILES ${COMMN_HEADER})

include_directories(../conf ../inc ../Utils ../hotupdate)

AUX_SOURCE_DIRECTORY(. A)

IF(CMAKE_SYSTEM_NAME MATCHES "Windows")
    ADD_LIBRARY(rayX SHARED   ${A} ${CUR_HEADER} ${COMMN_HEADER})
    set_target_properties(rayX  PROPERTIES VS_GLOBAL_VcpkgEnabled true)
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Linux")
    add_compile_options(-fcoroutines)
    ADD_LIBRARY(rayX  ${A})
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    ADD_LIBRARY(rayX  ${A})
ENDIF(CMAKE_SYSTEM_NAME MATCHES "Windows")

#target_link_libraries(rayX  PRIVATE ZLIB::ZLIB PRIVATE nng::nngpp    PRIVATE Utils PRIVATE configs PRIVATE hotupdate PRIVATE nlohmann_json::nlohmann_json PRIVATE nng::nng PRIVATE spdlog::spdlog PRIVATE OpenSSL::SSL OpenSSL::Crypto)


generate_export_header(rayX EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/../inc/os/${CMAKE_SYSTEM_NAME}/export_flags/${PROJECT_NAME}Export.h)

