////////////////////////////////////////////////////////////////////////////////////////////////////
// this class define the structure of the node in look up table
////////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef _LOOK_UP_TABLE_DEPRECIATED_NODE_CLASS
#define _LOOK_UP_TABLE_DEPRECIATED_NODE_CLASS

#include "DepreciatedBDDPointer.h"

template<typename T,typename V> // T,V: Templates for new bdd pointer
class LookUpTableDepreciatedNode {
protected:
	DepreciatedBDDPointer rawId;
	BDDPointer <T,V> newId;
	unsigned int hopInfo;
	volatile char Flags;	
public:
	// constructors
	LookUpTableDepreciatedNode (){
		hopInfo = 0;
		Flags = 0;
	}
	LookUpTableDepreciatedNode (int newId, int rawId){
		setRawId(rawId);
		setNewId(newId);
		hopInfo = 0;
		Flags = 0;
	}
	// get and set functions for ID
	unsigned int getRawId(){
		return this->rawId.getPointer();
	}
	void setRawId(unsigned int rawId){
		this->rawId.setPointer(rawId);
	}
	DepreciatedBDDPointer getRawIdP() {
		return rawId;
	}
	void setRawIdP( DepreciatedBDDPointer & bp){
		rawId = bp;
		Flags = Flags | 1;
	}
	BDDPointer <T,V> getNewId(){
		return this->newId;
	}
	void setNewId(BDDPointer <T,V> Id){
		this->newId = Id;
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
		cout << "dump its rawId" << endl;
		rawId.DumpToScreen();
		cout << "dump its key" << endl;
	}
	void Dump(ofstream & ff){
		rawId.Dump(ff);
		ff << showbase <<"rawId : " << hex << getRawId() << setbase (10)<< endl ;
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
};

#endif 
