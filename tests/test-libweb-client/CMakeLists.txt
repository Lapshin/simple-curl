file(GLOB SRCS *.cpp)

ADD_EXECUTABLE(test-libweb-client ${SRCS})

TARGET_LINK_LIBRARIES(test-libweb-client
    libweb-client
    libgtest
    libgmock
)

add_test(NAME test-libweb-client
         COMMAND test-libweb-client)