#ifndef _BDD_POINTER_CLASS
#define _BDD_POINTER_CLASS
#include "BDDUtil.h"
// this class is used as an external pointer to a node that is already stored in hash or hashqueue tables

template<typename T,typename V>	// T: is the type of the hashString (Default: unsigned int) , V: is the type of the id (default unsigned char)
class BDDPointer {
private:
	V id;	// Format: invert|temporary|unused|id(5bit)
	// invert = (1) if the pointer point to the invertion of the node; (0) if the pointer is a regular node
	// temporary = (1) if the pointer point to a temporary node (in hashQueue); (0) it the pointer is pointing to a node in the hash table
	T hashString;	// store the whole resultant of the hashing operation (without masking)
public:
	// constructor
	BDDPointer (){
		
	}
	// get and set for local id
	V getLocalId(){
		return (id & MASK_FOR_LOCAL_ID_IN_BDD_POINTER);	// get the corresponding bits only (the defination is in BDDUtil.h file) 
	}
	void setLocalId(V newId){
		if (newId > MASK_FOR_LOCAL_ID_IN_BDD_POINTER){
			cout << "BDDPointer : setLocalId : ID overflow" << endl;
			exit(-1);
		}
		id &= MASK_FOR_UPPER_FLAGS_IN_BDD_POINTER;	// get the last two bits only (remove the old id) (the defination is in BDDUtil.h file) 
		id |= newId;					// set the new id
	}
	// set and get for hashstring
	T getHashString(){
		return hashString;
	}
	void setHashString(T newHashString){
		hashString = newHashString;
	}
	// set and get for inversion and temporary properties of the id
	bool isInverted(){
		return (id & MASK_FOR_INVERTED_BIT_IN_BDD_POINTER);	// if non-zero value; then it is true
	}
	bool isTemporary(){
		return (id & MASK_FOR_TEMPORARY_BIT_IN_BDD_POINTER);	// if non-zero value; then it is true
	}
	void setTemporary(){
		id |= MASK_FOR_TEMPORARY_BIT_IN_BDD_POINTER;
	}
	void SetInverted(){
		id |= MASK_FOR_INVERTED_BIT_IN_BDD_POINTER;
	}
        // remove inversion bit
        void ClearInverted(){
            id &= MASK_FOR_REMOVING_INVERTED_BIT_IN_BDD_POINTER;
        }
        void clearTemporary(){
            id &= MASK_FOR_REMOVING_TEMPORARY_BIT_IN_BDD_POINTER;
        }
        void invert(){
		id ^= MASK_FOR_INVERTED_BIT_IN_BDD_POINTER;
        }
	// combine the hashstring and id and inverted flag 
	// this function is used in gethashCode
	T composePointer(){
		return hashString ^ ((T)(id & MASK_FOR_REMOVING_UNUSED_BIT_IN_BDD_POINTER));
	}
	// compare two pointers (equal pointers MUST have the same bucket and the same id)
        // inversion and temporary flags are compered also
	bool isEqual(BDDPointer bp) {
		 return ( hashString == bp.getHashString() ) && ( getLocalId() == bp.getLocalId() && (isInverted() == bp.isInverted()) && (isTemporary() == bp.isTemporary()) );
	}

	void DumpToScreen(){
		cout << showbase << "ID : " << hex  << (T) id << setbase (10) << endl;
		cout << "Hash String : " << (T) hashString << endl;
		
	}
	void Dump (ofstream & ff) {
		ff << showbase << "ID : " << hex  << id << setbase (10) << endl;
		ff << "Hash String : " << hashString << endl;
	}
/*	void Dump () {
		cout << showbase << "ID : " << hex  << Pointer << setbase (10) << endl;
		cout << "Bucket : " << Bucket << endl;
	}
*/
};

#endif 

