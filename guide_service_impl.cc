#include "guide_service_impl.h"
#include "guide.pb.h"
#include "hiredis/hiredis.h"
#include "util.h"
#include <grpc/grpc.h>
#include <cstdlib>
#include <unistd.h>
#include <string>
#include <cstring>


GuideServiceImpl::GuideServiceImpl(int car_num, IntPair boundary, const std::string &redis_host, int redis_port,
                                   const std::string &commit_script_sha, const std::string &recover_script_sha)
  : car_num(car_num), boundary(boundary), redis_host(redis_host), redis_port(redis_port),
    commit_script_sha(commit_script_sha), recover_script_sha(recover_script_sha) {}

GuideServiceImpl::~GuideServiceImpl() {}

grpc::Status GuideServiceImpl::GetNextStep(grpc::ServerContext *service_context,
                                           const CarState *car_state, Step *step) {
  int car_id = car_state->car_id();
  int seq = car_state->seq();
  IntPair cur_pos(car_state->cur_pos().row_idx(), car_state->cur_pos().col_idx());
  IntPair last_pos(car_state->last_pos().row_idx(), car_state->last_pos().col_idx());
  IntPair dst_pos(car_state->dst_pos().row_idx(), car_state->dst_pos().col_idx());
  redisContext *redis_context = redisConnect(redis_host.c_str(), redis_port);
  std::map<IntPair, int> owner_map;
  Step::StepCode step_code = Step_StepCode_STOP;
  IntPair acquiring_pos;
  IntPair releasing_pos;
  grpc::Status result;

  if (redis_context == NULL) {
    result = grpc::Status(grpc::StatusCode::INTERNAL, "RPC server failed to connect to Redis");
    goto leave;
  }

  if (seq < 0) {
    bool reset_timer_success = false;
    if (Recover(redis_context, car_id, seq, cur_pos, car_num, step_code) != 0) {
      result = grpc::Status(grpc::StatusCode::INTERNAL, "RPC server failed to recover Redis");
      goto leave;
    }
    if ((int) step_code < 0 && ResetTimer(redis_context, reset_timer_success) == 0 && reset_timer_success) {
      pid_t pid = fork();
      if (pid == 0)
        execlp("python3", "python3", "car-locator/locator_recover.py", "-d", "-n", std::to_string(seq).c_str(), NULL);
    }
    goto leave;
  }

  if (FetchOwnerMap(redis_context, owner_map) != 0) {
    result = grpc::Status(grpc::StatusCode::INTERNAL, "RPC server failed to fetch owner map");
    goto leave;
  }
  
  step_code = PlanRoute(car_id, cur_pos, dst_pos, owner_map);
  acquiring_pos = cur_pos + kOffsets[step_code];
  releasing_pos = last_pos == cur_pos || last_pos == acquiring_pos ? IntPair(-1, -1) : last_pos;

  if (Commit(redis_context, car_id, seq, step_code, acquiring_pos, releasing_pos, step_code) != 0) {
    result = grpc::Status(grpc::StatusCode::INTERNAL, "RPC server failed to commit");
    goto leave;
  }

leave:
  step->set_step_code(step_code);
  redisFree(redis_context);
  return result;
}

int GuideServiceImpl::FetchOwnerMap(redisContext *redis_context, std::map<IntPair, int> &owner_map_out) {
  int result = 0;
  redisReply *redis_reply = (redisReply *) redisCommand(redis_context, "HGETALL owner_map");
  
  if (redis_reply == NULL || redis_reply->type != REDIS_REPLY_ARRAY) {
    result = 1;
    goto leave;
  }

  owner_map_out.clear();
  for (int i = 0; i < redis_reply->elements; i += 2) {
    IntPair pos;
    int car_id = 0;
    sscanf(redis_reply->element[i]->str, "(%d,%d)", &pos.x, &pos.y);
    sscanf(redis_reply->element[i + 1]->str, "%d", &car_id);
    owner_map_out[pos] = car_id;
  }
  
leave:
  freeReplyObject(redis_reply);
  return result;
}

Step::StepCode GuideServiceImpl::PlanRoute(int car_id, IntPair cur_pos, IntPair dst_pos,
                                           const std::map<IntPair, int> &owner_map) {
  Step::StepCode result = Step_StepCode_STOP;
  IntPair cur_to_dst = dst_pos - cur_pos;

  for (int k = 0; k < 5; ++k) {
    IntPair offset = kOffsets[k];
    IntPair neighbor_pos = cur_pos + offset;
    if (neighbor_pos.Validate(boundary) && (!owner_map.count(neighbor_pos) || owner_map.at(neighbor_pos) == car_id)) {
      if (offset.DotProduct(cur_to_dst) > 0) {
        result = (Step::StepCode) k;
        break;
      } else if (std::rand() > RAND_MAX / 2) {
        result = (Step::StepCode) k;
      }
    }
  }
  if (cur_pos == dst_pos)
    result = Step_StepCode_STOP;
  
  return result;
}

int GuideServiceImpl::Commit(redisContext *redis_context, int car_id, int seq, Step::StepCode step_code,
                             IntPair acquiring_pos, IntPair releasing_pos, Step::StepCode &step_code_out) {
  int result = 0;
  redisReply *redis_reply = (redisReply *) redisCommand(
    redis_context,
    "EVALSHA %s 0 %d %d %d (%d,%d) (%d,%d)",
    commit_script_sha.c_str(),
    car_id,
    seq,
    (int) step_code,
    acquiring_pos.x,
    acquiring_pos.y,
    releasing_pos.x,
    releasing_pos.y
  );

  if (redis_reply == NULL || redis_reply->type != REDIS_REPLY_STRING) {
    result = 1;
    goto leave;
  }

  sscanf(redis_reply->str, "%d", &step_code_out);

leave:
  freeReplyObject(redis_reply);
  return result;
}

int GuideServiceImpl::Recover(redisContext *redis_context, int car_id, int nonce, IntPair cur_pos,
                              int car_num, Step::StepCode &step_code_out) {
  int result = 0;
  redisReply *redis_reply = (redisReply *) redisCommand(
    redis_context,
    "EVALSHA %s 0 %d %d (%d,%d) %d",
    recover_script_sha.c_str(),
    car_id,
    nonce,
    cur_pos.x,
    cur_pos.y,
    car_num
  );

  if (redis_reply == NULL || redis_reply->type != REDIS_REPLY_STRING) {
    result = 1;
    goto leave;
  }

  sscanf(redis_reply->str, "%d", &step_code_out);

leave:
  freeReplyObject(redis_reply);
  return result;
}

int GuideServiceImpl::ResetTimer(redisContext *redis_context, bool &success_out) {
  int result = 0;
  redisReply *redis_reply = (redisReply *) redisCommand(redis_context, "SET timer TIMER PX 5000 NX");
  
  if (redis_reply == NULL || (redis_reply->type != REDIS_REPLY_NIL && redis_reply->type != REDIS_REPLY_STATUS)) {
    result = 1;
    goto leave;
  }
  
  success_out = redis_reply->type != REDIS_REPLY_NIL;

leave:
  freeReplyObject(redis_reply);
  return result;
}
