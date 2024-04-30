cmake_minimum_required(VERSION 3.28)
PROJECT(whereami)

set(CMAKE_CXX_STANDARD 20)

add_definitions(-DBUILD_SHARED_LIBS -Dwhereami_EXPORTS)

include_directories()

FILE(GLOB_RECURSE CUR_HEADER "./*.h")
source_group("IncCur" FILES ${CUR_HEADER})

FILE(GLOB_RECURSE COMMN_HEADER "../inc/*.h")
source_group("IncComm" FILES ${COMMN_HEADER})

#FILE(GLOB_RECURSE MY_INTERFACEHEADERS "../../inc/networks/*.h")
#source_group("Headers" FILES ${MY_INTERFACEHEADERS})



include_directories(../conf ./src ../inc ../Utils ../hotupdate ../networkinterface ../configs)

AUX_SOURCE_DIRECTORY(./src A)

include(GenerateExportHeader)

IF(CMAKE_SYSTEM_NAME MATCHES "Windows")
    ADD_LIBRARY(whereami SHARED  ${A} ${CUR_HEADER} ${COMMN_HEADER})
    set_target_properties(whereami  PROPERTIES VS_GLOBAL_VcpkgEnabled true)
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Linux")
    add_definitions(-w)
    add_compile_options(-fcoroutines)
    link_libraries(dl)
    ADD_LIBRARY(whereami  ${A} )
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    ADD_LIBRARY(whereami  ${A} )
ENDIF(CMAKE_SYSTEM_NAME MATCHES "Windows")



generate_export_header(whereami EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/../inc/os/${CMAKE_SYSTEM_NAME}/export_flags/${PROJECT_NAME}Export.h)



#target_link_libraries(whereami PRIVATE nng::nngpp  PRIVATE ZLIB::ZLIB PRIVATE OpenSSL::SSL OpenSSL::Crypto PRIVATE unofficial::sqlite3::sqlite3 PRIVATE sqlpp11::sqlpp11 PRIVATE hashlibrary    PRIVATE Utils PRIVATE configs PRIVATE networkinterface PRIVATE spdlog::spdlog PRIVATE hotupdate PRIVATE date::date date::date-tz)



