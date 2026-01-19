cmake_minimum_required(VERSION 3.10...3.24)
project(eventbus VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(EVENTBUS_BUILD_EXAMPLES "Build EventBus examples" OFF)
option(EVENTBUS_BUILD_DOCS "Build EventBus documentation with Doxygen" OFF)

add_library(eventbus INTERFACE)

target_include_directories(eventbus INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(eventbus INTERFACE cxx_std_20)

include(GNUInstallDirs)

install(TARGETS eventbus
    EXPORT eventbusTargets
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT eventbusTargets
    FILE eventbusTargets.cmake
    NAMESPACE eventbus::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/eventbus
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/eventbusConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/eventbusConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/eventbusConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/eventbus
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/eventbusConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/eventbusConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/eventbus
)

if(EVENTBUS_BUILD_DOCS)
    # Doxygen documentation
    find_package(Doxygen OPTIONAL_COMPONENTS dot)
    if(Doxygen_FOUND)
        set(DOXYGEN_PROJECT_NAME "EventBus")
        set(DOXYGEN_PROJECT_VERSION ${PROJECT_VERSION})
        set(DOXYGEN_PROJECT_DESCRIPTION "A modern C++20 event bus library")
        set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/docs)
        set(DOXYGEN_GENERATE_HTML YES)
        set(DOXYGEN_GENERATE_LATEX NO)
        set(DOXYGEN_GENERATE_MAN NO)
        set(DOXYGEN_GENERATE_XML NO)
        set(DOXYGEN_WARN_AS_ERROR YES)
        set(DOXYGEN_EXTRACT_ALL YES)
        set(DOXYGEN_EXTRACT_PRIVATE NO)
        set(DOXYGEN_EXTRACT_STATIC NO)
        set(DOXYGEN_RECURSIVE YES)
        set(DOXYGEN_QUIET YES)

        doxygen_add_docs(eventbus_docs
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            COMMENT "Generate HTML documentation"
        )

        # Custom target to build docs
        add_custom_target(docs
            COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target eventbus_docs
            COMMENT "Building documentation with Doxygen"
        )
    endif()
endif()

if(EVENTBUS_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
