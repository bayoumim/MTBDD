#ifndef _GC_HASH_BUCKET_CLASS
#define _GC_HASH_BUCKET_CLASS

#include "GCHashNode.h"

class GCHashBucket{
private:
	volatile unsigned int hopInfo;
	GCHashNode node;
	volatile char Flags;	// unused | empty; (empty: 0 - means empty, 1 means full);
public:
	GCHashBucket() {
		hopInfo = 0;
		Flags = 0;
	}
	// get and set GCHashNode
	void setNode(GCHashNode node){
		this->node = node;
		Flags = Flags | 0x1;
	}
	GCHashNode getNode () {
		return this->node;
	}
	// get and set functions for Hop Informations
	unsigned int getHopInfo(){
		return this->hopInfo;
	}
	void setHopInfo(unsigned int hopInfo){
		this->hopInfo = hopInfo;
		//Flags = Flags | 0x1;
	}
	BDDPointer getBP(){
		return node.getBP();
	}
	// get and set functions for count
	int getCount(){
		return node.getCount();
	}
	void setCount(int count){
		node.setCount(count);
	}

	bool isEmpty() {
		if ( (Flags & 1) == 0 )
			return true;
		else 				
			return false;
	}
	void setFlags(unsigned char Flags){
		this->Flags = Flags;
	}
	unsigned char getFlags(){
		return this->Flags;
	}
	void clearAllFlags() {
		Flags = 0;
	}

	bool isReserved() {
		if ( (Flags & 2) == 0 )
			return false;
		else 				
			return true;			
	}
	void setReserved() {
		Flags = Flags | 2;
	}
	void clearReserved(){
		Flags = Flags & 0xFD;
	}

};

#endif 
