#ifndef _DEPRECIATED_BDD_POINTER_CLASS
#define _DEPRECIATED_BDD_POINTER_CLASS
#include "BDDUtil.h"

class DepreciatedBDDPointer {
private:
	volatile unsigned int Pointer;
	volatile unsigned int Bucket;
public:
	// constructor
	DepreciatedBDDPointer (){
	}
	void invert (){
		Pointer = Pointer ^ 0x80000000;	// invert edge
	}
	// get and set functions for Bucket
	unsigned int getBucket() {
		return Bucket;
	}
	void setBucket(unsigned int Bucket) {
		this->Bucket = Bucket;
	}

	// get and set functions for pointer

	unsigned int getPointer() {
		return this->Pointer;
	}
	void setPointer(unsigned int Pointer) {
		this->Pointer = Pointer;
	}
	bool isEqual(DepreciatedBDDPointer bp) {
		// return ( this->getPointer() == bp.getPointer() ) && ( this->getBucket() == bp.getBucket() ); // WE DON'T HAVE TO CHECK FOR BUCKET AS THE BUCKET DON'T ALWAYS BE SET APPROPRIETLY
		return ( this->getPointer() == bp.getPointer() );
	}
	void Dump (ofstream & ff) {
		ff << showbase << "ID : " << hex  << Pointer << setbase (10) << endl;
		ff << "Bucket : " << Bucket << endl;
	}
	void Dump () {
		cout << showbase << "ID : " << hex  << Pointer << setbase (10) << endl;
		cout << "Bucket : " << Bucket << endl;
	}
	void DumpToScreen(){
		cout << showbase << "ID : " << hex  << Pointer << setbase (10) << endl;
		cout << "Bucket : " << Bucket << endl;
		
	}
};

#endif 
