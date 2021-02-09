cmake_minimum_required(VERSION 3.17)
project(client)

set(CMAKE_CXX_STANDARD 14)

include(FetchContent)

# Use all of the damn CPU power to compile stuff on MSVC
if(MSVC)
    include(ProcessorCount)

    # Parallel builds
    set(MSVC_COMPILE_FLAGS "/MP")

    ProcessorCount(N)
    if(NOT N EQUAL 0)
        set(MSVC_COMPILE_FLAGS "/MP${N}")
        message("MSVC: Setting processor count to ${N}")
    endif()

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  ${MSVC_COMPILE_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MSVC_COMPILE_FLAGS}")
endif()

if (LINUX)
    # -mno-red-zone is to avoid VMP crashes
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

# On Windows we need to set the path to YASM or it won't compile properly...
if(WIN32)
    set(CMAKE_ASM_NASM_COMPILER "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/yasm.exe")
endif()

option(gRPC_MSVC_STATIC_RUNTIME "Link with static msvc runtime libraries" ON)

FetchContent_Declare(
        gRPC
        GIT_REPOSITORY https://github.com/grpc/grpc
        GIT_TAG        v1.35.0
)
set(FETCHCONTENT_QUIET OFF)
FetchContent_MakeAvailable(gRPC)

# Since FetchContent uses add_subdirectory under the hood, we can use
# the grpc targets directly from this build.
set(_PROTOBUF_LIBPROTOBUF libprotobuf)
set(_REFLECTION grpc++_reflection)
set(_PROTOBUF_PROTOC $<TARGET_FILE:protoc>)
set(_GRPC_GRPCPP grpc++)
if(CMAKE_CROSSCOMPILING)
    find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
else()
    set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:grpc_cpp_plugin>)
endif()

# Proto file
get_filename_component(hw_proto "protos/helloworld.proto" ABSOLUTE)
get_filename_component(hw_proto_path "${hw_proto}" PATH)

# Generated sources
set(hw_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/helloworld.pb.cc")
set(hw_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/helloworld.pb.h")
set(hw_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/helloworld.grpc.pb.cc")
set(hw_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/helloworld.grpc.pb.h")
add_custom_command(
        OUTPUT "${hw_proto_srcs}" "${hw_proto_hdrs}" "${hw_grpc_srcs}" "${hw_grpc_hdrs}"
        COMMAND ${_PROTOBUF_PROTOC}
        ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${hw_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${hw_proto}"
        DEPENDS "${hw_proto}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# Copy roots.pem into build directory
file(COPY ${CMAKE_CURRENT_LIST_DIR}/roots.pem DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_executable(client
        main.cpp
        HelloWorldManager.h
        HelloWorldManager.cpp
        ${hw_grpc_srcs}
        ${hw_grpc_hdrs}
        ${hw_proto_srcs}
        ${hw_proto_hdrs}
    )

target_link_libraries(client
        grpc++
    )