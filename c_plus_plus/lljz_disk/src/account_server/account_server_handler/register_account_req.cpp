#include "account_server_handler.h"

namespace lljz {
namespace disk {

void RegisterAccountReq(RequestPacket* req,
void* args, ResponsePacket* resp) {
    Document req_doc;
    Document resp_doc;
    Document::AllocatorType& resp_allocator=resp_doc.GetAllocator();
    StringBuffer resp_buffer;
    Writer<StringBuffer> resp_writer(resp_buffer);
    Value resp_json(kObjectType);
    Value resp_error_msg(kStringType);
    std::string resp_data;

    TBSYS_LOG(DEBUG,"-------data:%s",req->data_);
    req_doc.Parse(req->data_);
    std::string req_account;
    std::string req_password;

    //检查account
    if (!req_doc.HasMember("account") || !req_doc["account"].IsString()) {
        resp->error_code_=20001;
        resp_error_msg="account is invalid";
        resp_json.AddMember("error_msg",resp_error_msg,resp_allocator);

        resp_json.Accept(resp_writer);
        resp_data=resp_buffer.GetString();
        sprintf(resp->data_,"%s",resp_data.c_str());
        return;
    }
    req_account=req_doc["account"].GetString();

    //检查password
    if (!req_doc.HasMember("password") || !req_doc["password"].IsString() ) {
        resp->error_code_=20001;
        resp_error_msg="password is invalid";
        resp_json.AddMember("error_msg",resp_error_msg,resp_allocator);

        resp_json.Accept(resp_writer);
        resp_data=resp_buffer.GetString();
        sprintf(resp->data_,"%s",resp_data.c_str());
        return;
    }
    req_password=req_doc["password"].GetString();

    if (req_account.empty() || req_password.empty()) {
        resp->error_code_=20001;
        resp_error_msg="account,password is invalid";
        resp_json.AddMember("error_msg",resp_error_msg,resp_allocator);

        resp_json.Accept(resp_writer);
        resp_data=resp_buffer.GetString();
        sprintf(resp->data_,"%s",resp_data.c_str());
        return;
    }

    //增加账号
    RedisClient* rc=REDIS_CLIENT_MANAGER.GetRedisClient();
    char cmd[200];
    sprintf(cmd,"HSETNX %s account %s", 
        req_account.c_str(), req_account.c_str());
    redisReply* reply;
    int cmd_ret=Rhsetnx(rc,cmd,reply);
    if (SUCCESS_ACTIVE != cmd_ret) {
        REDIS_CLIENT_MANAGER.ReleaseRedisClient(rc,cmd_ret);

        resp->error_code_=20002;
        resp_error_msg="redis database server is busy";
        resp_json.AddMember("error_msg",resp_error_msg,resp_allocator);

        resp_json.Accept(resp_writer);
        resp_data=resp_buffer.GetString();
        sprintf(resp->data_,"%s",resp_data.c_str());
        return;
    }

    sprintf(cmd,"HMSET %s password %s timestamp %lld",
        req_account.c_str(),req_password.c_str(), 
        tbsys::CTimeUtil::getTime());
    cmd_ret=Rhmset(rc,cmd,reply);
    if (SUCCESS_ACTIVE != cmd_ret) {
        REDIS_CLIENT_MANAGER.ReleaseRedisClient(rc,cmd_ret);

        resp->error_code_=20002;
        resp_error_msg="redis database server is busy";
        resp_json.AddMember("error_msg",resp_error_msg,resp_allocator);

        resp_json.Accept(resp_writer);
        resp_data=resp_buffer.GetString();
        sprintf(resp->data_,"%s",resp_data.c_str());
        return;
    }

    REDIS_CLIENT_MANAGER.ReleaseRedisClient(rc,true);

    resp->error_code_=0;
    resp_error_msg="";
    resp_json.AddMember("error_msg",resp_error_msg,resp_allocator);

    resp_json.Accept(resp_writer);
    resp_data=resp_buffer.GetString();
    sprintf(resp->data_,"%s",resp_data.c_str());
}

}
}