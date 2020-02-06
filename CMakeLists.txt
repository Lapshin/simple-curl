cmake_minimum_required(VERSION 2.8)

add_compile_options("-pedantic")
add_compile_options("-Wall")
add_compile_options("-Wextra")
add_compile_options("-Werror")

set(CMAKE_CXX_STANDARD 11)

project(WC)

enable_testing()

add_subdirectory(app)
add_subdirectory(tests)
add_subdirectory(libweb-client)

