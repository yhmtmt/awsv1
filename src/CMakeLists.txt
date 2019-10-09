#set(PROTOS ${CMAKE_CURRENT_SOURCE_DIR}/../Proto/sample.proto)
set(PROTO_SRC_DIR ${CMAKE_CURRENT_BINARY_DIR}/ProtoSrc)

file(MAKE_DIRECTORY ${PROTO_SRC_DIR})

#protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_SRC_DIR} ${PROTOS})
#grpc_generate_cpp(GRPC_SRCS GRPC_HDRS ${PROTO_SRC_DIR} ${PROTOS})

add_executable(aws aws.cpp aws_temp.cpp CmdAppBase.cpp aws_clock.cpp aws_command.cpp aws_coord.cpp aws_map.cpp aws_nmea_ais.cpp aws_nmea_gps.cpp aws_nmea.cpp aws_serial.cpp aws_sock.cpp aws_stdlib.cpp channel_base.cpp filter_base.cpp) 

target_link_libraries(aws pthread dl)

#target_link_libraries(aws gRPC::grpc++_reflection protobuf::libprotobuf)

install(TARGETS aws DESTINATION bin)
