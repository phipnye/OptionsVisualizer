cmake_minimum_required(VERSION 3.22)
project(PricingModule VERSION 1.0.3 LANGUAGES CXX)

# --- Set the standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- Define strict compiler flags
add_library(StrictFlags INTERFACE)
target_compile_options(StrictFlags INTERFACE
    "-Wall"
    "-Wextra"
    "-Wshadow"
    "-Weffc++"
    "-Wconversion"
    "-Wno-error=sign-compare"
    "-pedantic-errors"
    "-Werror"
)

# --- Setup pybind module
set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 CONFIG REQUIRED)
pybind11_add_module(pricing 
    src/grid/index.cpp
    src/module.cpp
    src/greeks/trinomial/calculate_greeks.cpp
    src/greeks/trinomial/details/calculate_greeks.cpp
    src/greeks/trinomial/details/PricingParams.cpp
    src/greeks/trinomial/details/helpers.cpp
    src/greeks/trinomial/details/american_price.cpp
    src/greeks/GreeksResult.cpp
    src/greeks/bsm/calculate_greeks.cpp
)
install(TARGETS pricing DESTINATION .)

# Link the strict compiler flags
target_link_libraries(pricing PRIVATE StrictFlags)

# Apply Debug/Optimization and Sanitizers
set(SANITIZER_FLAGS 
    -fsanitize=address
    -fsanitize=undefined
    -fsanitize=leak
    -fsanitize=bounds
    -fsanitize=integer-divide-by-zero
    -fsanitize=float-divide-by-zero
    -fsanitize=null
)

# Apply sanitizer flags to the compiler
target_compile_options(pricing PRIVATE 
    # Debug info and no optimization
    $<$<CONFIG:Debug>:-O0 -g>

    # Sanitizers
    $<$<CONFIG:Debug>:${SANITIZER_FLAGS}>
)

# Apply Sanitizer flags to the linker to link the runtime libraries
target_link_options(pricing PRIVATE 
    $<$<CONFIG:Debug>:${SANITIZER_FLAGS}>
)

#--- Dependencies
# Include directories
target_include_directories(pricing PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Find and link against libtorch
find_package(Torch REQUIRED)
target_link_libraries(pricing PRIVATE ${TORCH_LIBRARIES})

# Link against pybind
target_link_libraries(pricing PRIVATE pybind11::module)

# --- Add Test Directory

# This line tells CMake to process the CMakeLists.txt inside the tests/ folder
add_subdirectory(tests)
