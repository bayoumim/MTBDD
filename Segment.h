#ifndef _SEGMENT_CLASS
#define _SEGMENT_CLASS

#include "RWLock.h"

class Segment {
private:
	//int volatile timestamp;
	RWLock lock;	// lock is automatically initialized
public:
	Segment() {
		//timestamp = 0;
	}
	void Init(){
	//	lock.Init();
	}
	// aquire read lock on the segment
	void ReadLock(){
		lock.ReadLock();
	}
	// aquire write lock on the segment
	void WriteLock(){
		lock.WriteLock();
	}
	// unlock
	void Unlock(){
		lock.Unlock();
	}		
	void DumpNoOfLocks(){
		lock.DumpNoOfLocks();

	}
        void checkSegmentsConsistancy(){
            lock.CheckConsistancyWithoutLock();
        }
/*	int getCounter(){
		return lock.getCounter();
	}*/
};

#endif
