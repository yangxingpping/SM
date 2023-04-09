
PROJECT(asyncEvent)
IF(CMAKE_SYSTEM_NAME MATCHES "Windows")
    add_definitions(-D _CRT_SECURE_NO_WARNINGS)
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Linux")
   
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    
ENDIF(CMAKE_SYSTEM_NAME MATCHES "Windows")



AUX_SOURCE_DIRECTORY(. A)



add_executable(asyncEvent ${A})

target_link_libraries(asyncEvent PRIVATE Catch2::Catch2 Catch2::Catch2WithMain PRIVATE hashlibrary PRIVATE database PRIVATE Utils PRIVATE configs PRIVATE networkinterface PRIVATE spdlog::spdlog PRIVATE hotupdate)


set_target_properties(asyncEvent PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/config")

set_target_properties(asyncEvent PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
set_target_properties(asyncEvent PROPERTIES VS_GLOBAL_VcpkgEnabled true)
