#include "epollserver.h"


#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <pthread.h>
#include <strings.h>
#include "global.h"
#include "threadpool.h"
#include "redis.h"
#include <zlib.h>
#include <string.h>
#include "readFile.h"

static threadpool*pool;


void* CEpollServer::demo(void *arg)
{
    epollData *recvData = (epollData*)arg;
    printMsg("recvdata: %s,fd: %d,pthread_self():%lu",recvData->buf,recvData->fd,pthread_self());

    Json::Reader reader;
    Json::Value root;
    if (reader.parse(recvData->buf, root))
    {
        //printMsg("type: %d",atoi(root["type"].asString().c_str()));
        if(atoi(root["type"].asString().c_str()) == DEV_LOGIN)
        {

            CEpollServer::terminal_dev_login(arg,root);

            // epoll_ctl(recvData->mPointer->mEpfd, EPOLL_CTL_DEL,recvData->fd, &recvData->mPointer->event);
            // close(recvData->fd);
        }
        else if(atoi(root["type"].asString().c_str()) == DEV_BEATS)
        {
            CEpollServer::terminal_dev_beats(arg,root);

            // epoll_ctl(recvData->mPointer->mEpfd, EPOLL_CTL_DEL,recvData->fd, &recvData->mPointer->event);
            //close(recvData->fd);
        }
    }

}



CEpollServer::CEpollServer()
{

}

bool CEpollServer::init(int port)
{
    pool = SThreadPool::instance()->thread_pool_init(10,MAX_QUEUE_NUM);

    //1.创建tcp监听套接字
    mSockFd = socket(AF_INET, SOCK_STREAM, 0);//SOCK_STREAM
    if(mSockFd < 0)
    {
        printMsg("socket error");
        return false;
    }

    //2.绑定sockfd
    struct sockaddr_in my_addr;
    bzero(&my_addr, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    int ret = bind(mSockFd, (struct sockaddr *)&my_addr, sizeof(my_addr));
    if(ret != 0)
    {
        printMsg("bind error");
        return false;
    }

    //3.监听listen

    if(listen(mSockFd, 20) < 0)
    {
        printMsg("listen error");
        return false;
    }

    struct timeval tv_out;
    tv_out.tv_sec = 10;//等待10秒
    tv_out.tv_usec = 0;
    setsockopt(mSockFd,SOL_SOCKET,SO_SNDTIMEO,&tv_out, sizeof(tv_out));
    setsockopt(mSockFd,SOL_SOCKET,SO_RCVTIMEO,&tv_out, sizeof(tv_out));

    mEpfd = epoll_create1(0); // 创建一个 epoll 的句柄，参数要大于 0， 没有太大意义
    if( -1 == mEpfd )
    {
        perror ("epoll_create");
        return false;
    }

    event.data.fd = mSockFd;
    event.events = EPOLLIN | EPOLLET;//读入,边缘触发方式
    int s = epoll_ctl(mEpfd, EPOLL_CTL_ADD, mSockFd, &event);
    if (s == -1)
    {
        perror ("epoll_ctl");
        return false;
    }
    // 监听线程，此线程负责接收客户端连接，加入到epoll中
    if (pthread_create( &mThreadId, 0, ( void *( * )( void * ))listenEpThread, this ) != 0 )
    {
        printMsg("Server 监听线程创建失败!!!");
        return false;
    }
    pthread_join(mThreadId,NULL);
}

void CEpollServer::terminal_dev_login(void *arg, Json::Value &root)
{
    epollData *recvData = (epollData*)arg;
    Json::Value sendRoot;
    Json::FastWriter  fast_write;
    string dev = root["devId"].asString();
    string key = root["key"].asString();
    string hashDev = "dev." + dev;
    string body = dev + key;
    list<dbInfo> dblist;
    ulong  crc = crc32(0L, (unsigned char *)body.c_str(),body.size());
    printMsg("crcRev:%lu,check crc:%lu",crc,strtoul(root["crc"].asString().c_str(),0,0));

    if(strtoul(root["crc"].asString().c_str(),0,0) != crc)
    {
        sendRoot["status"] = Json::Value("401");
    }
    else
    {
        int ret = SRedis::instance()->get(hashDev,dblist);
        if(1 == ret)
            sendRoot["status"] = Json::Value("200");
        else if(-1 == ret)
        {
            string ip = SReadFile::instance()->GetIniKeyString("INFO","ip");
            string port = SReadFile::instance()->GetIniKeyString("INFO","port");
            SRedis::instance()->connect(ip.c_str(),atoi(port.c_str()));
        }
        else
            sendRoot["status"] = Json::Value("402");
    }

    sendRoot["tick"] = root["tick"];

    string sendBody = sendRoot["status"].asString();
    ulong  sendCrc = crc32(0L, (unsigned char *)sendBody.c_str(),sendBody.size());

    char sCrc[64] = {0};
    sprintf(sCrc,"%lu",sendCrc);
    sendRoot["crc"] = Json::Value(sCrc);

    string sendData = fast_write.write(sendRoot);

    write(recvData->fd,sendData.c_str(),strlen(sendData.c_str()));
}

void CEpollServer::terminal_dev_beats(void *arg, Json::Value &root)
{
    epollData *recvData = (epollData*)arg;
    Json::Value sendRoot;
    Json::FastWriter  fast_write;
    string dev = root["devId"].asString();
    string cpu = root["cpu"].asString();
    string memory = root["memory"].asString();
    string playFileId = root["playFileId"].asString();

    string hashDev = "dev." + dev;
    string body = dev + cpu + memory + playFileId;
    list<dbInfo> dblist;
    ulong  crc = crc32(0L, (unsigned char *)body.c_str(),body.size());

    string sendBody;
    sendBody.clear();

    if(strtoul(root["crc"].asString().c_str(),0,0) != crc)
    {
        sendRoot["status"] = Json::Value("401");
        sendBody = sendBody + "401";
    }
    else
    {
        sendRoot["status"] = Json::Value("200");
        sendBody = sendBody + "200";
        int ret = SRedis::instance()->get(hashDev,dblist);
        if(1 == ret)
        {
            list<dbInfo>::iterator it;
            for(it = dblist.begin();it != dblist.end(); it++)
            {

                if((*it).key == KEYS[CPU] && (*it).value != cpu)
                {
                    dbInfo v;
                    v.key = (*it).key;
                    v.value = cpu;
                    SRedis::instance()->set(hashDev,v);
                }
                else if((*it).key == KEYS[MEMORY] && (*it).value != memory)
                {
                    dbInfo v;
                    v.key = (*it).key;
                    v.value = memory;
                    SRedis::instance()->set(hashDev,v);
                }
                else if((*it).key == KEYS[PLAYID] && (*it).value != playFileId)
                {
                    dbInfo v;
                    v.key = (*it).key;
                    v.value = playFileId;
                    SRedis::instance()->set(hashDev,v);
                }
                else if((*it).key == KEYS[PLAYLIST] || (*it).key == KEYS[PLAYUPDATE] || \
                        (*it).key == KEYS[ADDLIST] || (*it).key == KEYS[ADDUPDATE] || \
                        (*it).key == KEYS[NOTIFY] || (*it).key == KEYS[DEBUGLOG])
                {
                    sendRoot[(*it).key.c_str()] = Json::Value((*it).value.c_str());
                    sendBody = sendBody + (*it).value;
                }
            }
        }
        else if(-1 == ret)
        {
            string ip = SReadFile::instance()->GetIniKeyString("INFO","ip");
            string port = SReadFile::instance()->GetIniKeyString("INFO","port");
            SRedis::instance()->connect(ip.c_str(),atoi(port.c_str()));
        }
    }
    sendRoot["tick"] = root["tick"];

    ulong  sendCrc = crc32(0L, (unsigned char *)sendBody.c_str(),sendBody.size());

    char sCrc[64] = {0};
    sprintf(sCrc,"%lu",sendCrc);
    sendRoot["crc"] = Json::Value(sCrc);

    string sendData = fast_write.write(sendRoot);

    write(recvData->fd,sendData.c_str(),strlen(sendData.c_str()));
    printMsg("hearts beats send: %s ",sendData.c_str());
}

void CEpollServer::listenEpThread(void *arg)
{
    CEpollServer *mThread = (CEpollServer*)arg;

    while(1)
    {
        int n, i;
        // 监视并等待多个文件（标准输入，udp套接字）描述符的属性变化（是否可读）
        // 没有属性变化，这个函数会阻塞，直到有变化才往下执行，这里没有设置超时
        n = epoll_wait(mThread->mEpfd, mThread->wait_event, MAXEVENTS, -1);
        for (i = 0; i < n; i++)
        {
            if ((mThread->wait_event[i].events & EPOLLERR) ||
                    (mThread->wait_event[i].events & EPOLLHUP) ||
                    (!(mThread->wait_event[i].events & EPOLLIN)))
            {
                /* An error has occured on this fd, or the socket is not
                        ready for reading (why were we notified then?) */
                printMsg("epoll error\n");
                epoll_ctl(mThread->mEpfd, EPOLL_CTL_DEL,mThread->wait_event[i].data.fd, &mThread->event);
                close(mThread->wait_event[i].data.fd);
                continue;
            }
            else if(mThread->mSockFd == mThread->wait_event[i].data.fd)
            {
                struct sockaddr in_addr;
                socklen_t in_len;
                int infd;
                in_len = sizeof in_addr;
                infd = accept(mThread->mSockFd, &in_addr, &in_len);
                if(infd < 0)
                    continue;
                mThread->event.data.fd = infd;
                mThread->event.events = EPOLLIN | EPOLLET;
                int s = epoll_ctl(mThread->mEpfd, EPOLL_CTL_ADD, infd, &mThread->event);
                if (s == -1)
                {
                    perror ("epoll_ctl");
                    continue;
                }
            }
            else if(mThread->wait_event[i].events & EPOLLRDHUP)
            {
                printMsg("client fd close\n");
                epoll_ctl(mThread->mEpfd, EPOLL_CTL_DEL,mThread->wait_event[i].data.fd, &mThread->event);
                close(mThread->wait_event[i].data.fd);
                continue;
            }
            else if(mThread->wait_event[i].events & EPOLLIN)
            {
                ssize_t count = 0;
                char buf[512] ={0};

                count = read(mThread->wait_event[i].data.fd, buf, sizeof(buf));
                if(pool == NULL)
                {
                    printMsg("pool is null");

                }
                // write(mThread->wait_event[i].data.fd, buf, sizeof(buf));
                if (count <= 0)
                {
                    printMsg("count is 0");
                    //客户端主动关闭连接
                    epoll_ctl(mThread->mEpfd, EPOLL_CTL_DEL,mThread->wait_event[i].data.fd, &mThread->event);
                    close(mThread->wait_event[i].data.fd);
                    continue;
                }


                //printMsg("buf recv count: %d buf:%s",count,buf);
                epollData data;
                data.buf = buf;
                data.fd = mThread->wait_event[i].data.fd;
                data.mPointer = mThread;
                SThreadPool::instance()->threadpool_add_task(pool,demo,(void*)&data);
                //printMsg("count:%d ,value:%s ",count,buf);

#if 0
                if (done)
                {
                    // printf ("Closed connection on descriptor %d\n",
                    //    events[i].data.fd);

                    /* Closing the descriptor will make epoll remove it
                     from the set of descriptors which are monitored. */
                    close(mThread->wait_event[i].data.fd);
                }
#endif

            }
        }

    }
}

void CEpollServer::delEnter(char *buf)
{
    int i=0;
    int j=0;
    while(buf[j]!='\0'){
        if(buf[j]!=' '){
            buf[i]=buf[j];
            i++;
            j++;
        }
        else
        {
            // string[i]=string[j+1];
            j++;
        }
    }
    buf[i]='\0';
}

