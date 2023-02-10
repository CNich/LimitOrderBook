cmake_minimum_required(VERSION 3.10)
project(lib_lob)
enable_language(CXX)

set(CMAKE_CXX_STANDARD 14)

# Add the include directory to the include path
include_directories(include)

# Add the source files to the library
set(SOURCES
    binary_search_tree.hpp
    limit_ob.hpp
    LOB_buildingBlocks.hpp
    main.cpp
    doubly_linked_list.hpp
    LimitTree.hpp
    LOB_buildingBlocks.hpp.gch
    pointercastTest.cpp
    flat_hash_map.hpp
    LimitTree.hpp.gch
    csv_reader.cpp
)

# Create the library
add_library(lib_lob ${SOURCES})
add_executable(main main.cpp)
add_executable(csv_reader csv_reader.cpp)

