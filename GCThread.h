#ifndef _GC_THREAD_CLASS
#define _GC_THREAD_CLASS

#include "GCQueue.h"
#include "Hash.hh"
#include "thread.h"
#include "Barrier.h"
#include "OperationSchedular.h"

class GCThread : public Thread {
protected:
	Barrier *GCBarrier;
	GCQueue * gcQueue;
	Hash* HashTable;	// pointer to hash table
	OperationSchedular *opSchd;
	int GCEnabled;
	Queue * queue;
/////////////////////////////////////////////////////////////////////////////////////////////////////
//				PRIVATE FUNCTIONS
/////////////////////////////////////////////////////////////////////////////////////////////////////

public:
	GCThread () : Thread(){

	}
	GCThread(int id, Hash * &HashTable, GCQueue * gcQueue, Barrier *GCBarrier,int GCEnabled, Queue * queue ) : Thread(id) {
		this->GCBarrier = GCBarrier;
		this->HashTable = HashTable;
		this->gcQueue = gcQueue;
		this->GCEnabled = GCEnabled;
		this->queue = queue;
		start();	// run threads
	}
	void run(){
		// add your code here	
		// the idea in GC thread is that, it take the referenced pointers first (so that no node will be dereferenced by accident), and it ensure that by adding a barrier between 'referenced' and 'dereferenced' operations. note thaat the first loop will continue until the referenced pointers are finished, then teh second loop starts to continue
		// wait for all threads to be created
		GCHashNode gcNode;
		do {
			if (GCEnabled == H_GC_RF_ENABLED){	// if GC (RC) is enabled then; allow GC threads to do its duty
				GCBarrier -> wait();	// GC barrier (now GC threads can run)
				while(gcQueue->isRefListHasElements()){	// as long as there is request are not processed yet, keep executing Apply and reduce methods
					if(!gcQueue->GetNextPointer(gcNode))	// if the list is empty, this is not true, as we suppose to have  aremaining elements in reference list
						cout << "GCThread : run, error" << endl;

					if (gcNode.getCount() != 0){// if reference counter is equal to zero, then skip this pointer
						HashTable->UpdateRC(gcNode);	// pointer to hash table
					}
				};
				GCBarrier -> wait();	// GC barrier (now referenced pointers are finished; you can Dereference the 'dereference pointers')
				while(gcQueue->GetNextPointer(gcNode)){	// as long as there is request are not processed yet, keep executing Apply and reduce methods
					if (gcNode.getCount() != 0){// if reference counter is equal to zero, then skip this pointer
						HashTable->UpdateRC(gcNode);	// pointer to hash table
					}
				};
				GCBarrier -> wait();	// GC barrier (now GC threads finish)
			}
			else if (GCEnabled == H_GC_MARK_ENABLED) {	// if 'mark' GC
				GCBarrier -> wait();	// GC barrier (now GC threads can run)
				GCBarrier -> wait();	// GC barrier (operation schedular is now loaded)
				BDDPointer bp;
				// update pointers in 'gatelist' and in 'operation' list
				while(opSchd->getNextLivePointer(bp)){	// as long as there is request are not processed yet, keep executing Apply and reduce methods
					HashTable->UpdateMark(bp);	// pointer to hash table
					
				};
				// update pointers in 'queues' (F,G,H) and 'then', 'else' if they are processed. Note that, forwarded requests will not be marked (as they are updated round-by round. In other words, there requests are dated)
				while(queue->getNextLivePointer(bp)){	// as long as there is request are not processed yet, keep executing Apply and reduce methods
			//		cout << "GCThread : marking is ongoing" << endl;
					HashTable->UpdateMark(bp);	// pointer to hash table
					
				};
				GCBarrier -> wait();	// GC barrier (now GC threads finish)
			}
			else if (GCEnabled == H_GC_INCREMENTAL_MARK_ENABLED) {	// if incremental 'mark' GC
				GCBarrier -> wait();	// GC barrier (now GC threads can run)
				GCBarrier -> wait();	// GC barrier (operation schedular is now loaded)
				BDDPointer bp;
				// update pointers in 'gatelist' and in 'operation' list
				bool PermenantFlag;
				while(opSchd->getNextLivePointer(bp,PermenantFlag)){	// as long as there is request are not processed yet, keep executing Apply and reduce methods
					
					HashTable->UpdateMark(bp,PermenantFlag);	// pointer to hash table
					
				};	
				// NOTE THAT: We will not mark any requests pointer; as this function is called after hybrid approach, and hence, there will be no requests in the queue.
				GCBarrier -> wait();	// GC barrier (now GC threads finish)
			}
		}
		while(true);
	}	
	// this function is called in 'manager' run function, it is used to pass 'operation schedular', so that, GC can access all active nodes when 'MARK' GC is enabled
	void addSchedular(OperationSchedular *opSchd) {
		this->opSchd = opSchd;
	}
};

#endif
