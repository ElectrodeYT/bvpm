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