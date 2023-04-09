
PROJECT(networkinterface)

set(CMAKE_CXX_STANDARD 20)

include_directories(../configs ../database ../Utils ../inc ../..  ../inc/networks ./nngimpl ./tcpimpl ./websocketimpl ./ ../../capn)


FILE(GLOB_RECURSE CUR_HEADER "./*.h")
source_group("IncCur" FILES ${CUR_HEADER})

FILE(GLOB_RECURSE COMMN_HEADER "../inc/*.h")
source_group("IncComm" FILES ${COMMN_HEADER})

include_directories(../conf ../inc ../Utils ../hotupdate ./nng ./socket)

AUX_SOURCE_DIRECTORY(. A)
aux_source_directory(./nng B)
aux_source_directory(./socket C)
AUX_SOURCE_DIRECTORY(./dealers D)


IF(CMAKE_SYSTEM_NAME MATCHES "Windows")
    ADD_LIBRARY(networkinterface SHARED   ${A}  ${B} ${C} ${D} ${CUR_HEADER} ${COMMN_HEADER})
    set_target_properties(networkinterface  PROPERTIES VS_GLOBAL_VcpkgEnabled true)
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Linux")
    add_compile_options(-fcoroutines)
    ADD_LIBRARY(networkinterface  ${A} ${B} ${C} ${D})
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    ADD_LIBRARY(networkinterface  ${A} ${B} ${C} ${D})
ENDIF(CMAKE_SYSTEM_NAME MATCHES "Windows")

target_link_libraries(networkinterface  PRIVATE ZLIB::ZLIB PRIVATE nng::nngpp    PRIVATE Utils PRIVATE configs PRIVATE hotupdate PRIVATE nlohmann_json::nlohmann_json PRIVATE nng::nng PRIVATE spdlog::spdlog PRIVATE OpenSSL::SSL OpenSSL::Crypto)


generate_export_header(networkinterface EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/../inc/os/${CMAKE_SYSTEM_NAME}/export_flags/${PROJECT_NAME}Export.h)

