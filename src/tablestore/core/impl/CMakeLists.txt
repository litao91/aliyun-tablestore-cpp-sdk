set(SRCS
    api_traits.cpp
    async_batch_writer.cpp
    async_client.cpp
    async_client_base.cpp
    ots_constants.cpp
    ots_helper.cpp
    serde.cpp
    sync_client.cpp)
add_library(ots_core_impl ${SRCS})
target_link_libraries(ots_core_impl ots_protos)
