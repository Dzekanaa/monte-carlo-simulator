# monte-carlo-simulator/CMakeLists.txt
cmake_minimum_required(VERSION 3.15)
project(monte_carlo_simulator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Pronađi TBB
find_package(TBB REQUIRED)

# Dodaj slot-engine kao subdirectory
add_subdirectory(slot-engine)

# Glavni izvršni fajl
add_executable(monte_carlo_simulator
    src/main.cpp
    src/monte_carlo_simulator.cpp
    src/statistics.cpp
)

# Linkovanje
target_link_libraries(monte_carlo_simulator
    PRIVATE
    slot_engine
    TBB::tbb
)

# Include direktorijumi
target_include_directories(monte_carlo_simulator
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/slot-engine/include
)
