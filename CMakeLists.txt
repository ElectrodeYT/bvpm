cmake_minimum_required(VERSION 3.23)
project(bvpm)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")
add_executable(bvpm
        main.cpp
        InstallEngine.cpp
        DependencyEngine.cpp
        config.cpp
        )
target_include_directories(bvpm PUBLIC include)
target_link_libraries(bvpm PUBLIC archive)

add_custom_target(create_bvpm_package ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/package/root/usr/sbin/
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/package/root/etc/bvpm/packages
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:bvpm> ${CMAKE_CURRENT_BINARY_DIR}/package/root/usr/sbin/bvpm
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/bvpm.cfg ${CMAKE_CURRENT_BINARY_DIR}/package/root/etc/bvpm/bvpm.cfg
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/manifest ${CMAKE_CURRENT_BINARY_DIR}/package/manifest
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/owned-files ${CMAKE_CURRENT_BINARY_DIR}/package/owned-files
        COMMAND ${CMAKE_COMMAND} -E tar "cfv" "../bvpm.bvp" "*"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/package"
        DEPENDS bvpm
        )
