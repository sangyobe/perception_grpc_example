cmake_minimum_required(VERSION 3.13)
project(imu_subscriber)

list(APPEND CMAKE_PREFIX_PATH "${ARTF_INSTALL_DIR}/lib/cmake")
find_package(artf CONFIG REQUIRED)

file(GLOB SRCS "*.cpp" "${CMAKE_SOURCE_DIR}/proto/*.cc")
add_executable(${PROJECT_NAME} ${SRCS})
target_link_libraries(${PROJECT_NAME} PRIVATE
    artf::dtcore_grpc
    artf::dtproto_grpc
)
set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES
    CXX_STANDARD 17
)

project(vision_rpc_client)

set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

file(GLOB SRCS "*.cpp" "${CMAKE_SOURCE_DIR}/proto/*.cc")
message(STATUS "${PROJECT_NAME} sources: ${SRCS}")
add_executable(${PROJECT_NAME}
    ${SRCS}
)
target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${Protobuf_INCLUDE_DIRS}
)
target_link_libraries(${PROJECT_NAME}
    dtproto_grpc
    protobuf::libprotobuf
    gRPC::grpc++
    gRPC::grpc++_reflection
    spdlog::spdlog
    yaml-cpp
)