////////////////////////////////////////////////////////////////////////////////////////////////////
// this class define the structure of the node in BBD
////////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef _DEPRECIATED_BBD_NODE_CLASS
#define _DEPRECIATED_BBD_NODE_CLASS

#include "DepreciatedNodeKey.h"
#include "DepreciatedBDDPointer.h"

class DepreciatedBDDNode {
protected:
	DepreciatedBDDPointer id;
	DepreciatedNodeKey key;
public:
	// constructors
	DepreciatedBDDNode(){
		//BDDNode(0,0,0,0);
	}
	DepreciatedBDDNode(int level, int Then, int Else, int id){
		setId(id);
		setElse(Else);
		setThen(Then);
		setLevel(level);
	}
	DepreciatedBDDNode(DepreciatedNodeKey & key,int id){
		setId(id);
		setKey(key);
	}
	// set and get functions for key
	void setKey(DepreciatedNodeKey &key){
		setElse(key.getElse());
		setThen(key.getThen());
		setLevel(key.getLevel());
	}
	DepreciatedNodeKey getKey(){
		return this->key;
	}
	// get and set functions for ID
	int getId(){
		return this->id.getPointer();
	}
	void setId(int id){
		this->id.setPointer(id);
	}
	// get and set functions for Else
	void setElse(int Else){
		this->key.setElse(Else);
	}
	int getElse(){
		return this->key.getElse();
	}
	void setElse(DepreciatedBDDPointer &Else){
		this->key.setElse(Else);
	}
	DepreciatedBDDPointer getElseP(){
		return this->key.getElseP();
	}
	// get and set functions for Then
	void setThen(int Then){
		this->key.setThen(Then);
	}
	int getThen(){
		return this->key.getThen();
	}
	void setThen(DepreciatedBDDPointer & Then){
		this->key.setThen(Then);
	}	
	DepreciatedBDDPointer getThenP(){
		return this->key.getThenP();
	}
	// get and set functions for Level
	void setLevel(int level){
		this->key.setLevel(level);
	}
	int getLevel(){
		return key.getLevel();
	}
	void Dump(ofstream & ff){
		id.Dump(ff);
		key.Dump( ff);
		ff << showbase <<"id : " << hex << getId() << setbase (10)<< endl ;
	}
	
	DepreciatedBDDPointer getIdP() {
		return id;
	}
	void setIdP( DepreciatedBDDPointer & bp){
		id = bp;
	}
	void setThenBucket(unsigned int buck){
		key.setThenBucket(buck);
	}
	void setElseBucket(unsigned int buck){
		key.setElseBucket(buck);
	}

	void setIdBucket(unsigned int buck){
		id.setBucket(buck);
	}

	void DumpToScreen(){
		cout << "Dump BDD Node" << endl;
		cout << "dump its id" << endl;
		id.DumpToScreen();
		cout << "dump its key" << endl;
		key.DumpToScreen();
		cout << endl<< endl;
	}
};

#endif 
