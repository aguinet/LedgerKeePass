add_executable(kpl_set
  set.cpp)
target_link_libraries(kpl_set PRIVATE kpl)

add_executable(kpl_get
  get.cpp)
target_link_libraries(kpl_get PRIVATE kpl)

add_executable(kpl_derive
  derive.cpp)
target_link_libraries(kpl_derive PRIVATE kpl)

add_executable(kpl_get_slots
  get_slots.cpp)
target_link_libraries(kpl_get_slots PRIVATE kpl)

add_executable(kpl_list_devices
  list_devices.cpp)
target_link_libraries(kpl_list_devices PRIVATE kpl)
