# Specify the minimum version for CMake
cmake_minimum_required(VERSION 3.5)

# Message
message(STATUS "Transistor Placement")
message(STATUS ${CMAKE_CURRENT_SOURCE_DIR})
message(STATUS ${PROJECT_SOURCE_DIR})

# Find includes in the build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Set CC and CXX
execute_process(COMMAND which gcc OUTPUT_VARIABLE CC OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND which g++ OUTPUT_VARIABLE CXX OUTPUT_STRIP_TRAILING_WHITESPACE)

set(CMAKE_C_COMPILER ${CC})
set(CMAKE_CXX_COMPILER ${CXX})

# Project's name
project(transistorplacement)

# Z3 SMT prover - Temporarily disabled for testing
# set ( Z3_DIR ./z3/build )
# find_package(Z3
#   REQUIRED
#   CONFIG
#   # `NO_DEFAULT_PATH` is set so that -DZ3_DIR has to be passed to find Z3.
#   # This should prevent us from accidentally picking up an installed
#   # copy of Z3. This is here to benefit Z3's build system when building
#   # this project. When making your own project you probably shouldn't
#   # use this option.
#   NO_DEFAULT_PATH
# )
# message(STATUS "Z3_FOUND: ${Z3_FOUND}")
# message(STATUS "Found Z3 ${Z3_VERSION_STRING}")
# message(STATUS "Z3_DIR: ${Z3_DIR}")

# build configuration - Debug|Release|RelWithDebInfo|MinSizeRel
set ( CMAKE_BUILD_TYPE Debug )

# Set the output folder where your program will be created
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(CMAKE_CXX_STANDARD 17)

set(PATH_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Warnings/Errors

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=return-type")

file(GLOB_RECURSE SRC_FILES_
	${PATH_SRC}/*.cpp
	${PATH_SRC}/*.c)

# Exclude Router.cpp to avoid Z3 dependency
list(REMOVE_ITEM SRC_FILES_ "${PATH_SRC}/Router.cpp")
# Use simplified main.cpp
list(REMOVE_ITEM SRC_FILES_ "${PATH_SRC}/main.cpp")
list(APPEND SRC_FILES_ "${PATH_SRC}/main_simple.cpp")

set(SRC_FILES ${SRC_FILES_})

# Create SRC_FILES for width-only version
file(GLOB_RECURSE SRC_FILES_WIDTH_
	${PATH_SRC}/*.cpp
	${PATH_SRC}/*.c)

# Exclude Router.cpp and main files for width-only version
list(REMOVE_ITEM SRC_FILES_WIDTH_ "${PATH_SRC}/Router.cpp")
list(REMOVE_ITEM SRC_FILES_WIDTH_ "${PATH_SRC}/main.cpp")
list(REMOVE_ITEM SRC_FILES_WIDTH_ "${PATH_SRC}/main_simple.cpp")
list(APPEND SRC_FILES_WIDTH_ "${PATH_SRC}/main_width_only.cpp")

set(SRC_FILES_WIDTH ${SRC_FILES_WIDTH_})

# Create SRC_FILES for placement-only version
file(GLOB_RECURSE SRC_FILES_PLACEMENT_
	${PATH_SRC}/*.cpp
	${PATH_SRC}/*.c)

# Exclude Router.cpp and main files for placement-only version
list(REMOVE_ITEM SRC_FILES_PLACEMENT_ "${PATH_SRC}/Router.cpp")
list(REMOVE_ITEM SRC_FILES_PLACEMENT_ "${PATH_SRC}/main.cpp")
list(REMOVE_ITEM SRC_FILES_PLACEMENT_ "${PATH_SRC}/main_simple.cpp")
list(REMOVE_ITEM SRC_FILES_PLACEMENT_ "${PATH_SRC}/main_width_only.cpp")
list(APPEND SRC_FILES_PLACEMENT_ "${PATH_SRC}/main_placement_only.cpp")

set(SRC_FILES_PLACEMENT ${SRC_FILES_PLACEMENT_})

# Create SRC_FILES for routing-only version
file(GLOB_RECURSE SRC_FILES_ROUTING_
	${PATH_SRC}/*.cpp
	${PATH_SRC}/*.c)

# Exclude main files for routing-only version
list(REMOVE_ITEM SRC_FILES_ROUTING_ "${PATH_SRC}/main.cpp")
list(REMOVE_ITEM SRC_FILES_ROUTING_ "${PATH_SRC}/main_simple.cpp")
list(REMOVE_ITEM SRC_FILES_ROUTING_ "${PATH_SRC}/main_width_only.cpp")
list(REMOVE_ITEM SRC_FILES_ROUTING_ "${PATH_SRC}/main_placement_only.cpp")
list(APPEND SRC_FILES_ROUTING_ "${PATH_SRC}/main_routing_only.cpp")

set(SRC_FILES_ROUTING ${SRC_FILES_ROUTING_})

# Include Directories
include_directories(headers ${PATH_SRC})
# include_directories(headers ${PATH_SRC} ${Z3_CXX_INCLUDE_DIRS})

# Link library
# link_libraries ( ${Z3_LIBRARIES} )
# link_libraries ( ${Z3_LIBRARIES} "-static")

# Targets
add_executable(placement ${SRC_FILES})
add_executable(width_only ${SRC_FILES_WIDTH})
add_executable(placement_only ${SRC_FILES_PLACEMENT})
add_executable(routing_only ${SRC_FILES_ROUTING})

# for static
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(placement PRIVATE Threads::Threads)
target_link_libraries(width_only PRIVATE Threads::Threads)
target_link_libraries(placement_only PRIVATE Threads::Threads)
target_link_libraries(routing_only PRIVATE Threads::Threads)


# Linker Flags
#set_target_properties(main PROPERTIES LINK_FLAGS "-static -W1,--whole-archive -rdynamic -lpthread -W1,--no-whole-archive")
