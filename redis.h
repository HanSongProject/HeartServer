#ifndef REDIS_H
#define REDIS_H

#include "singleton.h"
#include "hiredis.h"
#include "list"
#include "global.h"
#include <iostream>


class Redis
{
public:

    Redis();
    bool connect(const char *ip,int port);
    int get(string dev,list<dbInfo>& redisList); //-1:断开重新连接 0: 失败 1: 成功
    int isExist(string dev,string key);
    int set(string dev,dbInfo key);
private:
    redisContext *mConnect;
    redisReply *reply;
    DECLARE_SINGLETON_CLASS(Redis)
};

typedef Singleton<Redis> SRedis;

#endif // REDIS_H
