
PROJECT(demo)
IF(CMAKE_SYSTEM_NAME MATCHES "Windows")
    add_definitions(-D NN_HAVE_WINDOWS)
    add_definitions(-D _CRT_SECURE_NO_WARNINGS)
    link_libraries(Ws2_32 Synchronization )#sqlpp11-connector-postgresql)
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Linux")
    link_directories(/home/ubuntu/vcpkg/installed/x64-linux/lib)
    link_libraries(backtrace ssl crypto z uring  sqlite3)#  sqlpp11-connector-postgresql sqlpp11-connector-postgresql pq)
ENDIF(CMAKE_SYSTEM_NAME MATCHES "Windows")

include_directories(../configs ../Utils ../hotupdate ../database ../networkinterface ../inc)

AUX_SOURCE_DIRECTORY(. A)



add_executable(demo ${A} )

target_link_libraries(demo PRIVATE whereami PRIVATE date::date date::date-tz PRIVATE spdlog::spdlog PRIVATE hashlibrary PRIVATE ZLIB::ZLIB PRIVATE OpenSSL::SSL PRIVATE OpenSSL::Crypto PRIVATE $<IF:$<TARGET_EXISTS:uv_a>,uv_a,uv>  PRIVATE Utils  PRIVATE configs PRIVATE database PRIVATE hotupdate PRIVATE networkinterface PRIVATE Taskflow::Taskflow PRIVATE nng::nng PRIVATE fmt::fmt PRIVATE efsw::efsw)


set_target_properties(demo PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/config")

set_target_properties(demo PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
set_target_properties(demo PROPERTIES VS_GLOBAL_VcpkgEnabled true)
