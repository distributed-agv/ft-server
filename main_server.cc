#include "guide_service_impl.h"
#include "util.h"
#include <cstdlib>
#include <string>
#include <utility>
#include <memory>
#include <iostream>
#include <vector>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/security/server_credentials.h>

int main(int argc, char *argv[]) {
  int car_num = std::atoi(argv[1]);
  int row_num = std::atoi(argv[2]);
  int col_num = std::atoi(argv[3]);
  char *redis_host = argv[4];
  int redis_port = std::atoi(argv[5]);
  char *server_host = argv[6];
  char *server_port = argv[7];
  char *commit_script_sha = argv[8];
  GuideServiceImpl service(
    car_num,
    IntPair(row_num, col_num),
    redis_host,
    redis_port,
    commit_script_sha
  );
  std::string server_addr = std::string(server_host) + ":" + std::string(server_port);
  grpc::ServerBuilder builder;
  std::unique_ptr<grpc::Server> server;

  builder.AddListeningPort(server_addr, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  server = builder.BuildAndStart();

  std::cout << "Server listening on " << server_addr << std::endl;
  server->Wait();

  return 0;
}
