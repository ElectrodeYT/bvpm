cmake_minimum_required(VERSION 3.18)
project(bvpm)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -Wall -Wextra -Werror -Wno-unused-parameter")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG -Wall -Wextra -Werror -Wno-unused-parameter")

set(BVP_DONT_ADD_DEPENDENCY FALSE CACHE BOOL "Add the dependency section to the bvpm.bvp file (bash, glibc)")

add_executable(bvpm
        main.cpp
        InstallEngine.cpp
        UninstallEngine.cpp
        DependencyEngine.cpp
        config.cpp
        LocalFolderRepository.cpp
        PackageFile.cpp
        RepositoryEngine.cpp
        )
target_include_directories(bvpm PUBLIC include)
target_link_libraries(bvpm PUBLIC archive)

install(TARGETS bvpm DESTINATION "bin")
# Create the bvpm-repo symlink
install(CODE "execute_process( \
            COMMAND ${CMAKE_COMMAND} -E create_symlink \
            bvpm   \
            \$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/bvpm-repo \
            )"
        )

add_custom_target(create_bvpm_package_folder
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/package
        )

if(BVP_DONT_ADD_DEPENDENCY)
add_custom_target(create_bvpm_package ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/package/root/usr/sbin/
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/package/root/etc/bvpm/packages
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:bvpm> ${CMAKE_CURRENT_BINARY_DIR}/package/root/usr/sbin/bvpm
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/bvpm.cfg ${CMAKE_CURRENT_BINARY_DIR}/package/root/etc/bvpm/bvpm.cfg
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/manifest-nodep ${CMAKE_CURRENT_BINARY_DIR}/package/manifest
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/owned-files ${CMAKE_CURRENT_BINARY_DIR}/package/owned-files
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/afterinstall.sh ${CMAKE_CURRENT_BINARY_DIR}/package/afterinstall.sh
        COMMAND ${CMAKE_COMMAND} -E tar "cf" "../bvpm.bvp" "*"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/package"
        DEPENDS bvpm
        DEPENDS create_bvpm_package_folder
        )
else()
add_custom_target(create_bvpm_package ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/package/root/usr/sbin/
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/package/root/etc/bvpm/packages
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:bvpm> ${CMAKE_CURRENT_BINARY_DIR}/package/root/usr/sbin/bvpm
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/bvpm.cfg ${CMAKE_CURRENT_BINARY_DIR}/package/root/etc/bvpm/bvpm.cfg
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/manifest ${CMAKE_CURRENT_BINARY_DIR}/package/manifest
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/owned-files ${CMAKE_CURRENT_BINARY_DIR}/package/owned-files
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/afterinstall.sh ${CMAKE_CURRENT_BINARY_DIR}/package/afterinstall.sh
        COMMAND ${CMAKE_COMMAND} -E tar "cf" "../bvpm.bvp" "*"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/package"
        DEPENDS bvpm
        DEPENDS create_bvpm_package_folder
        )
endif()

