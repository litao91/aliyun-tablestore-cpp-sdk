set(SRCS
    asio.cpp
    asio_impl.cpp
    bookmark_input_stream.cpp
    client.cpp
    connection_impl.cpp
    response_reader.cpp
    timer_impl.cpp
    types.cpp)
add_library(http ${SRCS})
