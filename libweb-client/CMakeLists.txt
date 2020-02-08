project(libweb-client)

aux_source_directory(./ SRCS)

# Declare the library
add_library(${PROJECT_NAME} STATIC
    ${SRCS}
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)