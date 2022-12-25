
PROJECT(sqlppTest)
IF(CMAKE_SYSTEM_NAME MATCHES "Windows")
    add_definitions(-D _CRT_SECURE_NO_WARNINGS)
    AUX_SOURCE_DIRECTORY(./win B)
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Linux")
    AUX_SOURCE_DIRECTORY(./linux B)
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    AUX_SOURCE_DIRECTORY(./osx B)
ENDIF(CMAKE_SYSTEM_NAME MATCHES "Windows")

include_directories(../../configs)

AUX_SOURCE_DIRECTORY(. A)



add_executable(sqlppTest ${A} ${B})

target_link_libraries(sqlppTest PRIVATE unofficial::sqlite3::sqlite3 PRIVATE OpenSSL::SSL OpenSSL::Crypto PRIVATE sqlpp11::sqlpp11 PRIVATE database PRIVATE Catch2::Catch2 Catch2::Catch2WithMain PRIVATE hashlibrary    PRIVATE Utils PRIVATE configs PRIVATE networkinterface PRIVATE spdlog::spdlog PRIVATE hotupdate)


set_target_properties(sqlppTest PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/config")

set_target_properties(sqlppTest PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
set_target_properties(sqlppTest PROPERTIES VS_GLOBAL_VcpkgEnabled true)
