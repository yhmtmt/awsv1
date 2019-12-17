project(aws)
cmake_minimum_required(VERSION 3.2)
add_compile_options(-std=c++11)

list(APPEND CMAKE_PREFIX_PATH "/opt/aws" "/opt/protobuf")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

set(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0 -pg")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -s -DNODEBUG -march=native")
find_package(Protobuf REQUIRED)
find_package(GRPC REQUIRED)
find_package(Flatbuffers REQUIRED)

set (CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})

set (SPDLOG_DIR ${PROJECT_SOURCE_DIR}/sub/spdlog)
set (SPDLOG_SRCS ${SPDLOG_DIR}/src/spdlog.cpp)
include_directories(${SPDLOG_DIR}/include PUBLIC ${PROJECT_SOURCE_DIR}/include)

file(GLOB FLATS ${PROJECT_SOURCE_DIR}/fbs/*.fbs)
set(FLAT_SRC_DIR ${PROJECT_SOURCE_DIR}/fbs/src)
include_directories(PUBLIC ${FLAT_SRC_DIR})

file(GLOB CHANS ${PROJECT_SOURCE_DIR}/channels/*.cpp)

flatc_generate_cpp(FLAT_HDRS FLAT_BFBS ${FLAT_SRC_DIR} ${FLATS})
add_custom_target(generate-protosrcs ALL DEPENDS ${PROTO_SRCS} ${PROTO_HDRS})
add_custom_target(generate-grpcsrcs ALL DEPENDS ${GRPC_SRCS} ${GRPC_HDRS})
add_custom_target(generate-flatobjs ALL DEPENDS ${FLAT_HDRS} ${FLAT_BFBS})
add_subdirectory(src)
add_subdirectory(filters)
add_subdirectory(rcmd)

enable_testing()
add_subdirectory(test)

install(FILES ${FLAT_BFBS} DESTINATION lib)

