#include "guide_service_impl.h"
#include "util.h"
#include <string>
#include <utility>
#include <memory>
#include <iostream>
#include <vector>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/security/server_credentials.h>

int main() {
  GuideServiceImpl service(2, IntPair(5, 5), "127.0.0.1", 6379, "bfc469b924aa723a5732ee35f91dbd8e714b2388");
  std::string server_addr = "0.0.0.0:50051";
  grpc::ServerBuilder builder;
  std::unique_ptr<grpc::Server> server;

  builder.AddListeningPort(server_addr, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  server = builder.BuildAndStart();

  std::cout << "Server listening on " << server_addr << std::endl;
  server->Wait();

  return 0;
}
