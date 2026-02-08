add_subdirectory(basic_sync)
add_subdirectory(async_exception_safety)
add_subdirectory(singleton_pattern)
add_subdirectory(pointer_ownership)
add_subdirectory(multi_publisher_single_subscriber)
add_subdirectory(sync_multi_subscribers)
add_subdirectory(template_specialization)
add_subdirectory(manual_unsubscribe)

add_custom_target(build_all_examples
    DEPENDS
        basic_sync
        async_exception_safety
        singleton_pattern
        pointer_ownership
        multi_publisher_single_subscriber
        sync_multi_subscribers
        manual_unsubscribe
    COMMENT "Building all EventBus examples"
)
