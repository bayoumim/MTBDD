////////////////////////////////////////////////////////////////////////////////////////////////////
// this class define the structure of the node in raw 
////////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef _DEPRECIATED_QUEUE_NODE_AFTER_REDUCE_CLASS
#define _DEPRECIATED_QUEUE_NODE_AFTER_REDUCE_CLASS

#include "DepreciatedRequestKey.h"
#include "DepreciatedBDDPointer.h"

class DepreciatedQueueNodeAfterReduce  {
protected:
	DepreciatedRequestKey key;
	DepreciatedBDDPointer forwardedNode;
	unsigned int hopInfo;
public:
	// constructors
	DepreciatedQueueNodeAfterReduce(){}

	DepreciatedRequestKey getKey(){
		return this->key;
	}

	// set and get for F
	DepreciatedBDDPointer getF(){
		return this->key.getF();
	}
	void setF(DepreciatedBDDPointer& F){
		key.setF(F);
	}

	// set and get for G
	DepreciatedBDDPointer  getG(){
		return key.getG();
	}
	void setG(DepreciatedBDDPointer & G){
		key.setG(G);
	}

	// set and get for h
	DepreciatedBDDPointer getH(){
		return key.getH();
	}
	void setH(DepreciatedBDDPointer & H){
		key.setH(H);
	}

	// get and set functions for Forwarded node
	DepreciatedBDDPointer getForwardedNode() {
		return forwardedNode;
	}
	void setForwardedNode( DepreciatedBDDPointer & bp){
		forwardedNode = bp;
	}

	// get and set functions for Hop Informations
	unsigned int getHopInfo(){
		return this->hopInfo;
	}
	void setHopInfo(unsigned int hopInfo){
		this->hopInfo = hopInfo;
	}
};

#endif 
