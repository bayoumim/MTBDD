#ifndef _DEPRECIATED_NODE_KEY_CLASS
#define _DEPRECIATED_NODE_KEY_CLASS

#include "BDDUtil.h"
#include "DepreciatedBDDPointer.h"

class DepreciatedNodeKey{
private:
	DepreciatedBDDPointer Then;
	DepreciatedBDDPointer Else;
	volatile int level;
public:
	// conststrcutors
	DepreciatedNodeKey(){
		//DepreciatedNodeKey(0,0,0);
	}
	DepreciatedNodeKey (int Then, int Else, int Level) {
		setThen (Then);
		setElse(Else);
		setLevel(Level);
	}
	DepreciatedNodeKey (DepreciatedBDDPointer &Then, DepreciatedBDDPointer & Else, int Level) {
		this->Then = Then;
		this->Else = Else;
		this->level = Level;
	}	
	// get and set functions for Else
	void setElse(int Else){
		this->Else.setPointer(Else);
	}
	int getElse(){
		return this->Else.getPointer();
	}
		// overloaded versions
	void setElse(DepreciatedBDDPointer &Else){
		this->Else = Else;
	}
	DepreciatedBDDPointer getElseP(){
		return this->Else;
	}	
	// get and set functions for Then
	void setThen(int Then){
		this->Then.setPointer(Then);
	}
	int getThen(){
		return this->Then.getPointer();
	}
		// overloaded
	void setThen(DepreciatedBDDPointer &Then){
		this->Then = Then;
	}
	DepreciatedBDDPointer getThenP(){
		return this->Then;
	}
	// get and set functions for Level
	void setLevel(int level){
		this->level = level;
	}
	int getLevel(){
		return level;
	}//Fred operator+ (Fred const& x, Fred const& y); int operator==(Mint & a)
	bool isEqual(DepreciatedNodeKey right){
		if ( (this->getLevel() == right.getLevel()) & (this->getThen() == right.getThen()) & (this->getElse() == right.getElse()) ){
			return true;
		}		
		return false;
	}
	// set and get for pointers
	
	void Dump(ofstream & ff){
		ff << "Then : " << endl;
		Then.Dump(ff);
		ff << "Else : " << endl;
		Else.Dump(ff);
		ff << "Level : " << level << "\n" << endl;
	}
	void DumpToScreen(){		
		cout << "Then : " << endl;
		Then.Dump();
		cout << "Else : " << endl;
		Else.Dump();
		cout << "Level : " << level << "\n" << endl;
	}
	void setThenBucket(unsigned int buck){
		Then.setBucket(buck);
	}
	void setElseBucket(unsigned int buck){
		Else.setBucket(buck);
	}
};

#endif
