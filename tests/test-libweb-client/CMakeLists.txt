project(test-libweb-client)

aux_source_directory(./ SRCS)
ADD_EXECUTABLE(test-libweb-client ${SRCS})

TARGET_LINK_LIBRARIES(${PROJECT_NAME}
    libweb-client
    libgtest
    libgmock
)

add_test(NAME ${PROJECT_NAME}
         COMMAND ${PROJECT_NAME})