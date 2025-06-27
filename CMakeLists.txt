cmake_minimum_required(VERSION 3.13)

project(bmr_rpc_example)

cmake_policy(SET CMP0077 NEW)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if("$ENV{ARTF_INSTALL_DIR}" STREQUAL "")
    set(ARTF_INSTALL_DIR "$ENV{HOME}/.local/ART_Framework")
else()
    file(TO_CMAKE_PATH "$ENV{ARTF_INSTALL_DIR}" ARTF_INSTALL_DIR)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

include_directories(${CMAKE_SOURCE_DIR}/proto)
include_directories(${ARTF_INSTALL_DIR}/include)
link_directories(${ARTF_INSTALL_DIR}/lib)
link_directories(${ARTF_INSTALL_DIR}/lib64)

# --------------------------------------------------------
# Build examples
# --------------------------------------------------------
add_subdirectory(examples)