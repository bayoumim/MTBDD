#include "thread.h"

void *_threadFunc(void *obj)
{
    void *retval = 0;
    Thread *thread = static_cast<Thread *>(obj);
	//cout<< "thread func called\n";
    thread->m_running = true;
    thread->run();
    thread->m_running = false;
//    fprintf(stderr, "ThreadFunc#%d exit\n", thread->m_threadid);
    pthread_exit(NULL);
    return retval;
}


Thread::Thread(int id)
       :m_running(false),
        m_threadid(id)
{
    size_t stacksize;
//pthread_init();
    pthread_attr_init(&m_attr);
    pthread_attr_getstacksize(&m_attr, &stacksize);
    stacksize *= 2;
    pthread_attr_setstacksize(&m_attr, stacksize);
//   fprintf(stderr, "Thread#%d stack size %d\n", m_threadid, stacksize);
}

Thread::~Thread()
{
    /*
    if (m_running)
        pthread_cancel(m_thread);
        */
    /*
    if (m_running)
        stop();
    pthread_attr_destroy(&m_attr);
    */
}

void Thread::start()
{
//    fprintf(stderr, "Thread#%d start() running=%d\n", m_threadid, m_running);
    if (m_running)
        return;

    if (pthread_create(&m_thread, &m_attr, _threadFunc, static_cast<void *>(this)) == 0)
    {	
//		cout<<"thread created successfully\n";
    }
}

void Thread::stop()
{
    fprintf(stderr, "Thread#%d stop() running=%d\n", m_threadid, m_running);
    //pthread_cancel(m_thread);
    m_running = false;
}

void Thread::join()
{
    //wait for the running thread to exit
    void *retValue;
   // fprintf(stderr, "Thread#%d join() running=%d\n", m_threadid, m_running);
    //pthread_attr_destroy(&m_attr);
    m_running = false;
    if (pthread_join(m_thread, &retValue) == 0)
    {
    }
}

bool Thread::isRunning()
{
    return m_running;
}





