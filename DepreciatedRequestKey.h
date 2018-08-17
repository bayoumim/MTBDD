#ifndef _DEPRECIATED_KEY_REQUEST_CLASS
#define _DEPRECIATED_KEY_REQUEST_CLASS

#include "DepreciatedBDDPointer.h"

class DepreciatedRequestKey {
private:
	DepreciatedBDDPointer Fptr;
	DepreciatedBDDPointer Gptr;
	DepreciatedBDDPointer Hptr;
public:
	// CONSTRUCTORS
	DepreciatedRequestKey (){

	}
	DepreciatedRequestKey (DepreciatedBDDPointer & F,DepreciatedBDDPointer &G ,DepreciatedBDDPointer &H ) {
		this->setF(F);
		this->setG(G);
		this->setH(H);
	}
	// function simulates else, then and level
	int getThen(){
		return Fptr.getPointer();
	}
	int getElse(){
		return Gptr.getPointer();
	}
	int getLevel(){
		return Hptr.getPointer();
	}
	// set and get for F
	DepreciatedBDDPointer getF(){
		return Fptr;
	}
	void setF(DepreciatedBDDPointer & F){
		Fptr = F;
	}
	// set and get for G
	DepreciatedBDDPointer getG(){
		return Gptr;
	}
	void setG(DepreciatedBDDPointer &G){
		Gptr = G;
	}
	// set and get for h
	DepreciatedBDDPointer getH(){
		return Hptr;
	}
	void setH(DepreciatedBDDPointer & H){
		Hptr = H;
	}
	bool isEqual (DepreciatedRequestKey & key) {
		DepreciatedBDDPointer f = key.getF();
		DepreciatedBDDPointer g = key.getG();
		DepreciatedBDDPointer h = key.getH();
		return ( this->getF().isEqual(f) ) && ( this->getG().isEqual(g) ) && ( this->getH().isEqual(h) );
	}
	void DumpToScreen(){
		cout << "Dump Request Key" << endl;
		cout << "F Pointer: " << endl;
		Fptr.DumpToScreen();
		cout << "G Pointer: " << endl;
		Gptr.DumpToScreen();
		cout << "H Pointer: " << endl;
		Hptr.DumpToScreen();
	}
	void Dump(ofstream & ff){
		ff << "Dump Request Key" << endl;
		ff << "F Pointer: " << endl;
		Fptr.Dump(ff);
		ff << "G Pointer: " << endl;
		Gptr.Dump(ff);
		ff << "H Pointer: " << endl;
		Hptr.Dump(ff);
	}
};

#endif 
