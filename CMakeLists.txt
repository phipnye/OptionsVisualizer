cmake_minimum_required(VERSION 3.22)
project(OptionsSurfaceModule VERSION 1.0.4 LANGUAGES CXX)

# --- Set the standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- Common sources
set(SOURCES
    src/pricing/PricingSurface.cpp
    src/pricing/GreeksResult.cpp
    src/core/OptionsManager.cpp
    src/core/linspace.cpp
    src/lru/ParamsHash.cpp
    src/lru/LRUCache.cpp
    src/lru/Params.cpp
    src/models/trinomial/calculate_greeks.cpp
    src/models/trinomial/internal/helpers.cpp
    src/models/bsm/calculate_greeks.cpp
)

# --- Static library for shared code
add_library(optionsSurfaceCore STATIC ${SOURCES})
target_compile_options(optionsSurfaceCore PUBLIC
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
target_include_directories(optionsSurfaceCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# BS thread pool dependency
find_path(BSHOSHANY_THREAD_POOL_INCLUDE_DIRS "BS_thread_pool.hpp")
target_include_directories(optionsSurfaceCore PUBLIC ${BSHOSHANY_THREAD_POOL_INCLUDE_DIRS})

# Eigen dependency
find_package(Eigen3 CONFIG REQUIRED)
target_link_libraries(optionsSurfaceCore PUBLIC Eigen3::Eigen)

# Pybind dependency
find_package(pybind11 CONFIG REQUIRED)
target_link_libraries(optionsSurfaceCore PUBLIC pybind11::module)

# --- Pybind module
set(PYBIND11_FINDPYTHON ON)
pybind11_add_module(options_surface src/module.cpp)
target_link_libraries(options_surface PRIVATE optionsSurfaceCore)
install(TARGETS options_surface DESTINATION .)

# --- Test executable
add_executable(optionsSurfaceTest tests/options_surface_test.cpp)
target_link_libraries(optionsSurfaceTest PRIVATE optionsSurfaceCore)

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
foreach(tgt optionsSurfaceCore options_surface optionsSurfaceTest)
    target_compile_options(${tgt} PRIVATE
        $<$<CONFIG:Debug>:-O0 -g>
        $<$<CONFIG:Debug>:${SANITIZER_FLAGS}>
    )
    target_link_options(${tgt} PRIVATE
        $<$<CONFIG:Debug>:${SANITIZER_FLAGS}>
    )
endforeach()
