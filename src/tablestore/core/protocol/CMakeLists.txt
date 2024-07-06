protobuf_generate_cpp(ots_proto_srcs ots_proto_headers table_store_filter.proto
                      table_store.proto)

# Ignore warnings while compiling protobuf-generated *.pb.h and *.pb.cpp files.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w -Wno-reserved-identifier")

# message("CMAKE_CURRENT_BINARY_DIR: ${CMAKE_CURRENT_BINARY_DIR}")

# Disable clang-tidy for protobuf-generated *.pb.h and *.pb.cpp files.
set(CMAKE_CXX_CLANG_TIDY "")
add_library(ots_protos ${ots_proto_srcs} ${ots_proto_headers})
target_include_directories(ots_protos SYSTEM PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(ots_protos ${Protobuf_LIBRARIES})
