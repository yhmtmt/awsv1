set(PROTOS ${PROJECT_SOURCE_DIR}/proto/command.proto)
set(PROTO_SRC_DIR ${PROJECT_SOURCE_DIR}/proto/src)
file(MAKE_DIRECTORY ${PROTO_SRC_DIR})
include_directories(PUBLIC ${PROTO_SRC_DIR})

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_SRC_DIR} ${PROTOS})
grpc_generate_cpp(GRPC_SRCS GRPC_HDRS ${PROTO_SRC_DIR} ${PROTOS})


set(GARMIN_XHD_RADAR_SRCS GarminxHDControl.cpp GarminxHDReceive.cpp socketutil.cpp)
add_executable(aws aws.cpp aws_temp.cpp CmdAppBase.cpp aws_clock.cpp aws_coord.cpp aws_map.cpp aws_nmea_ais.cpp aws_nmea_gps.cpp aws_nmea.cpp aws_serial.cpp aws_sock.cpp aws_stdlib.cpp ${CHANS} channel_base.cpp channel_factory.cpp table_base.cpp filter_base.cpp ${PROTO_SRCS} ${GRPC_SRCS})
add_dependencies(aws generate-protosrcs)
add_dependencies(aws generate-grpcsrcs)
target_link_libraries(aws pthread dl flatbuffers::libflatbuffers gRPC::grpc++_reflection protobuf::libprotobuf stdc++fs)
install(TARGETS aws DESTINATION bin)

add_executable(caws caws.cpp ${PROTO_SRCS} ${GRPC_SRCS})
target_link_libraries(caws flatbuffers::libflatbuffers gRPC::grpc++_reflection protobuf::libprotobuf)
install(TARGETS caws DESTINATION bin)

add_executable(t2str t2str.cpp aws_clock.cpp aws_stdlib.cpp)
install(TARGETS t2str DESTINATION bin)
add_executable(str2t str2t.cpp aws_clock.cpp aws_stdlib.cpp)
install(TARGETS str2t DESTINATION bin)
add_executable(blh2ecef blh2ecef.cpp aws_coord.cpp)
install(TARGETS blh2ecef DESTINATION bin)
add_executable(ecef2blh ecef2blh.cpp aws_coord.cpp)
install(TARGETS ecef2blh DESTINATION bin)

