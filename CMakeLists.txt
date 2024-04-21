cmake_minimum_required(VERSION 3.22)


set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
  CACHE STRING "Vcpkg toolchain file")

project(libs)

#set(CMAKE_CXX_COMPILER /Users/alqazy/Downloads/clang+llvm-13.0.1-x86_64-apple-darwin/bin/clang)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)



find_package(nng REQUIRED)
find_package(nanomsg REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
find_package(Taskflow CONFIG REQUIRED)
find_package(efsw CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(libuv CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(toml11 CONFIG REQUIRED)
find_package(asio CONFIG REQUIRED)
find_package(Catch2 CONFIG REQUIRED)
find_package(nngpp CONFIG REQUIRED)
find_package(date CONFIG REQUIRED)
find_package(Sqlpp11 CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(msgpack-c CONFIG REQUIRED)
find_package(function2 CONFIG REQUIRED)


add_definitions(-DUSE_SQLITE -DONESHOT_ASIO_STANDALONE -DASIO_HAS_FILE -DNOMINMAX -DSPDLOG_COMPILED_LIB)

include_directories(inc database configs hashlibrary hotupdate inc/os/${CMAKE_SYSTEM_NAME}/export_flags)

IF(CMAKE_SYSTEM_NAME MATCHES "Windows")
  if(PROJECT_IS_TOP_LEVEL)
    set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR})
    set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})
  endif()
    add_definitions(-DOTL_ODBC)
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Linux")
  SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wunused-function -Wunused-local-typedefs  -g -ggdb")
  SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")
  add_definitions(-DASIO_HAS_IO_URING)
  include_directories(/usr/local/include /usr/include/postgresql)
  include_directories("$ENV{VCPKG_ROOT}/installed/x64-linux/include")
  link_directories(/usr/local/lib "$ENV{VCPKG_ROOT}/installed/x64-linux/lib")
  link_libraries(ssl pthread crypto uring z)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fpermissive>)
  add_link_options(-Wl,--no-as-needed)
  add_definitions(-DOTL_ODBC_UNIX -DOS_LINUX)
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Darwin")
  include_directories(/usr/local/include)
  include_directories("$ENV{VCPKG_ROOT}/installed/x64-osx/include")
  link_directories(/usr/local/lib)
  add_definitions(-DOS_DARWIN)
  set(CMAKE_CXX_FLAGS"${CMAKE_CXX_FLAGS}  -Wno-unused-value -pedantic")
ENDIF(CMAKE_SYSTEM_NAME MATCHES "Windows")

include(GenerateExportHeader)

add_subdirectory(configs)
add_subdirectory(database)
add_subdirectory(networkinterface)
add_subdirectory(hotupdate)
add_subdirectory(Utils)
add_subdirectory(rayX)
add_subdirectory(whereami)
#add_subdirectory(endian)

#add_subdirectory(uSockets2)
add_subdirectory(hashlibrary)

if(PROJECT_IS_TOP_LEVEL)
    add_subdirectory(demo)
    add_subdirectory(tests)
endif()

enable_testing()

