cmake_minimum_required(VERSION 3.16)
project(monte_carlo_simulator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Pronađi TBB
find_package(TBB REQUIRED)

# Dodaj slot-engine submodule
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/slot-engine/CMakeLists.txt)
    add_subdirectory(slot-engine)
else()
    message(FATAL_ERROR "slot-engine submodule not found. Run: git submodule update --init --recursive")
endif()

# Glavni izvršni fajl
add_executable(monte_carlo_simulator
    src/main.cpp
    src/monte_carlo_simulator.cpp
    src/statistics.cpp
    src/rtp_calculator.cpp
    src/flow_graph_simulator.cpp
)

# Linkovanje
target_link_libraries(monte_carlo_simulator
    PRIVATE
    slot_engine
    TBB::tbb
)

# Include direktorijumi - OBAVEZNO DODAJ OBA
target_include_directories(monte_carlo_simulator
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/slot-engine/include  # ← OVO JE KLJUČNO!
)
