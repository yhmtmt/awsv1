set (AWS_COMMAND_CPP ${PROJECT_SOURCE_DIR}/src/aws_command.cpp)
set(PROTOS ${PROJECT_SOURCE_DIR}/proto/command.proto)
set(PROTO_SRC_DIR ${PROJECT_SOURCE_DIR}/proto/src)
file(MAKE_DIRECTORY ${PROTO_SRC_DIR})
include_directories(PUBLIC ${PROTO_SRC_DIR})

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_SRC_DIR} ${PROTOS})
grpc_generate_cpp(GRPC_SRCS GRPC_HDRS ${PROTO_SRC_DIR} ${PROTOS})

add_executable(caws caws.cpp ${PROTO_SRCS} ${GRPC_SRCS})
target_link_libraries(caws flatbuffers::libflatbuffers gRPC::grpc++_reflection protobuf::libprotobuf)
install(TARGETS caws DESTINATION bin)

