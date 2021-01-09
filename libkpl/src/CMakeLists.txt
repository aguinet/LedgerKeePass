find_package(HIDAPI REQUIRED) 
# WARNING: cmake/Findsodium.cmake breaks something under Windows, which makes
# FindHIDAPI.cmake to return a .dll instead of a .lib file for the hidapi library
# file. We find sodium at the end waiting for a proper fix.
find_package(sodium REQUIRED)

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

if (NOT BUILD_SHARED_LIBS)
  target_compile_definitions(kpl PUBLIC kpl_STATIC)
endif()

target_link_libraries(kpl PRIVATE sodium HIDAPI::hidapi)
if (WIN32)
  target_compile_definitions(kpl PRIVATE _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
  target_link_libraries(kpl PRIVATE ws2_32)
endif()
set_target_properties(kpl PROPERTIES
  C_VISIBILITY_PRESET hidden
  CXX_VISIBILITY_PRESET hidden
  VISIBILITY_INLINES_HIDDEN ON
  POSITION_INDEPENDENT_CODE ON
)

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

if (KPL_BUILD_C_API)
  add_subdirectory(c_api)
endif()
