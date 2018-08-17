#ifndef _KEY_REQUEST_CLASS
#define _KEY_REQUEST_CLASS

#include "BDDPointer.h"

template<typename T,typename V>	
class RequestKey {
private:
	BDDPointer <T,V> Fptr;
	BDDPointer <T,V> Gptr;
	BDDPointer <T,V> Hptr;
public:
	// CONSTRUCTORS
	RequestKey (){}
	RequestKey (BDDPointer <T,V> & F,BDDPointer <T,V> &G ,BDDPointer <T,V> &H ) {
		this->setF(F);
		this->setG(G);
		this->setH(H);
	}
	// set and get for F
	BDDPointer <T,V> getF(){
		return Fptr;
	}
	void setF(BDDPointer <T,V> F){
		Fptr = F;
	}
	// set and get for G
	BDDPointer <T,V> getG(){
		return Gptr;
	}
	void setG(BDDPointer <T,V> G){
		Gptr = G;
	}
	// set and get for h
	BDDPointer <T,V> getH(){
		return Hptr;
	}
	void setH(BDDPointer <T,V> H){
		Hptr = H;
	}
	bool isEqual (RequestKey & key) {
		BDDPointer <T,V> f = key.getF();
		BDDPointer <T,V> g = key.getG();
		BDDPointer <T,V> h = key.getH();
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
