
PROJECT(rayXtest)
IF(CMAKE_SYSTEM_NAME MATCHES "Windows")
    add_definitions(-D _CRT_SECURE_NO_WARNINGS)
    AUX_SOURCE_DIRECTORY(./win B)
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Linux")
    AUX_SOURCE_DIRECTORY(./linux B)
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    AUX_SOURCE_DIRECTORY(./osx B)
ENDIF(CMAKE_SYSTEM_NAME MATCHES "Windows")



AUX_SOURCE_DIRECTORY(. A)



add_executable(rayXtest ${A} ${B})

target_link_libraries(rayXtest PRIVATE function2::function2 PRIVATE rayX PRIVATE Utils PRIVATE nng::nng PRIVATE hotupdate PRIVATE OpenSSL::SSL OpenSSL::Crypto PUBLIC Utils PRIVATE Catch2::Catch2 Catch2::Catch2WithMain)


set_target_properties(rayXtest PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/config")

set_target_properties(rayXtest PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
set_target_properties(rayXtest PROPERTIES VS_GLOBAL_VcpkgEnabled true)
