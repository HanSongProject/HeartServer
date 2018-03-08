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
    void* (*callback_function)(void *arg);    //�̻߳ص�����
    void *arg;                                //�ص���������
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
    uint thread_init_num;              //��ʼ�߳���
    uint thread_num;                   //�̳߳��п����̵߳ĸ���
    uint queue_max_num;                //���������task�ĸ���
    struct task *task_list;            //ָ��task��βָ��
    pthread_t destroy;                 //���������߳�
    thread_info_t *pthread_list;      //�̳߳��������̵߳�pthread_t
    pthread_mutex_t mutex;            //�����ź���
    pthread_cond_t queue_empty;       //����Ϊ�յ���������
    pthread_cond_t queue_not_empty;   //���в�Ϊ�յ���������
    pthread_cond_t queue_not_full;    //���в�Ϊ������������
    int queue_cur_num;                //���е�ǰ��task����
    int queue_close;                  //�����Ƿ��Ѿ��ر�
    int pool_close;                   //�̳߳��Ƿ��Ѿ��ر�
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
