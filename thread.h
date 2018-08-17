
#ifndef _THREAD_H_
#define _THREAD_H_
#include "BDDUtil.h"

class Thread
{
public:
    Thread(int id = 0);
    virtual ~Thread();

    virtual void start();
    virtual void stop();
    virtual void join();
    bool         isRunning();
    virtual void run() = 0;

private:
    //private copy-ctor; prevent copied
    Thread(const Thread&);
    //private assignment operator; prevent copied
    Thread& operator=(const Thread&);

    friend void *_threadFunc(void *);

protected:
    bool              m_running;

private:
    pthread_t         m_thread;
    pthread_attr_t    m_attr;
    int               m_threadid;

};

#endif // _THREAD_H_


