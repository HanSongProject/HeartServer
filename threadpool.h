#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <pthread.h>
#include "singleton.h"


typedef enum{
    THREAD_STATUS_NEW = 0x00,
    THREAD_STATUS_WAIT,
    THREAD_STATUS_RUNNING,
    THREAD_STATUS_EXIT
}THREAD_STATUS;

struct task
{
    void* (*callback_function)(void *arg);    //线程回调函数
    void *arg;                                //回调函数参数
    struct task *next;
};
typedef struct thread_info_s
{
    pthread_t id;
    THREAD_STATUS status;
    struct thread_info_s *next;
}thread_info_t;

typedef struct threadpool_s
{
    uint thread_init_num;              //初始线程数
    uint thread_num;                   //线程池中开启线程的个数
    uint queue_max_num;                //队列中最大task的个数
    struct task *task_list;            //指向task的尾指针
    pthread_t destroy;                 //监听销毁线程
    thread_info_t *pthread_list;      //线程池中所有线程的pthread_t
    pthread_mutex_t mutex;            //互斥信号量
    pthread_cond_t queue_empty;       //队列为空的条件变量
    pthread_cond_t queue_not_empty;   //队列不为空的条件变量
    pthread_cond_t queue_not_full;    //队列不为满的条件变量
    int queue_cur_num;                //队列当前的task个数
    int queue_close;                  //队列是否已经关闭
    int pool_close;                   //线程池是否已经关闭
}threadpool;

class ThreadPool
{
public:
    ThreadPool();

    struct threadpool_s* thread_pool_init(uint thread_num,uint queue_num);
    void thread_pool_is_need_add(void *arg);
    static void thread_pool_is_need_delete(void *arg);
    static thread_info_t* get_thread_by_id(threadpool* pool,pthread_t id);

    int threadpool_add_task(threadpool *pool,void* (*callback_function)(void *arg),void *arg);
    static void threadpool_function(void*);
    static void handler(void *);
    int thread_pool_destory(void *arg);


    static void work_threadpool(void*arg);
    DECLARE_SINGLETON_CLASS(ThreadPool)
};
typedef Singleton<ThreadPool> SThreadPool;

#endif // THREADPOOL_H
