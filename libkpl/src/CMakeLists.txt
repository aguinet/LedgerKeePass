find_package(sodium REQUIRED)

add_library(kpl
  dongleComm.c
  dongleCommHid.c
  dongleCommHidHidapi.c
  dongleCommWinUSB.c
  dongleCommProxy.c
  kpl.cpp
  version.cpp
)

target_link_libraries(kpl PRIVATE sodium)

target_include_directories(kpl PUBLIC
  $<INSTALL_INTERFACE:include>
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)


install(TARGETS kpl
  EXPORT kpl-target
  DESTINATION "lib")

# End-user CMake
install(EXPORT kpl-target
  DESTINATION lib/cmake
)
