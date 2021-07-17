cmake_minimum_required(VERSION 3.19)
project(lexertk VERSION 0.2.0)

set(CMAKE_CXX_STANDARD 17)

option(BUILD_EXAMPLES "Enable example" ON)

find_package(fmt REQUIRED)

include(GNUInstallDirs)

add_subdirectory(lexertk)

if (BUILD_EXAMPLES)
    find_package(benchmark REQUIRED)
    add_subdirectory(example)
endif ()

include(CMakePackageConfigHelpers)
write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}ConfigVersion.cmake" COMPATIBILITY SameMajorVersion)

install(EXPORT ${CMAKE_PROJECT_NAME}Targets
        NAMESPACE ${CMAKE_PROJECT_NAME}::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/lexertk
        COMPONENT dev)

install(FILES cmake/${CMAKE_PROJECT_NAME}Config.cmake "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/lexertk
        COMPONENT dev
        )
