////////////////////////////////////////////////////////////////////////////////////////////////////
// this class define the structure of the node in look up table
////////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef _LOOK_UP_TABLE_NODE_CLASS
#define _LOOK_UP_TABLE_NODE_CLASS

#include "BDDPointer.h"
template<typename T,typename V>	// T and V are types of the fields in the BDDPointer
class LookUpTableNode {
protected:
    BDDPointer <T,V> rawId;
    BDDPointer <T,V> newId;
    unsigned int hopInfo;
    volatile char Flags;
public:
    // constructors
    LookUpTableNode(){
        hopInfo = 0;
        Flags = 0;
    }
    LookUpTableNode(int newId, int rawId){
        setRawId(rawId);
        setNewId(newId);
        hopInfo = 0;
        Flags = 0;
    }
    // get and set functions for ID
    BDDPointer <T,V> getRawId() {
        return rawId;
    }
    void setRawId( BDDPointer <T,V> & bp){
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
