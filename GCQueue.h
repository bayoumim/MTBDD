// this class implement a queue that store a unique set of keys (so it is some how very close to hash table)

#ifndef _GC_QUEUE_CLASS
#define _GC_QUEUE_CLASS

#include "BDDUtil.hh"
#include "BDDPointer.h"
#include "GCHashNode.h"
#include "IDGenerator.h"

class GCQueue{ 
private:
	vector < BDDPointer > *RefList;
	vector < BDDPointer > *DeRefList;
	vector < BDDPointer > *RefListTemp;
	vector < BDDPointer > *DeRefListTemp;
	Mutex RefLock; 
	Mutex DeRefLock; 
	IDGenerator *idGen;

	// this function add "count" to RC of the pointer bp
	void ModifyReferenceCount(BDDPointer bp, int count){
		if (bp.getPointer() == 0x3C00002D){
			cout << "GCQueue : ModifyReferenceCount:  node with id : "<< bp.getPointer()  << " : count : " << count  << endl;
			
		}
		if (idGen->isInverted(bp))	// is the pointer is inverted, then regulate it.
			bp = idGen->invert(bp);

		if(count == 1){
			RefLock.lock();
			RefList ->push_back(bp);
			RefLock.unlock();
		}
		else if (count == -1){
			DeRefLock.lock();
			DeRefList ->push_back(bp);
			DeRefLock.unlock();
		}
		else
			cout << "GCQueue : ModifyReferenceCount: ERROR" << endl;
			
	}
public:
	// constructor
	GCQueue() {
	}
	GCQueue(IDGenerator *idGen) {
		RefList = new vector < BDDPointer >();
		DeRefList = new vector < BDDPointer >();
		this->idGen = idGen;
	}
	// reference pointer
	void Ref(BDDPointer bp){
		ModifyReferenceCount(bp,1);
	}
	// DeReference pointer
	void DeRef(BDDPointer bp){
		ModifyReferenceCount(bp,-1);
	}
	// this function get the next node to be GC
	bool GetNextPointer(GCHashNode &gcNode){
		int loc;
		// first , you have to Ref pointer, then dereference it (So taht , no node can be GCed by accident) -- order here is MUST
		RefLock.lock();
		if(!RefList->empty()){	// IF reference list contains an element								
			loc = RefList->size()-1;
			gcNode.setBP((*RefList)[loc]);	// get the last element in Reference list
			gcNode.setCount(1);			// store 1								
			RefList->erase(RefList->end()-1);	// remove the last element
			RefLock.unlock();	// unlock
			return true;					
		}									
		RefLock.unlock();	// unlock
									
		DeRefLock.lock();
		if(!DeRefList->empty()){	// IF reference list contains an element								
			gcNode.setBP((*DeRefList)[DeRefList->size()-1]);	// get the last element in Reference list
			gcNode.setCount(-1);			// store 1								
			DeRefList->erase(DeRefList->end()-1);	// remove the last element
			DeRefLock.unlock();	// unlock
			return true;					
		}									
		DeRefLock.unlock();	// unlock									
											
		return false; // if there is no other elements									
	}
	int getSize(){
		return DeRefList->size() + RefList->size();
	}
	// Prepare the queue for mapping its pointer as the unique table is doing resizing
	void ResetQueueForHashResizing(){
		RefListTemp = new vector < BDDPointer >();
		DeRefListTemp = new vector < BDDPointer >();
	}
	// finalize queue, by update it with new list and new hashTable
	void FinalizeQueueForHashResizing(){
		delete RefList;	// remove old ref list 
		delete DeRefList;	// remove old Deref list
		RefList = RefListTemp;	// update the new list
		DeRefList = DeRefListTemp;	// update the new hash Table
	}
	// finalize queue, by update it with new list and new hashTable
	void InsertNode(GCHashNode &gcNode){		
		// store the old one temporary
		vector < BDDPointer > * RefListTemp1 = RefList;	
		vector < BDDPointer > * DeRefListTemp1 = DeRefList;	
		// set the list and hash table with the new one
		RefList = RefListTemp;	// update the new list
		DeRefList = DeRefListTemp;	// update the new hash Table

		// store the node and its count in the new table
		ModifyReferenceCount(gcNode.getBP(),gcNode.getCount());

		// set the list and hash table with the old one again
		RefList = RefListTemp1;	// update the new list
		DeRefList = DeRefListTemp1;	// update the new hash Table
	}
	// check whether the reference list has more elements
	bool isRefListHasElements(){
		bool result;
		RefLock.lock();
		result = !RefList->empty();
		RefLock.unlock();
		return result;						
	}
};
#endif 
