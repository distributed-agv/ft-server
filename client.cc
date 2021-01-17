#include "guide.grpc.pb.h"
#include <iostream>
#include <string>
#include <memory>
#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

int main() {
  std::string server_addr = "localhost:50051";
  std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(server_addr, grpc::InsecureChannelCredentials());
  std::unique_ptr<Guide::Stub> stub = Guide::NewStub(channel);

  while (true) {
    grpc::ClientContext context;
    grpc::Status status;
    CarState car_state;
    CarState_Position *cur_pos = new CarState_Position;
    CarState_Position *last_pos = new CarState_Position;
    CarState_Position *dst_pos = new CarState_Position;
    Step step;
    int car_id = 0;
    int seq = 0;
    int cur_row_idx = 0;
    int cur_col_idx = 0;
    int last_row_idx = 0;
    int last_col_idx = 0;
    int dst_row_idx = 0;
    int dst_col_idx = 0;

    std::cin >> car_id >> seq >> cur_row_idx >> cur_col_idx >> last_row_idx >> last_col_idx
             >> dst_row_idx >> dst_col_idx;
    car_state.set_car_id(car_id);
    car_state.set_seq(seq);
    cur_pos->set_row_idx(cur_row_idx);
    cur_pos->set_col_idx(cur_col_idx);
    car_state.set_allocated_cur_pos(cur_pos);
    last_pos->set_row_idx(last_row_idx);
    last_pos->set_col_idx(last_col_idx);
    car_state.set_allocated_last_pos(last_pos);
    dst_pos->set_row_idx(dst_row_idx);
    dst_pos->set_col_idx(dst_col_idx);
    car_state.set_allocated_dst_pos(dst_pos);

    status = stub->GetNextStep(&context, car_state, &step);

    if (status.ok())
      std::cout << step.step_code() << std::endl;
    else
      std::cout << status.error_message() << std::endl;
  }

  return 0;
}