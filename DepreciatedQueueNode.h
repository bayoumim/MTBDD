////////////////////////////////////////////////////////////////////////////////////////////////////
// this class define the structure of the node in raw 
////////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef _DEPRECIATED_QUEUE_NODE_CLASS
#define _DEPRECIATED_QUEUE_NODE_CLASS

#include "DepreciatedRequestKey.h"

class DepreciatedQueueNode {
protected:
	DepreciatedRequestKey key;
	unsigned int hopInfo;
public:
	// constructors
	DepreciatedQueueNode(){
		//BDDNode(0,0,0,0);
	}

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

	// get and set functions for Hop Informations
	unsigned int getHopInfo(){
		return this->hopInfo;
	}
	void setHopInfo(unsigned int hopInfo){
		this->hopInfo = hopInfo;
	}

	void DumpToScreen(){
		cout << "Dump BDD Node" << endl;
		cout << "dump its key" << endl;
		key.DumpToScreen();
		cout << endl<< endl;
	}
};

#endif 
