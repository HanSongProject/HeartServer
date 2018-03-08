#ifndef EPOLLSERVER_H
#define EPOLLSERVER_H

#include "singleton.h"
#include <sys/epoll.h>
#include <pthread.h>
#include "json/json.h"

#define MAXEVENTS 65535

class CEpollServer;
typedef struct epollData_s
{
    CEpollServer *mPointer;
    int fd;
    char *buf;
}epollData;

class CEpollServer
{
public:
    CEpollServer();

    bool init(int port);
    static void *demo(void *arg);
    static void terminal_dev_login(void *arg, Json::Value &root);
    static void terminal_dev_beats(void *arg, Json::Value &root);
    static void listenEpThread(void *);
    static void delEnter(char *buf);

public:
    int mSockFd;
    int mEpfd;
    pthread_t mThreadId;
    struct epoll_event event;   // 告诉内核要监听什么事件
    struct epoll_event wait_event[MAXEVENTS]; //内核监听完的结果

    DECLARE_SINGLETON_CLASS(CEpollServer)
};

typedef Singleton<CEpollServer> SCEpollServer;

#endif // EPOLLSERVER_H
