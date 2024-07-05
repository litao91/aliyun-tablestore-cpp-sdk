set(SRCS
    arithmetic.cpp
    assert.cpp
    logger.cpp
    logging.cpp
    mempiece.cpp
    mempool.cpp
    network.cpp
    prettyprint.cpp
    random.cpp
    security.cpp
    threading.cpp
    timestamp.cpp)
add_library(tablestore_util ${SRCS})
