
#ifndef _CONDITION_CLASS_
#define _CONDITION_CLASS_
#include "BDDUtil.h"
#include "mutex.h"

class Condition
{
private:
	Mutex * LookUpTableLock; 
	pthread_cond_t m_pcond;
public:
	Condition(Mutex * LookUpTableLock){
		pthread_cond_init(&m_pcond, 0);
		this->LookUpTableLock = LookUpTableLock;
	}

    	void notifyAll(){
//		LookUpTableLock->lock();
		pthread_cond_broadcast(&m_pcond);
//		LookUpTableLock->unlock();
	}
	void wait(){
	//	LookUpTableLock->lock();
		pthread_cond_wait( &m_pcond, (LookUpTableLock->m_pmutex) );
	//	LookUpTableLock->unlock();
	}
};

/*class Condition
{
private:
	HybridLock * LookUpTableLock; 
	pthread_cond_t m_pcond;
public:
	Condition(HybridLock * LookUpTableLock){
		pthread_cond_init(&m_pcond, 0);
		this->LookUpTableLock = LookUpTableLock;
	}

    	void notifyAll(){
		LookUpTableLock->Lock();
		pthread_cond_broadcast(&m_pcond);
		LookUpTableLock->Unlock();
	}
	void wait(){
		LookUpTableLock->Lock();
		pthread_cond_wait( &m_pcond, &(LookUpTableLock->lock) );
		LookUpTableLock->Unlock();
	}
};*/

#endif // _CONDITION_H_


