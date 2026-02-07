cmake_minimum_required(VERSION 3.16)
project(CreditRiskFilter LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include_directories(include)
find_package(Boost REQUIRED)
add_executable(credit_risk_filter 
    src/main.cpp
    src/particle_grid/gen_particle_grid.cpp
    src/particle_grid/quasi_random_sequence.cpp
    src/debtors/gen_pd.cpp
    src/debtors/simulate_defaults.cpp
    src/risk_measures/run_python.cpp)
target_link_libraries(credit_risk_filter PRIVATE Boost::boost)

target_compile_definitions(credit_risk_filter PRIVATE 
    PYTHON_SCRIPT_DIR="${CMAKE_BINARY_DIR}")

configure_file(
    ${CMAKE_SOURCE_DIR}/scripts/risk_analysis.py
    ${CMAKE_BINARY_DIR}/risk_analysis.py
    COPYONLY
)

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/data" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/saved_state" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")