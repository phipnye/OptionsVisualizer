cmake_minimum_required(VERSION 3.22)
project(PricingModule VERSION 1.0.3 LANGUAGES CXX)

# --- Set the standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- Common sources
set(SOURCES
    src/Grid/trinomial/calculate_greeks.cpp
    src/Grid/trinomial/internal/calculate_price.cpp
    src/Grid/trinomial/internal/helpers.cpp
    src/Grid/GreeksResult.cpp
    src/Grid/bsm/calculate_greeks.cpp
    src/Grid/Grid.cpp
)

# --- Static library for shared code
add_library(pricingCore STATIC ${SOURCES})
target_compile_options(pricingCore PUBLIC
    -Wall
    -Wextra
    -Wshadow
    -Weffc++
    -Wconversion
    -Wno-error=sign-compare
    -Wno-error=array-bounds
    -pedantic-errors
    -Werror
    -fPIC
)
target_include_directories(pricingCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# BS thread pool dependency
find_path(BSHOSHANY_THREAD_POOL_INCLUDE_DIRS "BS_thread_pool.hpp")
target_include_directories(pricingCore PUBLIC ${BSHOSHANY_THREAD_POOL_INCLUDE_DIRS})

# Eigen dependency
find_package(Eigen3 CONFIG REQUIRED)
target_link_libraries(pricingCore PUBLIC Eigen3::Eigen)

# Pybind dependency
find_package(pybind11 CONFIG REQUIRED)
target_link_libraries(pricingCore PUBLIC pybind11::embed)

# --- Pybind module
set(PYBIND11_FINDPYTHON ON)
pybind11_add_module(pricing src/module.cpp)
target_link_libraries(pricing PRIVATE pricingCore)
install(TARGETS pricing DESTINATION .)

# --- Test executable
add_executable(pricingTest tests/pricing_test.cpp)
target_link_libraries(pricingTest PRIVATE pricingCore)

# --- Debug / Sanitizer flags
set(SANITIZER_FLAGS 
    -fsanitize=address
    -fsanitize=undefined
    -fsanitize=leak
    -fsanitize=bounds
    -fsanitize=integer-divide-by-zero
    -fsanitize=float-divide-by-zero
    -fsanitize=null
)

# Apply sanitizer flags to both targets
foreach(tgt pricing pricingTest)
    target_compile_options(${tgt} PRIVATE 
        $<$<CONFIG:Debug>:-O0 -g>
        $<$<CONFIG:Debug>:${SANITIZER_FLAGS}>
    )
    target_link_options(${tgt} PRIVATE 
        $<$<CONFIG:Debug>:${SANITIZER_FLAGS}>
    )
endforeach()
