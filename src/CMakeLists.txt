set(PROTOS ${PROJECT_SOURCE_DIR}/proto/command.proto)
set(PROTO_SRC_DIR ${PROJECT_SOURCE_DIR}/proto/src)
file(MAKE_DIRECTORY ${PROTO_SRC_DIR})
include_directories(PUBLIC ${PROTO_SRC_DIR})

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_SRC_DIR} ${PROTOS})
grpc_generate_cpp(GRPC_SRCS GRPC_HDRS ${PROTO_SRC_DIR} ${PROTOS})


set(GARMIN_XHD_RADAR_SRCS GarminxHDControl.cpp GarminxHDReceive.cpp socketutil.cpp)
add_executable(aws aws.cpp aws_temp.cpp CmdAppBase.cpp aws_clock.cpp aws_command.cpp aws_coord.cpp aws_map.cpp aws_nmea_ais.cpp aws_nmea_gps.cpp aws_nmea.cpp aws_serial.cpp aws_sock.cpp aws_stdlib.cpp ${CHANS} channel_base.cpp table_base.cpp filter_base.cpp ${PROTO_SRCS} ${GRPC_SRCS})
add_dependencies(aws generate-protosrcs)
add_dependencies(aws generate-grpcsrcs)


target_link_libraries(aws pthread dl flatbuffers::libflatbuffers gRPC::grpc++_reflection protobuf::libprotobuf)

install(TARGETS aws DESTINATION bin)
