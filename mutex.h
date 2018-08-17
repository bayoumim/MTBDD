
#ifndef _MUTEX_H_
#define _MUTEX_H_


#include <pthread.h>


class Mutex
{
public:
    friend class Condition;

    Mutex();
    virtual ~Mutex();
    Mutex(const Mutex& copy);

    bool lock();
    bool unlock();
    bool tryLock();


private:
    pthread_mutex_t      *m_pmutex;
    bool                  m_isOwner;

};

#endif // _MUTEX_H_


