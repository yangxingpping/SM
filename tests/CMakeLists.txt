
cmake_minimum_required(VERSION 3.20)

project(tests)

include_directories(../networkinterface ../Utils ../hotupdate)

add_subdirectory(envcheck)
add_subdirectory(networktest)
add_subdirectory(asyncStreamtest)
add_subdirectory(asyncEvent)
add_subdirectory(sqlppTest)

add_test (test_run envcheck)

#add_test (test_run envcheck)