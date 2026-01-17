add_subdirectory(basic_sync)
add_subdirectory(async_exception_safety)
add_subdirectory(singleton_pattern)
add_subdirectory(pointer_ownership)
add_subdirectory(multi_publisher_single_subscriber)
add_subdirectory(sync_multi_subscribers)

add_custom_target(build_all_examples
    DEPENDS
        basic_sync
        async_exception_safety
        singleton_pattern
        pointer_ownership
        multi_publisher_single_subscriber
        sync_multi_subscribers
    COMMENT "Building all EventBus examples"
)
