#include "threadpool.h"
#include "global.h"
#include <unistd.h>



ThreadPool::ThreadPool()
{

}

//================================================================================================
//��������                   thread_pool_init
//����������                 ��ʼ���̳߳�
//���룺                    [in] thread_num     �̳߳ؿ������̸߳���
//                         [in] queue_num  ���е����task����
//�����                    ��
//���أ�                    �ɹ����̳߳ص�ַ ʧ�ܣ�NULL
//================================================================================================
struct threadpool_s* ThreadPool::thread_pool_init(uint thread_num, uint queue_num)
{
    struct threadpool_s *pool = NULL;
    //pool = NULL;
    do
    {
        pool = (struct threadpool_s*)malloc(sizeof(struct threadpool_s));
        if(NULL == pool)
        {
            printMsg("thread pool create fail");
            break;
        }
        pool->queue_cur_num = 0;
        pool->thread_init_num = thread_num;
        pool->thread_num = thread_num;
        pool->queue_max_num = queue_num;
        pool->pthread_list = NULL;
        pool->task_list = NULL;

        if(pthread_mutex_init(&pool->mutex,NULL))
        {
            printMsg("thread pool mutex fail");
            break;
        }
        if(pthread_cond_init(&pool->queue_empty,NULL))
        {
            printMsg("thread pool queue_empty mutex fail");
            break;
        }
        if(pthread_cond_init(&pool->queue_not_empty,NULL))
        {
            printMsg("thread pool queue_not_empty mutex fail");
            break;
        }
        if(pthread_cond_init(&pool->queue_not_full,NULL))
        {
            printMsg("thread pool queue_not_full fail");
            break;
        }

        pool->queue_close = 0;
        pool->pool_close = 0;

        int i = 0;
        thread_info_t *temp = NULL;
        for(i = 0; i < thread_num; i++)
        {
            temp = (thread_info_t*)malloc(sizeof(thread_info_t));
            if(NULL == temp)
            {
                --i;
                continue;
            }
            else
            {
                temp->next = pool->pthread_list;
                pool->pthread_list = temp;

                pthread_create(&(temp->id), NULL, (void* (*)(void*))threadpool_function, (void *)pool);
                temp->status = THREAD_STATUS_NEW;
                printMsg("pthread id: %lu",temp->id);
            }

        }
        pthread_create(&(pool->destroy),NULL,(void* (*)(void*))thread_pool_is_need_delete,(void*)pool);

        return pool;

    }while(0);

    return NULL;
}

//================================================================================================
//��������                    thread_pool_is_need_add
//����������                  ��չ�߳�
//���룺                     [in] pool                  �̳߳ص�ַ
//�����                     ��
//���أ�                     �ɹ�: ʧ��:
//================================================================================================
void ThreadPool::thread_pool_is_need_add(void *arg)
{
    threadpool *pool = (threadpool*)arg;
    if((float)pool->thread_num/pool->queue_cur_num >= THREAD_BUSY_PERCENT)
        return;
    int addnum = pool->queue_cur_num * THREAD_BUSY_PERCENT - pool->thread_num;
    if(addnum + pool->thread_num > MAX_ACTIVE_THREADS)
    {
        addnum = MAX_ACTIVE_THREADS - pool->thread_num;
    }

    pthread_mutex_lock(&pool->mutex);
    thread_info_t *temp = NULL;
    //ѭ������ָ����Ŀ���̣߳����ܳ�������߳���
    for(int i = 0;i < addnum;i++)
    {
        //�����̣߳����ж��Ƿ񴴽��ɹ���ʧ����ʾ���˳��ú���
        temp = (thread_info_t*)malloc(sizeof(thread_info_t));
        if(NULL == temp)
        {
            continue;
        }
        else
        {
            temp->next = pool->pthread_list;
            pool->pthread_list = temp;

            pool->thread_num++;
            pthread_create(&(temp->id), NULL, (void* (*)(void*))threadpool_function, (void *)pool);
            temp->status = THREAD_STATUS_NEW;
        }
    }
    pthread_mutex_unlock(&pool->mutex);

}


//================================================================================================
//��������                   thread_pool_is_need_delete
//����������                  �ָ���ʼ�̸߳���
//���룺                     [in] pool                  �̳߳ص�ַ
//�����                     ��
//���أ�                     �ɹ�: ʧ��:
//================================================================================================
void ThreadPool::thread_pool_is_need_delete(void *arg)
{
    threadpool *pool = (threadpool*)arg;
    thread_info_t *tmp = NULL,*p = NULL, *prev = NULL;
    while(1)
    {
        if((float)pool->thread_num/pool->queue_cur_num >= THREAD_BUSY_PERCENT && (pool->thread_num > pool->thread_init_num))
        {
            sleep(10);
            if((float)pool->thread_num/pool->queue_cur_num >= THREAD_BUSY_PERCENT && (pool->thread_num > pool->thread_init_num))
            {
                pthread_mutex_lock(&pool->mutex);
                tmp = pool->pthread_list;
                prev = NULL;  //�Ƿ���ͷָ��
                while(pool->thread_init_num < pool->thread_num && !tmp)
                {
                    if(tmp->status == THREAD_STATUS_WAIT)
                    {
                        tmp->status = THREAD_STATUS_EXIT;

                        pthread_cancel(tmp->id);
                        if(prev)
                        {
                            prev->next = tmp->next;
                        }
                        else
                        {
                            pool->pthread_list = tmp->next;
                        }
                        p = tmp;
                        tmp = tmp->next;
                        free(p);
                        pool->thread_num--;
                        continue;
                    }
                    prev = tmp;
                    tmp = tmp->next;
                }
                pthread_mutex_unlock(&pool->mutex);

            }
        }
        sleep(10);
    }
}

//================================================================================================
//��������                    get_thread_by_id
//����������                  ͨ���߳�id,�ҵ���Ӧ���߳�
//���룺                     [in] pool                  �̳߳ص�ַ
//                          [in] id                     �߳�id
//�����                     ��
//���أ�                     �ɹ����߳���Ϣ ʧ�ܣ�NULL
//================================================================================================
thread_info_t *ThreadPool::get_thread_by_id(threadpool *pool, pthread_t id)
{
    thread_info_t *temp = pool->pthread_list;
    while(temp != NULL)
    {
        if(id == temp->id)
            return temp;
        temp = temp->next;
    }
    return NULL;
}

//================================================================================================
//��������                    threadpool_add_task
//����������                  ���̳߳����������
//���룺                     [in] pool                  �̳߳ص�ַ
//                          [in] callback_function     �ص�����
//                          [in] arg                     �ص���������
//�����                     ��
//���أ�                     �ɹ���0 ʧ�ܣ�-1
//================================================================================================
int ThreadPool::threadpool_add_task(threadpool *pool, void *(*callback_function)(void *), void *arg)
{
    if(pool == NULL)
    {
        printMsg("pool is null");
        return -1;
    }
    if(callback_function == NULL)
    {
        printMsg("callback_function is null");
        return -1;
    }
    if(arg == NULL)
    {
        printMsg("arg is null");
        return -1;
    }
    pthread_mutex_lock(&(pool->mutex));
    while ((pool->queue_cur_num == pool->queue_max_num) && !(pool->queue_close || pool->pool_close))
    {
        pthread_cond_wait(&(pool->queue_not_full), &(pool->mutex));   //��������ʱ��͵ȴ�
    }
    if (pool->queue_close || pool->pool_close)    //���йرջ����̳߳عرվ��˳�
    {
        pthread_mutex_unlock(&(pool->mutex));
        return -1;
    }
    struct task *pjob =(struct task*) malloc(sizeof(struct task));
    if (NULL == pjob)
    {
        pthread_mutex_unlock(&(pool->mutex));
        return -1;
    }
    pjob->callback_function = callback_function;
    pjob->arg = arg;
    pjob->next = NULL;
    if (pool->task_list == NULL)
    {
        pool->task_list = pjob;
        pthread_cond_broadcast(&(pool->queue_not_empty));  //���пյ�ʱ����������ʱ��֪ͨ�̳߳��е��̣߳����зǿ�
    }
    else
    {
        struct task *tmp=pool->task_list;
        while(tmp->next!=NULL)
            tmp=tmp->next;
        tmp->next=pjob;

    }
    pool->queue_cur_num++;
    pthread_mutex_unlock(&(pool->mutex));
    return 0;
}

//================================================================================================
//��������                    threadpool_function
//����������                  �̳߳����̺߳���
//���룺                     [in] arg                  �̳߳ص�ַ
//�����                     ��
//���أ�                     ��
//================================================================================================
void ThreadPool::threadpool_function(void * arg)
{
    struct threadpool_s *pool = (struct threadpool_s*)arg;
    thread_info_t *threadInfo = NULL;
    struct task *ptask = NULL;

    while(1)
    {
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
        pthread_cleanup_push(handler,pool);
        pthread_mutex_lock(&pool->mutex);

        threadInfo = get_thread_by_id(pool,pthread_self());
        if(threadInfo)
            threadInfo->status = THREAD_STATUS_WAIT;
        while(0 == pool->queue_cur_num && !pool->pool_close)//����Ϊ��ʱ���͵ȴ����зǿ�
        {
            pthread_cond_wait(&pool->queue_not_empty,&pool->mutex);
        }
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
        if (pool->pool_close)   //�̳߳عرգ��߳̾��˳�
        {
            if(threadInfo)
                threadInfo->status = THREAD_STATUS_EXIT;
            pthread_mutex_unlock(&(pool->mutex));
            pthread_exit(NULL);
        }
        if(threadInfo)
            threadInfo->status = THREAD_STATUS_RUNNING;
        pool->queue_cur_num--;
        ptask = pool->task_list;
        pool->task_list = ptask->next;

        if (pool->queue_cur_num == 0)
        {
            pthread_cond_signal(&(pool->queue_empty));        //����Ϊ�գ��Ϳ���֪ͨthreadpool_destroy�����������̺߳���
        }
        if (pool->queue_cur_num == pool->queue_max_num - 1)
        {
            pthread_cond_broadcast(&(pool->queue_not_full));  //���з������Ϳ���֪ͨthreadpool_add_job���������������
        }
        pthread_mutex_unlock(&(pool->mutex));
        pthread_cleanup_pop(0);

        (*(ptask->callback_function))(ptask->arg);   //�߳�����Ҫ���Ĺ������ص������ĵ���
        free(ptask);
        ptask = NULL;
    }

}

void ThreadPool::handler(void *arg)
{
    struct threadpool_s *pool = (struct threadpool_s*)arg;
    pthread_mutex_unlock(&pool->mutex);
}

//================================================================================================
//��������                    thread_pool_destory
//����������                   �����̳߳�
//���룺                      [in] pool                  �̳߳ص�ַ
//�����                      ��
//���أ�                      �ɹ���0 ʧ�ܣ�-1
//================================================================================================
int ThreadPool::thread_pool_destory(void *arg)
{
    threadpool *pool = (threadpool*)arg;
    if(pool != NULL)
        return -1;
    pthread_mutex_lock(&(pool->mutex));
    if (pool->queue_close || pool->pool_close)   //�̳߳��Ѿ��˳��ˣ���ֱ�ӷ���
    {
        pthread_mutex_unlock(&(pool->mutex));
        return -1;
    }

    pool->queue_close = 1;        //�ö��йرձ�־
    while (pool->queue_cur_num != 0)
    {
        pthread_cond_wait(&(pool->queue_empty), &(pool->mutex));  //�ȴ�����Ϊ��
    }

    pool->pool_close = 1;      //���̳߳عرձ�־
    pthread_mutex_unlock(&(pool->mutex));
    pthread_cond_broadcast(&(pool->queue_not_empty));  //�����̳߳��������������߳�
    pthread_cond_broadcast(&(pool->queue_not_full));   //������������threadpool_add_job����
    int i;
    for (i = 0; i < pool->thread_num; ++i)
    {
        pthread_join(pool->pthread_list[i].id, NULL);    //�ȴ��̳߳ص������߳�ִ�����
    }

    pthread_mutex_destroy(&(pool->mutex));          //������Դ
    pthread_cond_destroy(&(pool->queue_empty));
    pthread_cond_destroy(&(pool->queue_not_empty));
    pthread_cond_destroy(&(pool->queue_not_full));
    free(pool->pthread_list);
    struct task *p;
    while (pool->task_list != NULL)
    {
        p = pool->task_list;
        pool->task_list = p->next;
        free(p);
    }
    free(pool);
    return 0;
}

void ThreadPool::work_threadpool(void *arg)
{
    char *s = (char*)arg;
    printf("printf value: %s\n",s);
}

