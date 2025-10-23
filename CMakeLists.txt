cmake_minimum_required(VERSION 3.24)


# Set project name
project(compgpt)

##
## setup compilation flags

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG=1")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb -march=native")

add_compile_options(-Wall -Wextra -pthread)

include_directories("${CMAKE_SOURCE_DIR}/include")


##
## setup PThreads

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

##
## setup BOOST

# note, consider using remodeling this after https://github.com/boostorg/cmake

include(ExternalProject)

# Download and configure Boost
ExternalProject_Add(
    boost_external
    URL https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.tar.xz
    PREFIX ${CMAKE_BINARY_DIR}/boost
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
    LOG_UPDATE ON
    LOG_CONFIGURE ON
    LOG_BUILD ON
)

# Get Boost source directory
ExternalProject_Get_Property(boost_external source_dir)
set(BOOST_ROOT ${source_dir})
set(BOOST_INCLUDE_DIR 
  ${source_dir}/libs/algorithm/include
  ${source_dir}/libs/align/include
  ${source_dir}/libs/asio/include
  ${source_dir}/libs/assert/include
  ${source_dir}/libs/bind/include
  ${source_dir}/libs/concept_check/include/ 
  ${source_dir}/libs/config/include/ 
  ${source_dir}/libs/container/include/ 
  ${source_dir}/libs/container_hash/include/ 
  ${source_dir}/libs/core/include/ 
  ${source_dir}/libs/date_time/include
  ${source_dir}/libs/describe/include
  ${source_dir}/libs/detail/include
  ${source_dir}/libs/filesystem/include
  ${source_dir}/libs/function/include
  ${source_dir}/libs/functional/include
  ${source_dir}/libs/fusion/include
  ${source_dir}/libs/integer/include
  ${source_dir}/libs/intrusive/include
  ${source_dir}/libs/io/include
  ${source_dir}/libs/iterator/include
  ${source_dir}/libs/json/include
  ${source_dir}/libs/lexical_cast/include
  ${source_dir}/libs/move/include
  ${source_dir}/libs/mp11/include
  ${source_dir}/libs/mpl/include
  ${source_dir}/libs/numeric/conversion/include
  ${source_dir}/libs/optional/include
  ${source_dir}/libs/predef/include
  ${source_dir}/libs/preprocessor/include
  ${source_dir}/libs/process/include
  ${source_dir}/libs/range/include
  ${source_dir}/libs/smart_ptr/include
  ${source_dir}/libs/static_assert/include
  ${source_dir}/libs/system/include
  ${source_dir}/libs/tokenizer/include
  ${source_dir}/libs/throw_exception/include
  ${source_dir}/libs/type_traits/include
  ${source_dir}/libs/type_index/include
  ${source_dir}/libs/utility/include
  ${source_dir}/libs/variant2/include
  ${source_dir}/libs/winapi/include
)

# for program_options -- not link-only
# ${source_dir}/libs/any/include
# ${source_dir}/libs/program_options/include


##
## Target: libllmtools.a

add_library(llmtools STATIC src/llmtools.cc)

target_include_directories(llmtools PRIVATE ${BOOST_INCLUDE_DIR})

add_dependencies(llmtools boost_external)


##
## Target: compgpt.bin

# Add executable using the source file from dependencies
add_executable(compgpt.bin src/compgpt.cc)

# Set include directories including Boost headers
target_include_directories(compgpt.bin PRIVATE ${BOOST_INCLUDE_DIR})
target_link_libraries(compgpt.bin PRIVATE Threads::Threads)
target_link_libraries(compgpt.bin PRIVATE llmtools)

# Make sure Boost is downloaded before building the executable
add_dependencies(compgpt.bin boost_external)


##
## Target: logfilter.bin

# Add executable using the source file from dependencies
add_executable(logfilter.bin src/logfilter.cc)

# Set include directories including Boost headers
target_include_directories(logfilter.bin PRIVATE ${BOOST_INCLUDE_DIR})

# Make sure Boost is downloaded before building the executable
add_dependencies(logfilter.bin boost_external)

##
## Target: prettyjson.bin

# Add executable using the source file from dependencies
add_executable(prettyjson.bin src/prettyjson.cc)

target_link_libraries(prettyjson.bin PRIVATE llmtools)

# Set include directories including Boost headers
target_include_directories(prettyjson.bin PRIVATE ${BOOST_INCLUDE_DIR})

# Make sure Boost is downloaded before building the executable
add_dependencies(prettyjson.bin boost_external)



##
## Target: prettyjson.bin

# Add executable using the source file from dependencies
add_executable(code-assist.bin src/code-assist.cc)

target_link_libraries(code-assist.bin PRIVATE llmtools)

# Set include directories including Boost headers
target_include_directories(code-assist.bin PRIVATE ${BOOST_INCLUDE_DIR})

# Make sure Boost is downloaded before building the executable
add_dependencies(code-assist.bin boost_external)



##
## Target: test-llmtools.bin

# Add executable using the source file from dependencies
#add_executable(test-llmtools.bin src/test-llmtools.cc)

#target_link_libraries(test-llmtools.bin PRIVATE llmtools)

# Set include directories including Boost headers
#target_include_directories(test-llmtools.bin PRIVATE ${BOOST_INCLUDE_DIR})

# Make sure Boost is downloaded before building the executable
#add_dependencies(test-llmtools.bin boost_external)

