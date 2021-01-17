cmake_minimum_required(VERSION 3.13)
set (CMAKE_CXX_STANDARD 17)
project(agv C CXX)

find_package(Threads REQUIRED)
find_package(gRPC CONFIG REQUIRED)

add_executable(main main.cc util.cc guide_server_impl.cc guide.grpc.pb.cc guide.pb.cc Astar.cc)
add_executable(client client.cc guide.grpc.pb.cc guide.pb.cc Astar.cc)

target_link_libraries(main gRPC::grpc++)
target_link_libraries(main Threads::Threads)
target_link_libraries(client gRPC::grpc++)
target_link_libraries(main /usr/local/lib/libhiredis.a)