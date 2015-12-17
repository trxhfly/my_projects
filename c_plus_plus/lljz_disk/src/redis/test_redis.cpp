#include "tbsys.h"
#include "tbnet.h"
#include "hiredis.h"
#include "redis_client_manager.h"
#include "redis_client.h"

using namespace lljz::disk;

void test_ping(RedisClientManager* manager) {
    RedisClient* rc=manager->GetRedisClient();
    if (NULL==rc || NULL==rc->redis_context_) {
        printf("connect redis fail\n");
        return;
    }
    redisReply* reply=(redisReply* )redisCommand(rc->redis_context_,"ping");
    if (NULL==reply) {
        printf("%s","redis_client ping fail");
        manager->ReleaseRedisClient(rc,false);
        return;
    }
    printf("type=%d, str=%s\n", reply->type, reply->str);
    if( !(reply->type == REDIS_REPLY_STATUS && strcasecmp(reply->str,"pong")==0)) {
        printf("%s","redis_client ping fail");
        freeReplyObject(reply);
        manager->ReleaseRedisClient(rc,false);
        return;
    }
    
    freeReplyObject(reply);
    manager->ReleaseRedisClient(rc,true);
}

void test_set(RedisClientManager* manager,int count) {
    char cmd[200];
    int64_t start=tbsys::CTimeUtil::getTime();
    for (int i=0;i<count;i++) {
        sprintf(cmd,"set yjb_%d %d",i,1000+i);
        RedisClient* rc=manager->GetRedisClient();
        if (NULL==rc || NULL==rc->redis_context_) {
            printf("connect redis fail\n");
            continue;
        }
        redisReply* reply=(redisReply* )redisCommand(rc->redis_context_,cmd);
        if (NULL==reply) {
            printf("%s","redis_client set fail");
            manager->ReleaseRedisClient(rc,false);
            continue;
        }
        printf("type=%d, str=%s\n", reply->type, reply->str);
        if( !(reply->type == REDIS_REPLY_STATUS && strcasecmp(reply->str,"OK")==0)) {
            printf("%s","redis_client ping fail");
            freeReplyObject(reply);
            manager->ReleaseRedisClient(rc,true);
            continue;
        }
    
        freeReplyObject(reply);
        manager->ReleaseRedisClient(rc,true);
    }
    int64_t end=tbsys::CTimeUtil::getTime();
    printf("time=%lld\n", end-start);
}

void test_setnx(RedisClientManager* manager) {
    RedisClient* rc=manager->GetRedisClient();
    if (NULL==rc || NULL==rc->redis_context_) {
        printf("connect redis fail\n");
        return;
    }
    char cmd[100];
    sprintf(cmd,"set test_setnx test_setnx");
    redisReply* reply=(redisReply* )redisCommand(rc->redis_context_,cmd);
    if (NULL==reply) {
        printf("%s fail\n",cmd);
        manager->ReleaseRedisClient(rc,false);
        return;
    }
    printf("%s:type=%d, str=%s\n", cmd, reply->type, reply->str);
    freeReplyObject(reply);

    sprintf(cmd,"setnx test_setnx test_setnx");
    reply=(redisReply* )redisCommand(rc->redis_context_,cmd);
    if (NULL==reply) {
        printf("%s fail\n",cmd);
        manager->ReleaseRedisClient(rc,false);
        return;
    }
    printf("%s:type=%d, str=%s\n", cmd, reply->type, reply->str);
    
    freeReplyObject(reply);
    manager->ReleaseRedisClient(rc,true);
}

void test_Rcommand(RedisClientManager* manager) {
    RedisClient* rc=manager->GetRedisClient();
    redisReply* reply;
    int cmd_ret=Rcommand(rc, "ping", reply);
    printf("cmd_ret=%d\n",cmd_ret);
    if (SUCCESS_ACTIVE != cmd_ret) {
        manager->ReleaseRedisClient(rc, cmd_ret);
    }
    manager->ReleaseRedisClient(rc,true);
}

void test_Rhsetnx(RedisClientManager* manager) {
    RedisClient* rc=manager->GetRedisClient();
    redisReply* reply;
    int cmd_ret=Rhsetnx(rc, "HSETNX yjb life happyfull", reply);
    printf("cmd_ret=%d\n",cmd_ret);
    if (SUCCESS_ACTIVE != cmd_ret) {
        manager->ReleaseRedisClient(rc, cmd_ret);
    }
    manager->ReleaseRedisClient(rc,true);
}

void test_Rhmset(RedisClientManager* manager) {
    RedisClient* rc=manager->GetRedisClient();
    redisReply* reply;
    int cmd_ret=Rhmset(rc, "HMSET yjb life happyfull age 27 sex man", reply);
    printf("cmd_ret=%d\n",cmd_ret);
    if (SUCCESS_ACTIVE != cmd_ret) {
        manager->ReleaseRedisClient(rc, cmd_ret);
    }
    manager->ReleaseRedisClient(rc,true);
}

int main(int argc,char* argv[]) {
    if(TBSYS_CONFIG.load("test.ini")) {
        printf("load test.ini fail");
        return -1;
    }
    RedisClientManager redis_client_manager;
    redis_client_manager.start();
    sleep(3);

//    test_ping(&redis_client_manager);
//    test_set(&redis_client_manager, 10000);
//    test_setnx(&redis_client_manager);
    test_Rcommand(&redis_client_manager);
    test_Rhsetnx(&redis_client_manager);
    test_Rhsetnx(&redis_client_manager);
    test_Rhmset(&redis_client_manager);

    redis_client_manager.stop();
    redis_client_manager.wait();
    return 0;
}