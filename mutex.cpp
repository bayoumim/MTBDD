

#include <cerrno>
#include "mutex.h"

Mutex::Mutex()
      :m_pmutex(0),
       m_isOwner(true)
{
    m_pmutex = new pthread_mutex_t;
    if (pthread_mutex_init(m_pmutex, 0) == 0)
    {
        //ok
    }
}

Mutex::~Mutex()
{
    if (m_isOwner)
    {
        if (pthread_mutex_destroy(m_pmutex) == EBUSY)
        {
            unlock();
            pthread_mutex_destroy(m_pmutex);
        }
        delete m_pmutex;
    }
}

Mutex::Mutex(const Mutex& copy)
{
    m_pmutex = copy.m_pmutex;
    m_isOwner = false;
}

bool Mutex::lock()
{
    if (pthread_mutex_lock(m_pmutex) == 0)
        return true;
    return false;
}

bool Mutex::unlock()
{
    if (pthread_mutex_unlock(m_pmutex) == 0)
        return true;
    return false;
}

bool Mutex::tryLock()
{
    if (pthread_mutex_trylock(m_pmutex) == 0)
        return true;
    return false;
}




