#include "connection_manager_to_server.hpp"
//#include "json/json.h"

namespace lljz {
namespace disk {

ConnectionManagerToServer::ConnectionManagerToServer(
    tbnet::Transport* transport,
    tbnet::IPacketStreamer* packet_streamer,
    tbnet::IPacketHandler* packet_handler)
:transport_(transport)
,packet_streamer_(packet_streamer)
,packet_handler_(packet_handler)
,stop_(false) {
    memset(config_server_spec_,0,
        sizeof(config_server_spec_));
    conn_to_config_server_=NULL;
}

ConnectionManagerToServer::~ConnectionManagerToServer() {
    destroy();
}

bool ConnectionManagerToServer::start(
    const char* config_server_spec) {
    memset(config_server_spec_,0,
        sizeof(config_server_spec_));
    strcat(config_server_spec_,config_server_spec);
    conn_to_config_server_=transport_->connect(
        config_server_spec_,packet_streamer_,true);
    if (NULL==conn_to_config_server_) {
        return false;
    }
    transport_->start();

    get_server_url_thread_.start(this,NULL);
    return true;
}

bool ConnectionManagerToServer::stop() {
    stop_=true;
    return true;
}

bool ConnectionManagerToServer::wait() {
    get_server_url_thread_.join();
    destroy();
    return true;
}

void ConnectionManagerToServer::destroy() {

}

void ConnectionManagerToServer::run(
    tbsys::CThread* thread, void* arg) {
    //get server url from config_server
    std::string spec;
    uint32_t server_type=0;
    uint32_t server_id=0;
    std::vector<ServerURL*> buff_server_url;
    bool server_changed=false;

    while (!stop_) {
/*        {
            printf("test::ConnectionManagerToServer::run\n");
            sleep(3);
            continue;
        }*/

        char data[1024]={0};
        Json::Value data_root;
        Json::Reader reader;
        if (!reader.parse(data, data_root, false)) {
            continue;
        }

        int size=data_root.size();
        int svr_url_size=server_url_.size();
        buff_server_url.clear();

        server_changed=false;
        //server_url_ check remove
        for (int i=0;i<svr_url_size;i++) {
            for (int j=0;j<size;j++) {
                spec=data_root[j]["spec"];
                if (0==strcmp(server_url_[i]->spec_,spec.c_str())) {
                    break;
                }
            }
            if (j==size) {
                server_url_[i]->changed_=2;
                server_changed=true;
            }
        }

        //server_url_ check add
        for (int i=0;i<size;i++) {
            spec=data_root[i]["spec"].asString();
            server_type=data_root[i]["server_type"].asInt();
            server_id=data_root[i]["server_id"].asInt();

            for (int j=0;j<svr_url_size;j++) {
                if (0==strcmp(server_url_[j]->spec_, spec.c_str())) {
                    break;
                }
            }
            if (j==svr_url_size) {
                ServerURL* svr_url=new ServerURL;
                strcat(svr_url->spec_,spec.c_str());
                svr_url->server_type_=server_type;
                svr_url->server_id_=server_id;
                svr_url->changed_=1;
                buff_server_url.push(svr_url);
                server_changed=true;
            }
        }

        size=buff_server_url.size();
        for (int i=0;i<size;i++) {
            server_url_.push(buff_server_url[i])
        }
        buff_server_url.clear();

        if (!server_changed) {
            sleep(30);
            continue;
        }

        //refresh connect manager
        __gnu_cxx::hash_map<uint32_t,
            LoadConnectionManager*>::iteraotr it;
        LoadConnectionManager* load_conn_manager=NULL;
        mutex_.lock();
        for (int i=0;i<server_url_.size();i++) {
            if (1==server_url_[i]->changed_) {
                //add a connection
                server_url_[i]->changed_=0;

                it=conn_manager_.find(
                    server_url_[i]->server_type_);
                if (conn_manager_.end()==it) {
                    load_conn_manager=new LoadConnectionManager(
                        transport_,
                        packet_streamer_,
                        packet_handler_);
                    conn_manager_[server_url_[i]->server_id_]=
                        load_conn_manager;
                } else {
                    load_conn_manager=it->second;
                }
                load_conn_manager->connect(
                    server_id_[i]->server_id_,
                    NULL,0,0);
            } else if (2==server_url_[i]->changed_) {
                //remove a connection
                //server_url_[i]->changed_=0;

                it=conn_manager_.find(
                    server_url_[i]->server_type_);
                if (conn_manager_.end() == it) {
                    continue;
                }
                load_conn_manager=it->second;
                load_conn_manager->disconnect(
                    server_url_[i]->server_id_);
            }
        }

        for (int i=0; i<server_url_.size();) {
            if (2==server_url_[i]->changed_) {
                server_url_.erase(server_url_.begin()+i);
                continue;
            }
            i++;
        }
        mutex_.unlock();
        sleep(30); //30s
    }
}

}
}