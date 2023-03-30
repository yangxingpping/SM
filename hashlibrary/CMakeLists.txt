cmake_minimum_required(VERSION 3.22)
PROJECT(hashlibrary)

set(CMAKE_CXX_STANDARD 20)

add_definitions(-DBUILD_SHARED_LIBS -Dhashlibrary_EXPORTS)

#include_directories(../inc/${CMAKE_SYSTEM_NAME}/export_flags)

include(GenerateExportHeader)

FILE(GLOB_RECURSE CUR_HEADER "./*.h")
source_group("IncCur" FILES ${CUR_HEADER})

FILE(GLOB_RECURSE COMMN_HEADER "../inc/*.h")
source_group("IncComm" FILES ${COMMN_HEADER})

#FILE(GLOB_RECURSE MY_INTERFACEHEADERS "../../inc/networks/*.h")
#source_group("Headers" FILES ${MY_INTERFACEHEADERS})

AUX_SOURCE_DIRECTORY(. A)

IF(CMAKE_SYSTEM_NAME MATCHES "Windows")
    ADD_LIBRARY(hashlibrary SHARED  ${A} ${CUR_HEADER} ${COMMN_HEADER})
    set_target_properties(hashlibrary  PROPERTIES VS_GLOBAL_VcpkgEnabled true)
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Linux")
    add_definitions(-w)
    ADD_LIBRARY(hashlibrary  ${A} )
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    ADD_LIBRARY(hashlibrary  ${A} )
ENDIF(CMAKE_SYSTEM_NAME MATCHES "Windows")



generate_export_header(hashlibrary EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/../inc/os/${CMAKE_SYSTEM_NAME}/export_flags/${PROJECT_NAME}Export.h)



#target_link_libraries(HASHLIB    PRIVATE Utils PRIVATE configs PRIVATE networkinterface PRIVATE spdlog::spdlog PRIVATE hotupdate)



