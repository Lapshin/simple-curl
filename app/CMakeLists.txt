project(simple-curl)

aux_source_directory(./ SRCS)

add_executable(${PROJECT_NAME}
    ${SRCS}
)

# Specify here the libraries this program depends on
target_link_libraries(${PROJECT_NAME}
    libweb-client
)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)