////////////////////////////////////////////////////////////////////////////////////////////////////
// this class implements the read/write lock using pthreads
// the code is inspired from the code implemented in Section 7.8 of the book:
// "Introduction to Parallel Computing, Second Edition"
// By Ananth Grama, Anshul Gupta, George Karypis, Vipin Kumar
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _RW_LOCK_CLASS
#define _RW_LOCK_CLASS

#include "BDDUtil.h"
//#include "mutex.h"


class RWLock{
private:
    pthread_rwlock_t lock ;
    int counter;
    Mutex mutex;
public:
    RWLock(){
        counter = 0 ;
        Init();
    }
    void Init(){
        pthread_rwlock_init(&lock,NULL);
    }
    void ReadLock(){
        pthread_rwlock_rdlock(&lock);
        mutex.lock();
        counter++;
        mutex.unlock();
    }
    void WriteLock(){
        pthread_rwlock_wrlock(&lock);
        mutex.lock();
        counter++;
        mutex.unlock();
    }
/*       int TryReadLock(){
        return pthread_rwlock_tryrdlock(&lock);
    }
    int TryWriteLock(){
        return pthread_rwlock_trywrlock(&lock);
    }*/
    void Unlock(){
        pthread_rwlock_unlock(&lock);
        mutex.lock();
        counter--;
        if (counter <0){
            cout << "RW lock : unlock : error in unlocking" << endl;
            mutex.unlock();
            exit(-1);
        }
        mutex.unlock();
    }
    void DumpNoOfLocks(){
            //cout << "no. of locked " << counter << endl;

    }
    void CheckConsistancy(){
   /*     if(counter !=0){
            cout << "RW lock : unlock : error in unlocking 2" << endl;
            exit(-1);
        }*/
    }
    void CheckConsistancyWithoutLock(){
        if(counter !=0){
            cout << "RW lock : unlock : error in unlocking 3" << endl;
            cout << "counter : " << counter << endl;
            exit(-1);
        }
    }
/*	int getCounter(){
		int i = 0;
		mutex.lock();	
		i = counter;
		mutex.unlock();
		return i;
	}*/
};


#endif 



