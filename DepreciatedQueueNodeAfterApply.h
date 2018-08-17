////////////////////////////////////////////////////////////////////////////////////////////////////
// this class define the structure of the node in raw 
////////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef _DEPRECIATED_QUEUE_NODE_AFTER_APPLY_CLASS
#define _DEPRECIATED_QUEUE_NODE_AFTER_APPLY_CLASS

#include "DepreciatedRequestKey.h"
#include "DepreciatedBDDPointer.h"

class DepreciatedQueueNodeAfterApply  {
protected:
	DepreciatedRequestKey key;
	DepreciatedBDDPointer id;
	DepreciatedBDDPointer RThen;
	DepreciatedBDDPointer RElse;
	
	unsigned int hopInfo;
public:
	// constructors
	DepreciatedQueueNodeAfterApply(){}

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

	// get and set functions for ID
	DepreciatedBDDPointer getIdP() {
		return id;
	}
	void setIdP( DepreciatedBDDPointer & bp){
		id = bp;
	}

	// get and set functions for Else
	void setRElse(DepreciatedBDDPointer &Else){
		RElse = Else;
	}
	DepreciatedBDDPointer getRElse(){
		return RElse;
	}

	// get and set functions for Then
	void setRThen(DepreciatedBDDPointer & Then){
		RThen = Then;
	}	
	DepreciatedBDDPointer getRThen(){
		return RThen;
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
