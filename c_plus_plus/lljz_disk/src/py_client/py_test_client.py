#!/usr/bin/env python

import py_client

'''
request
{
    "src_type":0,
    "src_id":0,
    "dest_type":0,
    "dest_id":0,
    "msg_id":0,
    "version":0,
    "data":{}
}

response
{
    "src_type":0,
    "src_id":0,
    "dest_type":0,
    "dest_id":0,
    "msg_id":0,
    "error_code":0,
    "data":{}
}
'''

def RunTest():
    #print(help(py_client))
    if not py_client.Init():
        print("init fail")
        return

    conn_id=py_client.Connect("tcp:127.0.0.1:10000",True)
    print("conn_id=%d" % conn_id)

    request = {
        "src_type":1001,
        "src_id":0,
        "dest_type":,1
        "dest_id":0,
        "msg_id":0,
        "version":0,
        "data":{
            "spec":"tcp:127.0.0.1:8000",
            "srv_type":1001,
            "srv_id":0
        }
    }

    py_client.Stop()
    py_client.Wait()


if "__main__"==__name__:
    RunTest()