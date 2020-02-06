file(GLOB SRCS *.cpp)

# Declare the library
add_library(libweb-client STATIC
    ${SRCS}
)

target_include_directories(libweb-client PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)