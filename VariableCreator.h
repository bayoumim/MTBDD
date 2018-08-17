// this class is responsible for creating one and zero, and all variables
#ifndef _VARIABLE_CREATOR_CLASS
#define _VARIABLE_CREATOR_CLASS
#include "Hash.h"
template<typename T,typename V,typename L,typename K,typename FLG>
// T,V,L,K : are templates for hash class
class VariableCreator {
private:
	BDDPointer <T,V> One;
	BDDPointer <T,V> Zero;
	BDDPointer <T,V> *Variable;	// pointers to variables, the index start from 0
	Hash <T,V,L,K,FLG> * UniqueTable;	// pointer to unique table
	L VariableCount;
	L * VariableLookUpTable;	// a look up table used to map between the level and its corresponding variables.(this used when the variable reordering is utilized)
 	BDDPointer <T,V> CreateOne () {
            BDDPointer <T,V> bp;
            bp.setHashString(TERMINAL_LEVEL);
            bp.setLocalId(ONE_ID);
            bp.clearTemporary();
            bp.ClearInverted();

            BDDNode <T,V,L> node;
            node.setThen(bp);
            node.setElse(bp);
            node.setLevel(ONE_LEVEL);

            UniqueTable->findOrAdd (node,One);	// store it in the unique table,  you don't have to ref 'one' as it's RC is set to '1' by default (and it don't deRef while forwarding as other requests)
            return One;
	}
	// create one and zero nodes and  store them in the unique table.
	BDDPointer <T,V> CreateZero () {
		Zero = One;
                Zero.invert();
		return Zero;
	}
public:
	VariableCreator (L VariableCount,Hash <T,V,L,K,FLG> * UniqueTable) {
		this->Variable = (BDDPointer <T,V> *) malloc (sizeof(BDDPointer <T,V>) * VariableCount) ;  // pointers to variables
		this->UniqueTable = UniqueTable;
		this->VariableCount = VariableCount;
		// create and store the Zero and One Nodes
		One = CreateOne ();
		Zero = CreateZero ();
		VariableLookUpTable = new L[VariableCount];
		// create Variables and store them in the unique table
		int i;
		BDDNode <T,V,L> var;

		// initiate variables
		for (i = 0; i < VariableCount ; i++) {
			VariableLookUpTable[i] = i;	// initailize look up table (this make look up table is transparent when the variable reordering is switched off)
			var.setLevel(i+1);	// set level to i (level is independent of the variable number. In other words level indicate the order in which this node will be processed in BFS)
			var.setThen(One);	// set 'then' child to one
			var.setElse (Zero);	// set 'else' child to one
			UniqueTable->findOrAdd (var,this->Variable[i]);	// store it in the unique table	, and store the pointer in the variable[i], you don't have to ref variables as it's RC is set to '1' by default (and it don't deRef while forwarding as other requests)
		}
	}

	BDDPointer <T,V> getOne () {
		return One;
	}
	BDDPointer <T,V> getZero () {
		return Zero;
	}
	BDDPointer <T,V> * getVariablePointer(){
		return Variable;
	}
	// get i-th variable,
	BDDPointer <T,V> getVariable (int i) {
		 return Variable[VariableLookUpTable[i]];
	}
	void setVarLookUpTable(L * variableLookUpTable){
		delete [] VariableLookUpTable; // delete the old lookup table
		this->VariableLookUpTable = variableLookUpTable;	// set the new lookup table
	}
	// map between level and variable number (start from 0)
	L mapVar(L i){
		return VariableLookUpTable[i];
	}
};

#endif
/*
	void UpdateVariablesAfterResizing(){
		 int i;
		 for (i = 0; i < VariableCount; i++){
		 	UniqueTable->updatePointerAfterResizing(Variable[i]);
		 }
	}
 */
