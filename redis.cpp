#include "redis.h"

#include "global.h"
using namespace std;
Redis::Redis()
{
    mConnect = NULL;
}

bool Redis::connect(const char *ip, int port)
{
    bool ret = true;
    if(mConnect)
        redisFree(mConnect);
    mConnect = redisConnect(ip, port);
    if (mConnect == NULL || mConnect->err)
    {
        if (mConnect) {
            printMsg("Error: %s\n", mConnect->errstr);

        } else {
            printMsg("Can't allocate redis context\n");
        }
        redisFree(mConnect);
        mConnect = NULL;
        ret = false;
    }
    return ret;

}

int Redis::get(string dev,list<dbInfo> &redisList)
{
    int ret = 0;
    redisList.clear();
    redisReply  *reply = NULL;
    reply= (redisReply*)redisCommand(mConnect,"HGETALL %s", dev.c_str());
    if(NULL == reply || mConnect->err)
    {
        ret = -1;
    }
    else if(reply->type == REDIS_REPLY_ARRAY)
    {
        for(uint i = 1;i <reply->elements;i = i+2)
        {
            dbInfo info;
            info.key = reply->element[i-1]->str;
            info.value = reply->element[i]->str;
            redisList.push_back(info);
        }
        ret = 1;
    }
    freeReplyObject(reply);
    return ret;
}

int Redis::isExist(string dev, string key)
{
    int ret = 0;
    redisReply *reply = NULL;
    reply = (redisReply*)redisCommand(mConnect,"HEXISTS %s %s",dev.c_str(),key.c_str());
    if(NULL == reply || mConnect->err)
    {
        ret = -1;
    }
    else if(reply->type == REDIS_REPLY_INTEGER)
    {
        printMsg("type: %lld",reply->integer);
        if(reply->integer == 1)
        ret = 1;
    }

    freeReplyObject(reply);
    return ret;
}

int Redis::set(string dev, dbInfo key)
{
    /*
    list<dbInfo>::iterator it;  //定义迭代器

    for(it = key.begin(); it != key.end(); it++)
    {
        str = str + " " + (*it).key + " " + (*it).value;

    }
    */
    int ret = 0;
   // printf("set: dev: %s,key: %s :  %s\n",dev.c_str(),key.key.c_str(),key.value.c_str());
    redisReply *reply = (redisReply*)redisCommand(mConnect,"HSET %s %s %s",dev.c_str(),key.key.c_str(),key.value.c_str());
    if(NULL == reply || mConnect->err)
    {
        ret = -1;
    }
    else if(reply->type == REDIS_REPLY_INTEGER)
    {
        //返回1 表示新建域,返回0 表示新值覆盖旧值
        if(reply->integer == 0)
            ret = 1;
        //printMsg("type :%d",reply->integer);
    }
    freeReplyObject(reply);
    return ret;
}

