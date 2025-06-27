cmake_minimum_required(VERSION 3.13)
project(opstate_subscriber)

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