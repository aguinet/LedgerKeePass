add_executable(set
  set.cpp)
target_link_libraries(set PRIVATE kpl)

add_executable(get
  get.cpp)
target_link_libraries(get PRIVATE kpl)

add_executable(derive
  derive.cpp)
target_link_libraries(derive PRIVATE kpl)

add_executable(get_slots
  get_slots.cpp)
target_link_libraries(get_slots PRIVATE kpl)
