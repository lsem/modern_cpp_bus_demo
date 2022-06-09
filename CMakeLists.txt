cmake_minimum_required(VERSION 3.5)

project(modern_bus)

set (CMAKE_CXX_STANDARD 17)

add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)

file(GLOB HPP_FILES CONFIGURE_DEPENDS "*.hpp")
file(GLOB CPP_FILES CONFIGURE_DEPENDS "*.cpp")

add_executable(modern_bus ${HPP_FILES} ${CPP_FILES})
target_include_directories(modern_bus PRIVATE .)
target_compile_options(modern_bus PRIVATE -fsanitize=address)
target_link_options(modern_bus PRIVATE -fsanitize=address)
