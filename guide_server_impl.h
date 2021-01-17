#include <grpc/grpc.h>
#include "guide.grpc.pb.h"
#include "util.h"
#include <pthread.h>
#include <vector>
#include <map>
#include <utility>
#include </usr/local/include/hiredis/hiredis.h>

using namespace std;
class GuideServiceImpl final : public Guide::Service {
 public:
   explicit GuideServiceImpl(int, IntPair, const std::string &, int, const std::string &,const std::string &,const std::string &);
  virtual ~GuideServiceImpl();
  virtual grpc::Status GetNextStep(grpc::ServerContext *, const CarState *, Step *);


  int releaseLock(string*,int,int,int,redisContext *);
  int  getLock(int ,int,int,int,redisContext *);
  int  findNextStep(int ,int ,int&,int&,int,int,int,redisContext *);
  string* findReleaseLock(int x,int y,int nextX,int nextY,int car,redisContext *conn);
  int ResetTimer(redisContext *, bool &);
 private:

  int car_num;
  IntPair boundary;
  std::string redis_host;
  int redis_port;
  std::string commit_script_sha;
  std::string recover_script_sha;
  std::string getlock_script_sha;
  int car, x, y;
  int mx[10][10];
  int w;
  int l;
  pthread_mutex_t mutex;
};

