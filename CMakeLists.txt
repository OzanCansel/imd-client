cmake_minimum_required( VERSION 3.10 )
project( imd )

find_package(
    Boost

    COMPONENTS
    thread

    REQUIRED
)

add_library( imd INTERFACE )
add_library( imd::imd ALIAS imd )

target_include_directories(
    imd
    
    INTERFACE
    
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:src>
)

target_link_libraries(
    imd

    INTERFACE

    Boost::thread
)

target_compile_features(
    imd
    INTERFACE
    cxx_std_17
)

include( GNUInstallDirs )

install(
    TARGETS imd
    EXPORT imd-targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/imd
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

include( CMakePackageConfigHelpers )

install(
    EXPORT imd-targets
    FILE imd-targets.cmake
    NAMESPACE imd::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/imd
)

configure_package_config_file(
    ${PROJECT_SOURCE_DIR}/cmake/imd-config.cmake.in
    ${CMAKE_BINARY_DIR}/cmake/imd-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/imd
)

write_basic_package_version_file(
    ${CMAKE_BINARY_DIR}/cmake/imd-config-version.cmake
    VERSION 3.1
    COMPATIBILITY AnyNewerVersion
)

install(
    FILES
    ${CMAKE_BINARY_DIR}/cmake/imd-config.cmake
    ${CMAKE_BINARY_DIR}/cmake/imd-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/imd
)

export( EXPORT imd-targets
    FILE ${CMAKE_BINARY_DIR}/cmake/imd-targets.cmake
    NAMESPACE imd::
)

add_subdirectory( example )

enable_testing()

add_subdirectory( test )