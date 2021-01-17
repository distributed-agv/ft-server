#include "guide_server_impl.h"
#include <pthread.h>
#include <exception>
#include <iostream>
#include "Astar.h"
#include </usr/local/include/hiredis/hiredis.h>
#include <cstdlib>
#include <stdio.h>
#include <unistd.h>
using namespace std;
GuideServiceImpl::GuideServiceImpl(int car_num, IntPair boundary, const std::string &redis_host, int redis_port,
                                   const std::string &commit_script_sha, const std::string &recover_script_sha,const std::string &getlock_script_sha)
  : car_num(car_num), boundary(boundary), redis_host(redis_host), redis_port(redis_port),
    commit_script_sha(commit_script_sha),recover_script_sha(recover_script_sha),getlock_script_sha(getlock_script_sha){
      w=boundary.x;
      l=boundary.y;
    }

  GuideServiceImpl::~GuideServiceImpl() {}


int GuideServiceImpl::releaseLock(string *release_lock,int next_step,int seq,int car_id,redisContext *redis_context){
  redisReply *redis_reply = (redisReply *) redisCommand(
    redis_context,
    "EVALSHA %s 0 %d %d %s %s %s %s %d",
    commit_script_sha.c_str(),
    car_id,
    seq,
    release_lock[0].c_str(),
    release_lock[1].c_str(),
    release_lock[2].c_str(),
    release_lock[3].c_str(),
    next_step
  );
  if (redis_reply==NULL||redis_reply->type == REDIS_REPLY_ERROR)
    return 6;
  int result;

  sscanf(redis_reply->str, "%d", &result);

  freeReplyObject(redis_reply);
  return result;
}

int GuideServiceImpl::getLock(int x,int y,int car_id,int seq,redisContext *redis_context){
  string key=to_string(car);
  string *name=new string[5];
  if(x+1<w){
    name[0]="("+to_string(x+1)+","+to_string(y)+")";
  }
  else{
    name[0]="";
  }
  if(y+1<l){
    name[1]="("+to_string(x)+","+to_string(y+1)+")";
  }
  else{
    name[1]="";
  }
  if(x-1>=0){
    name[2]="("+to_string(x-1)+","+to_string(y)+")";
  }
  else{
    name[2]="";
  }
  if(y-1>=0){
    name[3]="("+to_string(x)+","+to_string(y-1)+")";
  }
  else{
    name[3]="";
  }
  name[4]="("+to_string(x)+","+to_string(y)+")";
  redisReply *redis_reply = (redisReply *) redisCommand(
      redis_context,
      "EVALSHA %s 0 %d %d %s %s %s %s %s",
      getlock_script_sha.c_str(),
      car_id,
      seq,
      name[0].c_str(),
      name[1].c_str(),
      name[2].c_str(),
      name[3].c_str(),
      name[4].c_str()
    );
    if (redis_reply == NULL || redis_reply->type != REDIS_REPLY_STRING){
      return 6;
    }
    int result;
    sscanf(redis_reply->str, "%d", &result);
    freeReplyObject(redis_reply);
    return result;
}


Step::StepCode changeNextToDirect(int nowX, int nowY, int nextX,int nextY){
  if(nextX==-1&&nextY==-1)
     return (Step::StepCode)0;
  if(nowX==nextX-1)
     return (Step::StepCode)3;
  if(nowX==nextX+1)
     return (Step::StepCode)4;
  if(nowY==nextY-1)
     return (Step::StepCode)2;
  if(nowY==nextY+1)
     return (Step::StepCode)1;
  return (Step::StepCode)0;
}
  
int  GuideServiceImpl::findNextStep(int x,int y,int &nextX,int &nextY,int car,int goalX,int goalY,redisContext *conn){
   int **map=new int*[w];
    for(int i=0;i<w;i++)
    {
        map[i]=new int [l];
        for(int j=0;j<l;j++)
        {
          map[i][j]=-1;
        }    
    }
    
    redisReply* redis_reply= (redisReply*)redisCommand(conn, "HGETALL owner_map");
    if(redis_reply==NULL){
      freeReplyObject(redis_reply);
      return 0;
    }
    for (int i = 0; i < redis_reply->elements; i += 2) {
      int car_id = 0;
      int tmpX;
      int tmpY;
      sscanf(redis_reply->element[i]->str, "(%d,%d)", &tmpX, &tmpY);
      sscanf(redis_reply->element[i + 1]->str, "%d", &car_id);
      map[tmpX][tmpY]=car_id;
    }
   
    Astar astar(l-1,w-1,car);
    Node *startPos = new Node(x,y);
    Node *endPos = new Node(goalX,goalY);
  
    int *tmpX=new int;
    int *tmpY=new int;
    if(astar.search(startPos,endPos,map,tmpX,tmpY)){
      nextY=*tmpY;
      nextX=*tmpX;
    }
    else{
      nextX=-1;
      nextY=-1;
    }
    freeReplyObject(redis_reply);
    return 1;
}



string* GuideServiceImpl::findReleaseLock(int x,int y,int nextX,int nextY,int car,redisContext *redis_context){
  string *release_lock=new string[4];
  int num =0;
  string name="("+to_string(x)+","+to_string(y+1)+")";
  redisReply* reply=(redisReply*)redisCommand(redis_context, "HGET %s %s ", "owner_map",name.data());
  if(reply==NULL){
    release_lock[0]="error";
    freeReplyObject(reply);
    return release_lock;
  }
  if(reply->str && atoi(reply->str)==car && nextY!=y+1){
    release_lock[num]=name;
    num++;
  }
  name="";
  
  name="("+to_string(x+1)+","+to_string(y)+")";
  
  reply=(redisReply*)redisCommand(redis_context, "HGET %s %s ", "owner_map",name.data());
  if(reply==NULL){
    release_lock[0]="error";
    freeReplyObject(reply);
    return release_lock;
  }
  if(reply->str && atoi(reply->str)==car && nextX!=x+1){
    release_lock[num]=name;
    num++;
  }
  if(x-1>=0){
    name="";
    name="("+to_string(x-1)+","+to_string(y)+")";

    reply=(redisReply*)redisCommand(redis_context, "HGET %s %s ", "owner_map",name.data());
    if(reply==NULL){
      release_lock[0]="error";
      freeReplyObject(reply);
      return release_lock;
    }
    if(reply->str && atoi(reply->str)==car && nextX!=x-1){
      release_lock[num]=name;
      num++;
    }
  }
  if(y-1>=0){
    name="";
    name="("+to_string(x)+","+to_string(y-1)+")";
    reply=(redisReply*)redisCommand(redis_context, "HGET %s %s ", "owner_map",name.data());
    if(reply==NULL){
      release_lock[0]="error";
      freeReplyObject(reply);
      return release_lock;
    }
    if(reply->str && atoi(reply->str)==car && nextY!=y-1){
      release_lock[num]=name;
      num++;
    }
  }
  freeReplyObject(reply);
  return release_lock;
}




grpc::Status GuideServiceImpl::GetNextStep(grpc::ServerContext *service_context,
                                           const CarState *car_state, Step *step) {
                                             
  
  int car_id = car_state->car_id();
  int seq = car_state->seq();
  IntPair cur_pos(car_state->cur_pos().row_idx(), car_state->cur_pos().col_idx());
  IntPair last_pos(car_state->last_pos().row_idx(), car_state->last_pos().col_idx());
  IntPair dst_pos(car_state->dst_pos().row_idx(), car_state->dst_pos().col_idx());
  redisContext *redis_context = redisConnect(redis_host.c_str(), redis_port);
  cout<<car_id<<" "<<seq<<"  "<<cur_pos.x<<" "<<cur_pos.y<<" "<<last_pos.x<<" "<<last_pos.y<<endl;
  grpc::Status result;

  //判断是否连接到redis
  if (NULL == redis_context || redis_context->err)
  {
      result = grpc::Status(grpc::StatusCode::INTERNAL, "RPC server failed to connect to Redis");
      Step::StepCode step_code = Step_StepCode_STOP;
      step->set_step_code(step_code);//停止
      redisFree(redis_context);
      return result;
  }

  //判断是否存在lua脚本
  redisReply *lua_exit=(redisReply *)redisCommand(redis_context,"SCRIPT %s %s" ,"EXISTS",recover_script_sha.data());
  if (NULL == lua_exit ){
      result = grpc::Status(grpc::StatusCode::INTERNAL, "RPC server failed to connect to Redis");
      redisFree(redis_context);
      return result;
  }
  if(lua_exit->element[0]->integer==0){
    pid_t pid = fork();
      if (pid == 0)
        execlp("python3", "python3", "recover.py");
    result = grpc::Status(grpc::StatusCode::INTERNAL, "RPC server failed to connect to Redis");
    freeReplyObject(lua_exit);
    redisFree(redis_context);
    return result;
  }
  freeReplyObject(lua_exit);
  //判断是否处于恢复数据库状态
  if(seq<0){
    redisReply *redis_reply = (redisReply *) redisCommand(
      redis_context,
      "EVALSHA %s 0 %d %d (%d,%d) %d",
      recover_script_sha.c_str(),
      car_id,
      seq,
      cur_pos.x,
      cur_pos.y,
      car_num
    );
   
    if (redis_reply == NULL || redis_reply->type != REDIS_REPLY_STRING){
      result = grpc::Status(grpc::StatusCode::INTERNAL, "RPC server failed to connect to Redis");
      freeReplyObject(redis_reply);
      redisFree(redis_context);
      return result;
    }
    int next_step;
    sscanf(redis_reply->str, "%d", &next_step);
    freeReplyObject(redis_reply);
  
    bool reset_timer_success = false;

    if (next_step < 0 && ResetTimer(redis_context, reset_timer_success) == 0 && reset_timer_success) {
      
      pid_t pid = fork();
      if (pid == 0)
        execlp("python3", "python3", "car-locator/locator_recover.py", "-d", "-n", std::to_string(seq).c_str(), NULL);
    }
    redisFree(redis_context);
    step->set_step_code((Step::StepCode)next_step);
    return result;
  }

  //判断是否过期请求
  string tmp= "seq:"+ to_string(car_id);
  redisReply* reply = (redisReply*)redisCommand(redis_context, "GET %s", tmp.data());
  if(reply==NULL){
    result = grpc::Status(grpc::StatusCode::INTERNAL, "RPC server failed to connect to Redis");
    freeReplyObject(reply);
    redisFree(redis_context);
    return result;
  }
  if(reply->type == REDIS_REPLY_NIL){
      string tmp= "seq:"+ to_string(car_id);
      string seqTmp=to_string(0);
      redisCommand(redis_context, "SETNX %s %s", tmp.data(),seqTmp.data());
  }

  freeReplyObject(reply);
  


  //拿锁阶段
  int getlockResult=getLock(cur_pos.x,cur_pos.y,car_id,seq,redis_context);
  if(getlockResult==6){
      redisFree(redis_context);
      result = grpc::Status(grpc::StatusCode::INTERNAL, "RPC server failed to connect to Redis");
      return result;
  }
  else if(getlockResult<0){
    redisFree(redis_context);
    step->set_step_code((Step::StepCode)getlockResult);
    return result;
  }
  else if(getlockResult!=7){
    redisFree(redis_context);
    step->set_step_code((Step::StepCode)getlockResult);
    return result;
  }

  
  int nextX,nextY;
 


  //寻路阶段
  int findNextStepResult=findNextStep(cur_pos.x,cur_pos.y,nextX,nextY,car_id,dst_pos.x,dst_pos.y,redis_context);
  if(findNextStepResult==0){
    redisFree(redis_context);
    result = grpc::Status(grpc::StatusCode::INTERNAL, "RPC server failed to connect to Redis");
    return result;
  }

  Step::StepCode step_code=changeNextToDirect(cur_pos.x,cur_pos.y,nextX,nextY);
  step->set_step_code(step_code);



  //放锁阶段
  string* release_lock;
  release_lock=findReleaseLock(cur_pos.x,cur_pos.y,nextX,nextY,car_id,redis_context);
  if(release_lock[0]=="error"){
    redisFree(redis_context);
    result = grpc::Status(grpc::StatusCode::INTERNAL, "RPC server failed to connect to Redis");
    return result;
  }
  step_code=(Step::StepCode)releaseLock(release_lock,(int)step_code,seq,car_id,redis_context);
  if(step_code==6){
    redisFree(redis_context);
    result = grpc::Status(grpc::StatusCode::INTERNAL, "RPC server failed to connect to Redis");
    return result;
  }
  step->set_step_code(step_code);
  redisFree(redis_context);
  return  result;
}

int GuideServiceImpl::ResetTimer(redisContext *redis_context, bool &success_out) {
  int result = 0;
  redisReply *redis_reply = (redisReply *) redisCommand(redis_context, "SET timer TIMER PX 60000 NX");
  if (redis_reply == NULL || (redis_reply->type != REDIS_REPLY_NIL && redis_reply->type != REDIS_REPLY_STATUS)) {
    result = 1;
    goto leave;
  }
  success_out = redis_reply->type != REDIS_REPLY_NIL;
leave:
  freeReplyObject(redis_reply);
  return result;
}