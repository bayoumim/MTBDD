
////////////////////////////////////////////////////////////////////////////////////////////////////
// this class define the structure of the node in BBD
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _BBDNODE_CLASS
#define _BBDNODE_CLASS
#include "BDDPointer.h"

template<typename T,typename V, typename L>	// T and V are types of the fields in the BDDPointer, L: is the type of the level (Default: unsigned short) ,
class BDDNode {
protected:
	L Level;    // inverted| forwarded | unused (1 bits) | level (13 bits)
	BDDPointer <T,V>Then;
	BDDPointer <T,V>Else;
public:

	// constructors
	BDDNode(){
	}
	BDDNode(T level, BDDPointer <T,V> Then, BDDPointer <T,V> Else){
		setElse(Else);
		setThen(Then);
		setLevel(level);
	}

	// set and get functions for Else
	void setElse(BDDPointer <T,V> newElse){
		Else = newElse;
	}
	BDDPointer <T,V> getElse(){
		return Else;
	}

	// set and get functions for then
	void setThen(BDDPointer <T,V> newThen){
		Then = newThen;
	}
	BDDPointer <T,V> getThen(){
		return Then;
	}

	// set and get functions for level
	void setLevel(L newLevel){
            if ( newLevel > MASK_FOR_LEVEL_IN_BDD_NODE){
                cout << "bdd node : setLevel : error" << endl;
                exit(-1);
            }
		Level = (Level & MASK_FOR_REMOVING_LEVEL_IN_BDD_NODE ) | newLevel ;
	}
	L getLevel(){
		return Level & MASK_FOR_LEVEL_IN_BDD_NODE;
	}
        // inverted NODE set and get
	bool isInverted(){
            return (Level & MASK_FOR_INVERTED_FLAG_IN_BDD_NODE);
	}
	void SetInverted(){
             Level |= MASK_FOR_INVERTED_FLAG_IN_BDD_NODE;
	}
	void clearInverted(){
		Level &=  MASK_FOR_REMOVING_INVERTED_FLAG_IN_BDD_NODE;
	}
        // FORWARDED NODE set and get
	bool isForwarded(BDDPointer <T,V> & bp){
            if((Level & MASK_FOR_FORWARDED_FLAG_IN_BDD_NODE)){
                bp = this->getElse();
                return true;
            }
            return false;
	}
        // FORWARDED NODE set and get
	bool isForwarded(){
            return (Level & MASK_FOR_FORWARDED_FLAG_IN_BDD_NODE);
	}
	void SetForwarded(){
             Level |= MASK_FOR_FORWARDED_FLAG_IN_BDD_NODE;
	}
	void clearForwarded(){
		Level &=  MASK_FOR_REMOVING_FORWARDED_FLAG_IN_BDD_NODE;
	}
	void forwardTo(BDDPointer <T,V> bp){
            this->SetForwarded();
            this->setElse(bp);
	}
        
	// compare two nodes
	bool isEqual(BDDNode  <T,V,L> & right){
		if ( (right.getLevel() == getLevel()) && (right.getThen().isEqual(getThen())) && (right.getElse().isEqual(getElse())) ){
			return true;
		}		
		return false;
	}

	void DumpToScreen(){
		cout << "Dump BDD Node" << endl;
		cout << "dump its Level : " << (L)Level  << endl;
                cout << "Then: " << endl;
                Then.DumpToScreen();
                cout << "Else : " << endl;
                Else.DumpToScreen();
	}
	void Dump(ofstream & ff){
            ff << "level" << getLevel() << endl;
            ff << "then: " << endl;
            getThen().Dump(ff);
            ff << "else: " << endl;
            getElse().Dump(ff);
	}
 
};

#endif 
