#ifndef EDGE_QUEUE_CLASS
#define EDGE_QUEUE_CLASS
#include "BDDPointer.h"
template<typename T,typename V>
class EdgeQueue{
private:
	int head;
	vector < BDDPointer <T,V> > list0, list1;
	// enque the pointer
	void enque (BDDPointer <T,V> from, BDDPointer <T,V> to){
		list0.push_back(from);
		list1.push_back(to);
	}
public:
	EdgeQueue(){
	}

	// check if the element is exists or not, if not it enque the 
	bool isExistAndEnq (BDDPointer <T,V> from, BDDPointer <T,V> to){
		unsigned int i;
		for (i = 0 ; i < list0.size(); i++) {
			if (list0[i].isEqual(from) && list1[i].isEqual(to)) {
				return true;								
			}
		}
		enque (from,to);
		return false;
	}
/*	void reset() {
		head = -1;
	}*/
};

#endif
