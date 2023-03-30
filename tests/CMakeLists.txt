
cmake_minimum_required(VERSION 3.20)

project(tests)

include_directories(../networkinterface ../Utils ../hotupdate ../rayX)

add_subdirectory(envcheck)
add_subdirectory(networktest)
add_subdirectory(asyncStreamtest)
add_subdirectory(asyncEvent)
add_subdirectory(sqlppTest)
add_subdirectory(rayXtest)

add_test (test_run envcheck)

#add_test (test_run envcheck)