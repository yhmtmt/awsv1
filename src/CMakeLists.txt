set(PROTOS ${PROJECT_SOURCE_DIR}/proto/command.proto)
set(PROTO_SRC_DIR ${PROJECT_SOURCE_DIR}/proto/src)
file(MAKE_DIRECTORY ${PROTO_SRC_DIR})
include_directories(PUBLIC ${PROTO_SRC_DIR})

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_SRC_DIR} ${PROTOS})
grpc_generate_cpp(GRPC_SRCS GRPC_HDRS ${PROTO_SRC_DIR} ${PROTOS})


set(GARMIN_XHD_RADAR_SRCS GarminxHDControl.cpp GarminxHDReceive.cpp socketutil.cpp)
add_executable(aws aws.cpp aws_temp.cpp CmdAppBase.cpp aws_clock.cpp aws_coord.cpp aws_state.cpp aws_map.cpp aws_map_point.cpp aws_map_coast_line.cpp aws_map_depth.cpp aws_png.cpp aws_nmea_ais.cpp aws_nmea_gps.cpp aws_nmea.cpp aws_serial.cpp aws_sock.cpp aws_stdlib.cpp ${CHANS} channel_base.cpp channel_factory.cpp table_base.cpp filter_base.cpp ${PROTO_SRCS} ${GRPC_SRCS})
add_dependencies(aws generate-protosrcs)
add_dependencies(aws generate-grpcsrcs)
target_link_libraries(aws pthread dl flatbuffers::libflatbuffers gRPC::grpc++_reflection protobuf::libprotobuf stdc++fs atomic png)
install(TARGETS aws DESTINATION bin)

add_executable(caws caws.cpp ${PROTO_SRCS} ${GRPC_SRCS})
target_link_libraries(caws flatbuffers::libflatbuffers gRPC::grpc++_reflection protobuf::libprotobuf atomic)
install(TARGETS caws DESTINATION bin)

add_executable(t2str t2str.cpp aws_clock.cpp aws_stdlib.cpp)
install(TARGETS t2str DESTINATION bin)
add_executable(str2t str2t.cpp aws_clock.cpp aws_stdlib.cpp)
install(TARGETS str2t DESTINATION bin)
add_executable(blh2ecef blh2ecef.cpp aws_coord.cpp)
install(TARGETS blh2ecef DESTINATION bin)
add_executable(ecef2blh ecef2blh.cpp aws_coord.cpp)
install(TARGETS ecef2blh DESTINATION bin)

#build configuration for libaws_state.so
find_package(PythonLibs 3 REQUIRED)

STRING(REGEX REPLACE "^([0-9]+)\\.[0-9]+\\.[0-9]+" "\\1" python_major_vers "${PYTHONLIBS_VERSION_STRING}")
STRING(REGEX REPLACE "^[0-9]+\\.([0-9])+\\.[0-9]+" "\\1" python_minor_vers "${PYTHONLIBS_VERSION_STRING}")
STRING(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]+)" "\\1" python_patch_vers "${PYTHONLIBS_VERSION_STRING}")


find_package(Boost COMPONENTS python-py${python_major_vers}${python_minor_vers} REQUIRED)

add_library(aws_state SHARED aws_state.cpp aws_coord.cpp aws_clock.cpp)
target_include_directories(aws_state PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(aws_state PUBLIC ${Boost_INCLUDE_DIRS})
target_include_directories(aws_state PUBLIC ${PYTHON_INCLUDE_DIRS})
target_compile_definitions(aws_state PUBLIC PY_EXPORT)
target_link_libraries(aws_state ${Boost_LIBRARIES})
install(TARGETS aws_state DESTINATION lib)

