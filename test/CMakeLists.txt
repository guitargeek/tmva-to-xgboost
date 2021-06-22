cmake_minimum_required(VERSION 3.2)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

project(GBRForest)
set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCE_FILES src/*)

include_directories(include)

add_library (gbrforest SHARED ${SOURCE_FILES})

file(GLOB_RECURSE HEADER_FILES "include/*.h")
set_target_properties(gbrforest PROPERTIES PUBLIC_HEADER "${HEADER_FILES}")
