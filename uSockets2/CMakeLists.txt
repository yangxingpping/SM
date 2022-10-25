cmake_minimum_required(VERSION 3.10)
PROJECT(uSockets2)

add_definitions(-DLIBUS_NO_SSL)
#set(CMAKE_BUILD_TYPE "Release")
include_directories(src)
AUX_SOURCE_DIRECTORY(./src/ DIR_SRCS)
AUX_SOURCE_DIRECTORY(./src/eventing/ DIR_SRCS2)

IF(CMAKE_SYSTEM_NAME MATCHES "Windows")
    #on windows use libuv
    add_definitions(-DLIBUS_USE_LIBUV)
    ADD_LIBRARY(uSockets2 SHARED ${DIR_SRCS} src/eventing/libuv.c)
    set_target_properties(uSockets2  PROPERTIES VS_GLOBAL_VcpkgEnabled true)
    target_link_libraries(uSockets2 PRIVATE $<IF:$<TARGET_EXISTS:uv_a>,uv_a,uv>)
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Linux")
    #on linux use epoll
    add_definitions(-DLIBUS_USE_EPOLL)
    ADD_LIBRARY(uSockets2  ${DIR_SRCS} src/eventing/epoll_kqueue.c)
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    include_directories(/usr/local/include)
    link_directories(/usr/local/lib)
    link_libraries(uv)
    ADD_LIBRARY(uSockets2 ${DIR_SRCS} ${DIR_SRCS2})
ENDIF(CMAKE_SYSTEM_NAME MATCHES "Windows")




