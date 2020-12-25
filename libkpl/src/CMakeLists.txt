find_package(sodium REQUIRED)
find_package(HIDAPI REQUIRED)

add_library(kpl
  errors.cpp
  ledger_answer.cpp
  ledger_client.cpp
  ledger_device.cpp
  ledger_device_tcp.cpp
  ledger_device_usb.cpp
  kpl.cpp
  version.cpp
)

target_link_libraries(kpl PRIVATE sodium HIDAPI::hidapi)

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
