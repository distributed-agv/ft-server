#ifndef GUIDE_SERVICE_IMPL_H
#define GUIDE_SERVICE_IMPL_H

#include "guide.grpc.pb.h"
#include "guide.pb.h"
#include "util.h"
#include "hiredis/hiredis.h"
#include <string>
#include <map>

class GuideServiceImpl final : public Guide::Service {
public:
  explicit GuideServiceImpl(int, IntPair, const std::string &, int, const std::string &);
  virtual ~GuideServiceImpl();
  virtual grpc::Status GetNextStep(grpc::ServerContext *, const CarState *, Step *);
private:
  int car_num;
  IntPair boundary;
  std::string redis_host;
  int redis_port;
  std::string commit_script_sha;
  int FetchOwnerMap(redisContext *, std::map<IntPair, int> &);
  Step::StepCode PlanRoute(int, IntPair, IntPair, const std::map<IntPair, int> &);
  int Commit(redisContext *, int, int, Step::StepCode, IntPair, IntPair, Step::StepCode &);
};

#endif
