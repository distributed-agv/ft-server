cmake_minimum_required(VERSION 3.13)
project(distributed-agv)

set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)
find_package(gRPC CONFIG REQUIRED)

add_subdirectory(${PROJECT_SOURCE_DIR}/hiredis/)

add_executable(server main_server.cc guide_service_impl.cc util.cc guide.grpc.pb.cc guide.pb.cc)
add_executable(test_client main_test_client.cc guide.grpc.pb.cc guide.pb.cc)

target_link_libraries(server gRPC::grpc++ hiredis)
target_link_libraries(test_client gRPC::grpc++)
