file(GLOB SRCS *.cpp)

add_executable(simple-curl
    ${SRCS}
)

# Specify here the libraries this program depends on
target_link_libraries(simple-curl
    libweb-client
)

install(TARGETS simple-curl DESTINATION bin)