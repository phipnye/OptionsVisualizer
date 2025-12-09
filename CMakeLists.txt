cmake_minimum_required(VERSION 3.22)
project(PricingEngineServer VERSION 1.0.0 LANGUAGES CXX)

# --- Set the standard
set(CMAKE_CXX_COMPILER /usr/bin/g++)
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

# --- Define executable
add_executable(PricingEngineServer src/main.cpp src/server/HeatMapController.cpp)

# Link the strict compiler flags
target_link_libraries(PricingEngineServer PRIVATE StrictFlags)

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
target_compile_options(PricingEngineServer PRIVATE 
    # Debug info and no optimization
    $<$<CONFIG:Debug>:-O0 -g>

    # Sanitizers
    $<$<CONFIG:Debug>:${SANITIZER_FLAGS}>
)

# Apply Sanitizer flags to the linker to link the runtime libraries
target_link_options(PricingEngineServer PRIVATE 
    $<$<CONFIG:Debug>:${SANITIZER_FLAGS}>
)

#--- Dependencies
# Include directories
target_include_directories(PricingEngineServer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Find and link boost
find_package(Boost REQUIRED COMPONENTS math)
target_link_libraries(PricingEngineServer PRIVATE Boost::math)
find_package(Boost REQUIRED COMPONENTS multiprecision)
target_link_libraries(PricingEngineServer PRIVATE Boost::multiprecision)

# Find and link drogon
find_package(Drogon CONFIG REQUIRED)
target_link_libraries(PricingEngineServer PRIVATE Drogon::Drogon)

# Find and link jsoncpp
find_package(jsoncpp CONFIG REQUIRED)
target_link_libraries(PricingEngineServer PRIVATE JsonCpp::JsonCpp)
