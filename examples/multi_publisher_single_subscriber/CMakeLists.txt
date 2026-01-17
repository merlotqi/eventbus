add_executable(multi_publisher_single_subscriber
    main.cpp
    ../sync_eventbus_singleton.cpp
    ../sync_eventbus_singleton.h
)

target_link_libraries(multi_publisher_single_subscriber eventbus)
