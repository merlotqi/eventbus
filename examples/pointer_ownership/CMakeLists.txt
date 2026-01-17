add_executable(pointer_ownership
    main.cpp
    ../sync_eventbus_pointer_singleton.cpp
    ../sync_eventbus_pointer_singleton.h
)

target_link_libraries(pointer_ownership eventbus)
