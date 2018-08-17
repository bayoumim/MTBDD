////////////////////////////////////////////////////////////////////////////////////////////////////
// this class define the structure of the node in raw 
////////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef _DEPRECIATED_TABLE_NODE_CLASS
#define _DEPRECIATED_TABLE_NODE_CLASS

#include "DepreciatedNodeKey.h"
#include "DepreciatedBDDPointer.h"

class DepreciatedTableNode {
protected:
	DepreciatedBDDPointer id;
	DepreciatedNodeKey key;
	unsigned int hopInfo;
public:
	// constructors
	DepreciatedTableNode(){
		//BDDNode(0,0,0,0);
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
	unsigned int getId(){
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
	// get and set functions for Hop Informations
	unsigned int getHopInfo(){
		return this->hopInfo;
	}
	void setHopInfo(unsigned int hopInfo){
		this->hopInfo = hopInfo;
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
