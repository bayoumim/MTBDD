////////////////////////////////////////////////////////////////////////////////////////////////////
// this class define the structure of the node in raw 
////////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef _GCED_RAW_TABLE_NODE_CLASS
#define _GCED_RAW_TABLE_NODE_CLASS

#include "NodeKey.h"
#include "BDDPointer.h"

class GCedRawTableNode {
protected:
	BDDPointer id;
	NodeKey key;
	unsigned int hopInfo;
	unsigned char Flags;
public:
	// constructors
	GCedRawTableNode(){
		//BDDNode(0,0,0,0);
	}
	// set and get functions for key
	void setKey(NodeKey &key){		
		setElse(key.getElse());
		setThen(key.getThen());
		setLevel(key.getLevel());
	}
	NodeKey getKey(){
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
	void setElse(BDDPointer &Else){
		this->key.setElse(Else);
	}
	BDDPointer getElseP(){
		return this->key.getElseP();
	}
	// get and set functions for Then
	void setThen(int Then){
		this->key.setThen(Then);
	}
	int getThen(){
		return this->key.getThen();
	}
	void setThen(BDDPointer & Then){
		this->key.setThen(Then);
	}	
	BDDPointer getThenP(){
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
	
	BDDPointer getIdP() {
		return id;
	}
	void setIdP( BDDPointer & bp){
		id = bp;
	}
	// get and set functions for Hop Informations
	unsigned int getHopInfo(){
		return this->hopInfo;
	}
	void setHopInfo(unsigned int hopInfo){
		this->hopInfo = hopInfo;
	}
	void setFlags(unsigned char Flags){
		this->Flags=Flags;
	}
	void DumpToScreen(){
		cout << "Dump BDD Node" << endl;
		cout << "dump its id" << endl;
		id.DumpToScreen();
		cout << "dump its key" << endl;
		key.DumpToScreen();
		cout << "hopinfo :" << hopInfo << endl;
		cout << "flag :" << (int)Flags << endl;
		cout << endl<< endl;
	}
	bool isEmpty() {
		if ( (Flags & 1) == 0 )
			return true;
		else 				
			return false;
			
	}
};

#endif 
