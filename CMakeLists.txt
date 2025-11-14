cmake_minimum_required(VERSION 3.26)
project(llmtools)

# Set C++ version
set(CMAKE_CXX_STANDARD 20)

# Set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -O3 -march=native -DNDEBUG=1")

# Set default install prefix if not specified
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "Installation prefix" FORCE)
endif()

# Set RPATH settings
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Try to find Boost - requiring 1.79.0 or higher
find_package(Boost 1.79.0 COMPONENTS program_options filesystem atomic json container QUIET)

# Include directories and library setup
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Set library output path for build
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)

# Define the shared library
add_library(llmtools SHARED src/llmtools.cc)
set_target_properties(llmtools PROPERTIES
    SOVERSION 1
    VERSION 1.0.0
)

# Define binaries
set(BINARIES compgpt logfilter prettyjson code-assist test-llmtools)

# If Boost is not found, download and build it
if(NOT Boost_FOUND)
    include(ExternalProject)
    set(BOOST_VERSION 1.84.0)
    set(BOOST_URL "https://github.com/boostorg/boost/releases/download/boost-${BOOST_VERSION}/boost-${BOOST_VERSION}.tar.xz")
    set(BOOST_INSTALL_DIR "${CMAKE_BINARY_DIR}/boost-install")

    message(STATUS "Boost not found. Will download and build Boost ${BOOST_VERSION}")

    ExternalProject_Add(
        boost_external
        URL ${BOOST_URL}
        URL_HASH SHA256=2e64e5d79a738d0fa6fb546c6e5c2bd28f88d268a2a080546f74e5ff98f29d0e
        PREFIX "${CMAKE_BINARY_DIR}/boost-src"
        CONFIGURE_COMMAND ./bootstrap.sh --prefix=${BOOST_INSTALL_DIR} --with-libraries=program_options,filesystem,atomic,json,container,system
        BUILD_COMMAND ./b2 install --prefix=${BOOST_INSTALL_DIR} link=shared threading=multi variant=release
        BUILD_IN_SOURCE 1
        INSTALL_COMMAND ""
    )

    # Set paths for the built boost
    set(BOOST_ROOT ${BOOST_INSTALL_DIR})

    # Add include directory to our targets
    target_include_directories(llmtools PRIVATE "${BOOST_INSTALL_DIR}/include")

    # Add dependencies
    add_dependencies(llmtools boost_external)

    # Setup link flags for the library
    target_link_directories(llmtools PRIVATE "${BOOST_INSTALL_DIR}/lib")
    target_link_libraries(llmtools
        -lboost_program_options
        -lboost_filesystem
        -lboost_atomic
        -lboost_json
        -lboost_container
        -lboost_system
        dl
    )

    # Setup executables
    foreach(binary ${BINARIES})
        add_executable(${binary} src/${binary}.cc)
        target_include_directories(${binary} PRIVATE "${BOOST_INSTALL_DIR}/include")
        target_link_directories(${binary} PRIVATE "${BOOST_INSTALL_DIR}/lib" "${CMAKE_CURRENT_SOURCE_DIR}/lib")
        target_link_libraries(${binary}
            llmtools
            -lboost_program_options
            -lboost_filesystem
            -lboost_atomic
            -lboost_json
            -lboost_container
            -lboost_system
            pthread
        )
        add_dependencies(${binary} boost_external)
        set_target_properties(${binary} PROPERTIES
            OUTPUT_NAME "${binary}.bin"
        )
    endforeach()

    # For development use (not installed), set RPATH to find the libraries
    set_target_properties(llmtools PROPERTIES INSTALL_RPATH "${BOOST_INSTALL_DIR}/lib")
    foreach(binary ${BINARIES})
        set_target_properties(${binary} PROPERTIES INSTALL_RPATH "${BOOST_INSTALL_DIR}/lib;${CMAKE_CURRENT_SOURCE_DIR}/lib")
    endforeach()

    # Also install the Boost libraries we downloaded
    install(DIRECTORY ${BOOST_INSTALL_DIR}/lib/
            DESTINATION lib
            FILES_MATCHING PATTERN "*.so*")

else()
    # Using system Boost
    include_directories(${Boost_INCLUDE_DIRS})

    target_link_libraries(llmtools
        ${Boost_PROGRAM_OPTIONS_LIBRARY}
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_ATOMIC_LIBRARY}
        ${Boost_JSON_LIBRARY}
        ${Boost_CONTAINER_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        dl
    )

    # Setup executables with system Boost
    foreach(binary ${BINARIES})
        add_executable(${binary} src/${binary}.cc)
        target_link_libraries(${binary}
            llmtools
            ${Boost_PROGRAM_OPTIONS_LIBRARY}
            ${Boost_FILESYSTEM_LIBRARY}
            ${Boost_ATOMIC_LIBRARY}
            ${Boost_JSON_LIBRARY}
            ${Boost_CONTAINER_LIBRARY}
            ${Boost_SYSTEM_LIBRARY}
            pthread
        )
        set_target_properties(${binary} PROPERTIES
            OUTPUT_NAME "${binary}.bin"
        )
    endforeach()

    # For development use (not installed), set RPATH to find the libraries
    set_target_properties(llmtools PROPERTIES INSTALL_RPATH "${Boost_LIBRARY_DIRS}")
    foreach(binary ${BINARIES})
        set_target_properties(${binary} PROPERTIES INSTALL_RPATH "${Boost_LIBRARY_DIRS};${CMAKE_CURRENT_SOURCE_DIR}/lib")
    endforeach()
endif()

include(GNUInstallDirs)

# Install headers, library and executables
install(DIRECTORY include/ DESTINATION include)
install(TARGETS llmtools
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

foreach(binary ${BINARIES})
    install(TARGETS ${binary}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endforeach()

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/etc/llmtools/"
        DESTINATION "etc/llmtools")

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
        DESTINATION "include")



# Installation message
install(CODE "message(STATUS \"Installation complete.\")")
