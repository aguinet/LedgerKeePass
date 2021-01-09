set(KPL_C_SRCS
  ledger_device_c.cpp
  kpl_c.cpp
)
if (KPL_C_API_SHARED)
  add_library(kpl_c SHARED ${KPL_C_SRCS})
else()
  add_library(kpl_c ${KPL_C_SRCS})
  if (NOT BUILD_SHARED_LIBS)
    target_compile_definitions(kpl_c PUBLIC kpl_c_STATIC)
  endif()
endif()

target_link_libraries(kpl_c PRIVATE kpl)

set_target_properties(kpl_c PROPERTIES
  C_VISIBILITY_PRESET hidden
  CXX_VISIBILITY_PRESET hidden
  VISIBILITY_INLINES_HIDDEN ON
  POSITION_INDEPENDENT_CODE ON
)

install(TARGETS kpl_c
  EXPORT kpl_c-target
  DESTINATION "lib")

# End-user CMake
install(EXPORT kpl_c-target
  DESTINATION lib/cmake
)
