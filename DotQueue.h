#ifndef DOT_QUEUE_CLASS
#define DOT_QUEUE_CLASS
#include "BDDPointer.h"
class DotQueue{
private:
	int head;
	BDDPointer * list;
	// enque the pointer
	void enque (BDDPointer element){
		head ++;
		list[head] = element;
	}
public:
	DotQueue(int NodeNumber){
		head =  -1;
		list = new BDDPointer[NodeNumber];
	}

	// check if the element is exists or not, if not it enque the 
	bool isExistAndEnq (BDDPointer element){
		int i;
		for (i = 0 ; i <= head; i++) {
			if (list[i].isEqual(element)) {
				return true;
				
				
			}
		}
		enque (element);
		return false;
	}
	void reset() {
		head = -1;
	}
};

#endif
