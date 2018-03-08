#include <iostream>
#include "epollserver.h"
#include "threadpool.h"
#include "global.h"
#include "redis.h"
#include "json/json.h"
#include "readFile.h"
#include <unistd.h>


using namespace std;


int main()
{
#if 0
    dbInfo d;
    d.key = "cpu1";
    d.value = "123";
    if(d.key == KEYS[CPU])
        printMsg("cpu is success");
    const char* str = "{\"uploadid\": \"UP000000\",\"code\": \"100\",\"msg\": \"\",\"files\": \"\"}";

    Json::Reader reader;
    Json::Value root;
    if (reader.parse(str, root))  // reader将Json字符串解析到root，root将包含Json里所有子元素
    {
        std::string upload_id = root["uploadid"].asString();  // 访问节点，upload_id = "UP000000"
        string s = root["code"].asString();
        // printMsg("ss: %s",s.c_str());
        ulong code = strtoul(root["code"].asString().c_str(),0,0);    // 访问节点，code = 100
        if(code == 100)
        {
            printMsg("upload:%s ,code: %lu",upload_id.c_str(),code);
        }
    }
    //printMsg("connect:%d",SRedis::instance()->connect("127.0.0.1",6379));

    //printMsg("dev is exist: %d ", SRedis::instance()->isExist("dev.d05af1801cac","cpu"))

    printMsg("connect:%d",SRedis::instance()->connect("127.0.0.1",6379));
    list<dbInfo> ret ;
    SRedis::instance()->get("dev.d05af1801cac",ret);
    dbInfo v= ret.back();
    v.value = "1";
#endif
    SReadFile::instance()->GetCurrentPath("Config.ini");

    string ip = SReadFile::instance()->GetIniKeyString("INFO","ip");
    string port = SReadFile::instance()->GetIniKeyString("INFO","port");
    printMsg("ip:%s port:%s",ip.c_str(),port.c_str());


    SRedis::instance()->connect(ip.c_str(),atoi(port.c_str()));
    SCEpollServer::instance()->init(8080);
    //dbInfo db;
    //db.key = "cpu";
   // db.value = "88";
    //SRedis::instance()->set("dev.d05af1801cac",db);

    cout << "Hello World!" << endl;
    return 0;
}

