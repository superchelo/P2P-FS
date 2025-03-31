cmake_minimum_required(VERSION 3.10)
project(P2P-FS)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Define the source files
set(SOURCES
    main.cpp
    client.cpp
    server.cpp
)

# Define the header files
set(HEADERS
    client.h
    server.h
)

# Create the executable
add_executable(P2P-FS ${SOURCES} ${HEADERS})

# Link necessary Windows libraries
if (WIN32)
    target_link_libraries(P2P-FS ws2_32)
endif()

# Enable compiler warnings
if (MSVC)
    target_compile_options(P2P-FS PRIVATE /W4)
else()
    target_compile_options(P2P-FS PRIVATE -Wall -Wextra -pedantic)
endif()