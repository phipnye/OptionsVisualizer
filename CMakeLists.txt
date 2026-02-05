cmake_minimum_required(VERSION 3.22)
project(PricingEngineModule VERSION 1.0.4 LANGUAGES CXX)

# --- Set the standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- Common source files
set(SOURCES
        src/pricing/PricingSurface.cpp
        src/pricing/GreeksResult.cpp
        src/core/OptionsManager.cpp
        src/core/linspace.cpp
        src/params/PricingParamsHash.cpp
        src/params/PricingParams.cpp
        src/models/trinomial/internal/helpers.cpp
        src/models/bsm/calculate_greeks.cpp)

# --- Static library for shared code
add_library(PricingEngineCore STATIC ${SOURCES})
target_compile_options(PricingEngineCore PUBLIC
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
target_include_directories(PricingEngineCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# BS thread pool dependency
find_path(BSHOSHANY_THREAD_POOL_INCLUDE_DIRS "BS_thread_pool.hpp")
target_include_directories(PricingEngineCore PUBLIC ${BSHOSHANY_THREAD_POOL_INCLUDE_DIRS})

# Eigen dependency
find_package(Eigen3 CONFIG REQUIRED)
target_link_libraries(PricingEngineCore PUBLIC Eigen3::Eigen)

# Pybind dependency
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11 CONFIG REQUIRED)
Python3_add_library(CppPricingEngine src/module.cpp)
target_link_libraries(PricingEngineCore PUBLIC pybind11::module)
target_link_libraries(CppPricingEngine PRIVATE PricingEngineCore)
install(TARGETS CppPricingEngine DESTINATION .)

# --- Tests
enable_testing()
find_package(GTest CONFIG REQUIRED)
add_executable(PricingEngineTests tests/validate_results.cpp)
target_link_libraries(PricingEngineTests PRIVATE PricingEngineCore GTest::gtest_main)
add_test(NAME PricingEngineTests COMMAND PricingEngineTests)
target_compile_definitions(PricingEngineTests PRIVATE TEST_DATA_PATH="${CMAKE_CURRENT_SOURCE_DIR}/tests/data/")

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

# Apply sanitizer flags and debugging compilation flags
foreach (tgt PricingEngineCore PricingEngineTests)
    target_compile_options(${tgt} PRIVATE  $<$<CONFIG:Debug>:-O0 -g ${SANITIZER_FLAGS}>)
    target_link_options(${tgt} PRIVATE $<$<CONFIG:Debug>:${SANITIZER_FLAGS}>)
endforeach ()
