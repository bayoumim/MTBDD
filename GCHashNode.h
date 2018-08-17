////////////////////////////////////////////////////////////////////////////////////////////////////
// this class define the structure of the node in GCHashNode
////////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef _GC_HASH_NODE_CLASS
#define _GC_HASH_NODE_CLASS

#include "BDDPointer.h"
template<typename T,typename V>	// T, V : are the templates for bdd pointer field in this clas
class GCHashNode {
protected:
	BDDPointer <T,V> BP;

	int count;
public:
	// constructors
	GCHashNode(){
		//count = 0;
	}
	// set and get functions for BP
	void setBP(BDDPointer  <T,V> BP){
		this->BP = BP;
	}
	BDDPointer  <T,V> getBP(){
		return this->BP;
	}
	// get and set functions for count
	int getCount(){
		return this->count;
	}
	void setCount(int count){
		this->count = count;
	}
};

#endif 
