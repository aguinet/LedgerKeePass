cmake_minimum_required(VERSION 3.10)
project(keepass_ledger)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

install(DIRECTORY include/kpl DESTINATION include)
install(FILES
  cmake/kpl-config.cmake
  cmake/Findsodium.cmake
  cmake/FindHIDAPI.cmake
  DESTINATION lib/cmake)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_subdirectory(src)
add_subdirectory(tools)
