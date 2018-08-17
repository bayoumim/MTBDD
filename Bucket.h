#ifndef _BUCKET_CLASS
#define _BUCKET_CLASS
// this class is used as the basic building block for Unique Table (hash.h class). it represent a single bucket in the table
#include "BDDNode.h"

template<typename T,typename V,typename L>	
// T: is the type of fields of the nodes (Default: unsigned int). it could be "unsigned int" so the size of the bucket will be 16 byte OR it could be "unsigned" long so the size of the bucket will be 32 byte.
// V: is the type of the local id
class Bucket{
private:
    T Then;
    T Else;
    T hopInfo;
    T GroupedData;	// contain flags and RC,level .. etc

    // WHEN YOU REMOVE RESERVED AND SWAPPED YOU SHOULD REPLACE IT WITH RC
    // default (when you change to 64 bit representation you have to make another values for this format)

    // FORMAT: unused (2 bits) | level(13 bit) | full | Else Inverted Edge | Else local id (5 bit) | Then local id (5 bit) | node local id (5 bit)
    // level : the node support up to 8 k variables (2^13)
    // full; (0 - means empty, 1 means full)
    // Else Inverted Edge : 0 means the else flag is regular, 1 means the else flag is inverted
    // Else local id : local id for Else
    // Then local id : local id for then
    // node local id : local id for this node



    // RC require more space when RC GC is running, while in mark and incremental mark requires less space.
public:
    Bucket() {
        GroupedData = 0;
        hopInfo = 0;
    }
    // get and set BDD Nodes
    void setNode(BDDNode <T,V,L> node){
        setThen(node.getThen());
        setElse(node.getElse());
        setLevel(node.getLevel());
        setFull();
    }
    BDDNode <T,V,L> getNode () {
        BDDNode <T,V,L> node;	// create a node
        node.setThen(getThen());
        node.setElse(getElse());
        node.setLevel(getLevel());
        node.clearForwarded();
        node.clearInverted();
        return node;
    }
    // get and set functions for Then
    BDDPointer <T,V> getThen(){
        BDDPointer <T,V> bp;
        bp.setHashString(Then);	// set hash string
        bp.setLocalId(((GroupedData & MASK_FOR_THEN_ID_IN_BDD_BUCKET)>>SHIFT_FOR_THEN_ID_IN_BDD_BUCKET));	// mask and shift to get local "then" id
        bp.clearTemporary();
        bp.ClearInverted();
        return bp;
    }
    void setThen(BDDPointer <T,V> newThen){
        Then = newThen.getHashString();	// set new hash string
        GroupedData = (GroupedData & MASK_FOR_REMOVE_THEN_ID_IN_BDD_BUCKET) | (newThen.getLocalId() << SHIFT_FOR_THEN_ID_IN_BDD_BUCKET);	// remove the old local id and add the new one.
    }

    // get and set functions for Else
    BDDPointer <T,V> getElse(){
        BDDPointer <T,V> bp;
        bp.setHashString(Else);	// set hash string
        T temp = GroupedData & MASK_FOR_ELSE_ID_IN_BDD_BUCKET;
        temp >>= SHIFT_FOR_ELSE_ID_IN_BDD_BUCKET;
        bp.setLocalId( (V) temp);	// mask and shift to get local "Else" id

        if(this->isElseInverted())
            bp.SetInverted();
        else
            bp.ClearInverted();
        bp.clearTemporary();
        return bp;
    }
    void setElse(BDDPointer <T,V> newElse){
        Else = newElse.getHashString();	// set new hash string
        T temp = newElse.getLocalId();
        temp <<= SHIFT_FOR_ELSE_ID_IN_BDD_BUCKET;
        T temp1 = GroupedData & MASK_FOR_REMOVE_ELSE_ID_IN_BDD_BUCKET;
        GroupedData = temp | temp1;	// remove the old local id and add the new one with the inverted edge
        if(newElse.isInverted())
            this->setElseInverted();
        else
            this->clearElseInverted();
    }

    // get and set functions for Hop Informations
    T getHopInfo(){
        return this->hopInfo;
    }
    void setHopInfo(T newHopInfo){
        this->hopInfo = newHopInfo;
    }
    // get and set function for local id
    V getLocalId() {
        return GroupedData & MASK_FOR_LOCAL_ID_IN_BDD_BUCKET;	// Get the local id for the node
    }
    void setLocalId(V id){
        if (id > MASK_FOR_LOCAL_ID_IN_BDD_BUCKET){
            cout << "Bucket : setLocalId : ID overflow" << endl;
            exit(-1);
        }

        GroupedData = (GroupedData & MASK_FOR_REMOVE_LOCAL_ID_IN_BDD_BUCKET) | id;	// remove the old local id and add the new one.
    }
    // get empty flag
    bool isEmpty(){
        return !(GroupedData & MASK_FOR_FULL_FLAG_IN_BDD_BUCKET);
    }
    void setFull(){
        GroupedData |= MASK_FOR_FULL_FLAG_IN_BDD_BUCKET;
    }
    void clearFull(){
        GroupedData &= MASK_FOR_REMOVING_FULL_FLAG_IN_BDD_BUCKET;
    }
    // get and set function for level
    L  getLevel(){
        T temp =GroupedData & MASK_FOR_GETTING_LEVEL_IN_BDD_BUCKET;
        temp >>= SHIFT_FOR_LEVEL_IN_BDD_BUCKET;
        return (L) temp;	// get the level only, then shift it.
    }
    void setLevel(L level){
        if (level > MAX_NO_OF_LEVELS_IN_BDD_BUCKET){
            cout << "Bucket : setLevel : ID overflow" << endl;
            exit(-1);
        }
        T temp = GroupedData & MASK_FOR_REMOVING_LEVEL_IN_BDD_BUCKET;
        T temp1 = level;
        temp1 <<= SHIFT_FOR_LEVEL_IN_BDD_BUCKET;
        temp = temp | temp1;
        GroupedData =  temp;	// remove old level, then add the new level.
    }
    // set and get for else inverted edge
    bool isElseInverted(){
        return (GroupedData & MASK_FOR_ELSE_INVERTED_EDGE_IN_BDD_BUCKET);
    }
    void setElseInverted(){
        GroupedData |= MASK_FOR_ELSE_INVERTED_EDGE_IN_BDD_BUCKET;
    }
    void clearElseInverted(){
        GroupedData = GroupedData & MASK_FOR_REMOVING_ELSE_INVERTED_EDGE_IN_BDD_BUCKET;
    }

    void clearAllFlags() {
        GroupedData &= MASK_FOR_REMOVING_ALL_FLAGS_IN_BDD_BUCKET;
    }
    // get and set for all flags
    void setFlags(T Flags){
        GroupedData &= MASK_FOR_REMOVING_ALL_FLAGS_IN_BDD_BUCKET;   // remove old flags
        GroupedData |= Flags;                                       // add new flags
    }
    T getFlags(){
        return (GroupedData & MASK_FOR_ALL_FLAGS_IN_BDD_BUCKET) ;	// return full, swap and reserved flags in their original places
    }
    // BUILT THESE FUNCTION WHEN YOU INCLUDE GC AGAIN (AFTER REMOVING RESERVED AND SWAPPED FLAGS)
    void setReferenceCounter(int ReferenceCounter){

    }
    int getReferenceCounter () {
            return 0;
    }
    void incrementRC(){
            ;
    }
    void decrementRC(){
            ;
    }
    // output data in various formats
    void DumpToScreen(){
        cout << showbase << "hop Information : " << hex << (T) this->hopInfo << "\n";
        cout << showbase << "Then : " << hex << (T) this->Then << "\n";
        cout << showbase << "Else : " << hex << (T) this->Else << "\n";
        cout << showbase << "group data : " << hex << (T) this->GroupedData << "\n";
        cout << setbase (10) ;
    }
    void Dump(ofstream & ff){
        ff << showbase << "hop Information : " << hex << (T) this->hopInfo << "\n";
        ff << showbase << "Then : " << hex << (T) this->Then << "\n";
        ff << showbase << "Else : " << hex << (T) this->Else << "\n";
        ff << showbase << "group data : " << hex << (T) this->GroupedData << "\n";
        ff << setbase (10) ;
    }
    // compare two nodes
    bool isEqual(BDDNode <T,V,L> right){
        if ( (right.getLevel() == getLevel()) && (right.getThen().isEqual(getThen())) && (right.getElse().isEqual(getElse())) ){
            return true;
        }
        return false;
    }
    void DumpToFile(ofstream & ff){
        ff << hopInfo << endl;
        ff << Then << endl;
        ff << Else << endl;;
        ff << GroupedData << endl;
    }
    // function for integrity check purposes
    void setI(T hopInfo){
        this->hopInfo = hopInfo;
    }
    void setT(T then){
        this->Then = then;
    }
    void setE(T Else){
        this->Else = Else;
    }
    void setGD(T GD){
        this->GroupedData = GD;
    }

};

#endif 

		
		
		

/*	// get and set reference counter
	void setReferenceCounter(int ReferenceCounter){
		if(ReferenceCounter > 255){
			cout << "Bucket : setReferenceCounter: error : data out of range" << endl;
			exit(-1);
		}
		this->ReferenceCounter = (unsigned char) ReferenceCounter;
	}
	int getReferenceCounter () {
		return (int)this->ReferenceCounter;
	}
	void incrementRC(){
		ReferenceCounter++;
	}
	void decrementRC(){
		ReferenceCounter--;
	}
*/

