cmake_minimum_required(VERSION 3.22)
enable_testing()

# Find the testing framework (Google Test)
find_package(GTest CONFIG REQUIRED)

# Add an executable for the tests
add_executable(PricingModuleTests pricing_module_tests.cpp)

# Link dependencies
find_package(Torch REQUIRED)
target_link_libraries(PricingModuleTests PRIVATE 
    GTest::gtest_main 
    ${TORCH_LIBRARIES}
    StrictFlags
)

# Include directories
# Path to main 'include' directory to access the internal headers
target_include_directories(PricingModuleTests PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

# Define test cases for CTest
# CTest automatically handles the main function based on GTest::gtest_main
include(GoogleTest)
gtest_discover_tests(PricingModuleTests)
