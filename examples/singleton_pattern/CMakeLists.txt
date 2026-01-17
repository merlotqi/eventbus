add_executable(singleton_pattern 
  main.cpp 
  ../sync_eventbus_singleton.cpp)

target_link_libraries(singleton_pattern eventbus)
