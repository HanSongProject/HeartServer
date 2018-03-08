#ifndef SINGLETON_H
#define SINGLETON_H

#include <memory>
#include <pthread.h>


using namespace std;

template <class T>
class Singleton
{

public:
    static inline T* instance();

private:

    Singleton(void){}

    ~Singleton(void){}

    Singleton(const Singleton&){}

    Singleton & operator= (const Singleton &){}

    static auto_ptr<T> _instance;

};

template <class T>
auto_ptr<T> Singleton<T>::_instance;



template <class T>


inline T* Singleton<T>::instance()
{
    static pthread_mutex_t mutex;

    if( 0 == _instance.get())
    {
        pthread_mutex_init(&mutex,NULL);
        pthread_mutex_lock(&mutex);
        _instance.reset ( new T);
        pthread_mutex_unlock(&mutex);
    }
    return _instance.get();
}

#define DECLARE_SINGLETON_CLASS( type ) \
       friend class auto_ptr< type >;\
       friend class Singleton< type >;



#endif // SINGLETON_H
