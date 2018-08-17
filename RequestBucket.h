#ifndef _REQUEST_BUCKET_CLASS
#define _REQUEST_BUCKET_CLASS
// this class is used as the basic building block for Queue (hashQueue.h class). it represent a single bucket in the Queue
#include "BDDPointer.h"
#include "BDDNode.h"
#include "Request.h"

template<typename T,typename V,typename L,typename FLG>
// T: is the type of fields of the nodes (Default: unsigned int). it could be "unsigned int" so the size of the bucket will be 16 byte OR it could be "unsigned" long so the size of the bucket will be 32 byte.
// V: is the type of the local id

class RequestBucket{
private:
    T F;                // hash string for F
    T G;                // hash string for G
    T H;                // hash string for H
    T HopInfo;          // hop info
    T Then;             // haR Elsesh string for 'then'
    T Else;             // hash string for 'else'
    T GroupData1;       //  5 ids
    // FORMAT : unused (2 bits) | Else id (5 bits)  | Then id (5 bits) | H (5 bits) | G id (5 bits) | F id (5 bits) | Node id (5 bit)
    // node inverted : 1 the R is inverted, 0 the R is not.
    // else inverted : 1 the else pointer for R is inverted, 0 it is regular

    T GroupData2;   // flags are stored in 15 bits (LSBs)
    // FORMAT : level (13 bits) | unused (4 bits) | forwarded flag | else pointer is inverted| then pointer is inverted | H inv | G inv | F inv | node inverted | Else temporary Flag |Then temporary Flag |Else Pointer processed in Reduce|Then Pointer processed in Reduce |Else Pointer processed in Apply|Then Pointer processed in Apply| old Version (dated) | Full
    // else temporary flag: 'else' pointer of R is temporary
    // then temporary flag : 'then' pointer of R is temporary
    //(for old version = 0-new version,1 - old version);
    //(for empty = 0-empty, 1-occupied);
    //(reserved; 0 mean not reserved, 1 means reserved,
    //(swaped; 0 means not swaped, 1 means swaped, this happens when the thread exist the while loop in add_or_find method in hashqueue, and there are two different segments need to be added).
    //(processed, 0-not processed, 1-processed. processed node is the node whose then and else are pointing to some other nodes in hashtable or in the queue)
public:	
    RequestBucket()  {
        HopInfo = 0;
        GroupData1 = 0;
        GroupData2 = 0;
    }
    // get and set function for key
    RequestKey <T,V>  getKey(){
        RequestKey <T,V> key;
        key.setH(getH());
        key.setG(getG());
        key.setF(getF());
        return key;
    }
    void setKey(RequestKey <T,V>  key){
        setF(key.getF());
        setG(key.getG());
        setH(key.getH());
    }
    // get and set BDD Nodes
    void setNode(BDDNode <T,V,L> node){
        setRThen(node.getThen());
        setRElse(node.getElse());
        setLevel(node.getLevel());
        if(node.isInverted())
            this->SetRNodeInverted();
        else
            this->clearRNodeInverted();

        if(node.isForwarded())
            this->SetForwarded();
        else
            this->clearForwarded();

        setFull();
    }
    BDDNode <T,V,L> getNode (){
        BDDNode <T,V,L> node;
        node.setElse(getRElse());
        node.setThen(getRThen());
        node.setLevel(getLevel());

        if(this->isRNodeInverted())
            node.SetInverted();
        else
            node.clearInverted();

        if(this->isForwarded())
            node.SetForwarded();
        else
            node.clearForwarded();

        return node;
    }
    // get and set functions for request
    Request  <T,V,L> getRequest(){
        Request  <T,V,L> req;
        req.setR(getNode());
        req.setKey(getKey());
        return req;
    }
    void setRequest(Request  <T,V,L> req){
        setKey(req.getKey());
        setNode(req.getR());
    }

    // set and get for F
    BDDPointer <T,V>  getF(){
        BDDPointer <T,V> bp;

        bp.setHashString(F);    // store F hash string
        if(isFInverted()){
            bp.SetInverted();
        }
        else
            bp.ClearInverted();

        bp.setLocalId(this->getFLocalId());
        bp.clearTemporary();
        return bp;
    }
    void setF(BDDPointer <T,V>  F){
        if(F.isInverted()) // if
            SetFInvertedEdge();
        else
            clearFInvertedEdge();
        this->setFLocalId(F.getLocalId());
        this->F = F.getHashString();

    }
    // set and get for G
    BDDPointer <T,V>  getG(){
        BDDPointer <T,V> bp;

        bp.setHashString(G);    // store G hash string
        if(isGInverted()){
            bp.SetInverted();
        }
        else
            bp.ClearInverted();
        bp.clearTemporary();
        bp.setLocalId(this->getGLocalId());

        return bp;
    }
    void setG(BDDPointer <T,V>  G){
        if(G.isInverted()) // if
            SetGInvertedEdge();
        else
            clearGInvertedEdge();
        this->setGLocalId(G.getLocalId());
        this->G = G.getHashString();
    }
    // set and get for H
    BDDPointer <T,V>  getH(){
        BDDPointer <T,V> bp;

        bp.setHashString(H);    // store H hash string
        if(isHInverted()){
            bp.SetInverted();
        }
        else
            bp.ClearInverted();
        bp.clearTemporary();
        bp.setLocalId(this->getHLocalId());
        return bp;
    }
    void setH(BDDPointer <T,V> H){
        if(H.isInverted()) // if
            SetHInvertedEdge();
        else
            clearHInvertedEdge();
        this->setHLocalId(H.getLocalId());
        this->H = H.getHashString();
    }
    // get and set functions for Hop Informations
    unsigned int getHopInfo(){
            return this->HopInfo;
    }
    void setHopInfo(unsigned int hopInfo){
            this->HopInfo = hopInfo;
    }
    // set and get for R THEN
    BDDPointer <T,V>  getRThen(){
        BDDPointer <T,V> bp;
        bp.setHashString(Then);    // store F hash string
        // set inversion
        if(this->isRThenInverted()){
            bp.SetInverted();
        }
        else
            bp.ClearInverted();
        // set temporary
        if(isRThenTemporary()){
            bp.setTemporary();
        }
        else
            bp.clearTemporary();
        // set id
        bp.setLocalId(getRThenLocalId());
        return bp;
    }
    void setRThen(BDDPointer <T,V>  F){
        // set inversion
        if(F.isInverted()) // if
            SetRThenInvertedEdge();
        else
            clearRThenInvertedEdge();
        // set temporary
        if(F.isTemporary()) // if
            SetRThenTemporary();
        else
            clearRThenTemporary();
        // set hash string
        this-> Then= F.getHashString();
        // store id
        setRThenLocalId(F.getLocalId());
    }
    // set and get for R Else
    BDDPointer <T,V>  getRElse(){
        BDDPointer <T,V> bp;
        bp.setHashString(Else);    // store F hash string
        // set inversion
        if(this->isRElseInverted()){
            bp.SetInverted();
        }
        else
            bp.ClearInverted();
        // set temporary
        if(isRElseTemporary()){
            bp.setTemporary();
        }
        else
            bp.clearTemporary();
        // set id
        bp.setLocalId(getRElseLocalId());
        return bp;
    }
    void setRElse(BDDPointer <T,V> F){
        // set inversion
        if(F.isInverted()) // if
            SetRElseInvertedEdge();
        else
            clearRElseInvertedEdge();
        // set temporary
        if(F.isTemporary()) // if
            SetRElseTemporary();
        else
            clearRElseTemporary();
        // set hash string
        this->Else= F.getHashString();
        // store id
        setRElseLocalId(F.getLocalId());
    }

   // level get and set
    L getLevel(){
        return (GroupData2 >> SHIFT_FOR_LEVEL_REQUEST_BUCKET);
    }
    void setLevel(L level){
        GroupData2 =  (GroupData2 & MASK_FOR_REMOVING_LEVEL_IN_REQUEST_BUCKET) | (level << SHIFT_FOR_LEVEL_REQUEST_BUCKET);
    }
    //get and set for R node id
    V getRLocalId(){
        return (GroupData1 & MASK_FOR_ID_IN_REQUEST_BUCKET);
    }
    void setRLocalId(V id){
        if (id > MASK_FOR_LOCAL_ID_IN_BDD_POINTER){
            cout << "RequestBucket : setRLocalId : ID overflow" << endl;
            exit(-1);
        }
        GroupData1 = (GroupData1 & MASK_FOR_REMOVING_R_NODE_ID_IN_REQUEST_BUCKET) | id;
    }
    //get and set for F id
    V getFLocalId(){
        return ((GroupData1 >> SHIFT_FOR_F_IN_REQUEST_BUCKET) & MASK_FOR_ID_IN_REQUEST_BUCKET);
    }
    void setFLocalId(V id){
        if (id > MASK_FOR_LOCAL_ID_IN_BDD_POINTER){
            cout << "RequestBucket : setFLocalId : ID overflow" << endl;
            exit(-1);
        }
        GroupData1 = (GroupData1 & MASK_FOR_REMOVING_F_ID_IN_REQUEST_BUCKET) | (id << SHIFT_FOR_F_IN_REQUEST_BUCKET);
    }
    //get and set for G id
    V getGLocalId(){
        return ((GroupData1 >> SHIFT_FOR_G_IN_REQUEST_BUCKET) & MASK_FOR_ID_IN_REQUEST_BUCKET);
    }
    void setGLocalId(V id){
        if (id > MASK_FOR_LOCAL_ID_IN_BDD_POINTER){
            cout << "RequestBucket : setGLocalId : ID overflow" << endl;
            exit(-1);
        }
        GroupData1 = (GroupData1 & MASK_FOR_REMOVING_G_ID_IN_REQUEST_BUCKET) | (id << SHIFT_FOR_G_IN_REQUEST_BUCKET);
    }
    //get and set for H id
    V getHLocalId(){
        return ((GroupData1 >> SHIFT_FOR_H_IN_REQUEST_BUCKET) & MASK_FOR_ID_IN_REQUEST_BUCKET);
    }
    void setHLocalId(V id){
        if (id > MASK_FOR_LOCAL_ID_IN_BDD_POINTER){
            cout << "RequestBucket : setHLocalId : ID overflow" << endl;
            exit(-1);
        }
        GroupData1 = (GroupData1 & MASK_FOR_REMOVING_H_ID_IN_REQUEST_BUCKET) | (id << SHIFT_FOR_H_IN_REQUEST_BUCKET);
    }
    //get and set for R THEN id
    V getRThenLocalId(){
        return ((GroupData1 >> SHIFT_FOR_R_THEN_IN_REQUEST_BUCKET) & MASK_FOR_ID_IN_REQUEST_BUCKET);
    }
    void setRThenLocalId(V id){
        if (id > MASK_FOR_LOCAL_ID_IN_BDD_POINTER){
            cout << "RequestBucket : setRThenLocalId : ID overflow" << endl;
            exit(-1);
        }
        GroupData1 = (GroupData1 & MASK_FOR_REMOVING_R_THEN_ID_IN_REQUEST_BUCKET) | (id << SHIFT_FOR_R_THEN_IN_REQUEST_BUCKET );
    }
    //get and set for R ELSE id
    V getRElseLocalId(){
        return ((GroupData1 >> SHIFT_FOR_R_ELSE_IN_REQUEST_BUCKET) & MASK_FOR_ID_IN_REQUEST_BUCKET);
    }
    void setRElseLocalId(V id){
        if (id > MASK_FOR_LOCAL_ID_IN_BDD_POINTER){
            cout << "RequestBucket : setRThenLocalId : ID overflow" << endl;
            exit(-1);
        }
        GroupData1 = (GroupData1 & MASK_FOR_REMOVING_R_ELSE_ID_IN_REQUEST_BUCKET) | (id << SHIFT_FOR_R_ELSE_IN_REQUEST_BUCKET);
    }
    // full set and get
    bool isEmpty(){
        return !(GroupData2 & MASK_FOR_FULL_FLAG_IN_REQUEST_BUCKET);
    }
    void setFull(){
        GroupData2 |= MASK_FOR_FULL_FLAG_IN_REQUEST_BUCKET;
    }
    void clearFull(){
        GroupData2 &= MASK_FOR_REMOVING_FULL_FLAG_IN_REQUEST_BUCKET;
    }
    // dated set and get
    bool isDated(){
        return (GroupData2 & MASK_FOR_DATED_FLAG_IN_REQUEST_BUCKET);
    }
    void SetDated(){
        GroupData2 |= MASK_FOR_DATED_FLAG_IN_REQUEST_BUCKET;
    }
    void clearDated(){
        GroupData2 &= MASK_FOR_REMOVING_DATED_FLAG_IN_REQUEST_BUCKET;
    }
    // THEN PROCESSED IN APPLY set and get
    bool isThenProcessedInApply(){
        return (GroupData2 & MASK_FOR_THEN_PROCESSED_IN_APPLY_FLAG_IN_REQUEST_BUCKET);
    }
    void SetThenProcessedInApply(){
        GroupData2 |= MASK_FOR_THEN_PROCESSED_IN_APPLY_FLAG_IN_REQUEST_BUCKET;
    }
    void clearThenProcessedInApply(){
        GroupData2 &= MASK_FOR_REMOVING_THEN_PROCESSED_IN_APPLY_FLAG_IN_REQUEST_BUCKET;
    }
    // ELSE PROCESSED IN APPLY set and get
    bool isElseProcessedInApply(){
        return (GroupData2 & MASK_FOR_ELSE_PROCESSED_IN_APPLY_FLAG_IN_REQUEST_BUCKET);
    }
    void SetElseProcessedInApply(){
        GroupData2 |= MASK_FOR_ELSE_PROCESSED_IN_APPLY_FLAG_IN_REQUEST_BUCKET;
    }
    void clearElseProcessedInApply(){
        GroupData2 &= MASK_FOR_REMOVING_ELSE_PROCESSED_IN_APPLY_FLAG_IN_REQUEST_BUCKET;
    }
    // THEN PROCESSED IN REDUCE set and get
    bool isThenProcessedInReduce(){
        return (GroupData2 & MASK_FOR_THEN_PROCESSED_IN_APPLY_FLAG_IN_REQUEST_BUCKET);
    }
    void SetThenProcessedInReduce(){
        GroupData2 |= MASK_FOR_THEN_PROCESSED_IN_REDUCE_FLAG_IN_REQUEST_BUCKET;
    }
    void clearThenProcessedInReduce(){
        GroupData2 &= MASK_FOR_REMOVING_THEN_PROCESSED_IN_REDUCE_FLAG_IN_REQUEST_BUCKET;
    }
    // ELSE PROCESSED IN REDUCE set and get
    bool isElseProcessedInReduce(){
        return (GroupData2 & MASK_FOR_ELSE_PROCESSED_IN_REDUCE_FLAG_IN_REQUEST_BUCKET);
    }
    void SetElseProcessedInReduce(){
        GroupData2 |= MASK_FOR_ELSE_PROCESSED_IN_REDUCE_FLAG_IN_REQUEST_BUCKET;
    }
    void clearElseProcessedInReduce(){
        GroupData2 &= MASK_FOR_REMOVING_ELSE_PROCESSED_IN_REDUCE_FLAG_IN_REQUEST_BUCKET;
    }
    // THEN PROCESSED IN REDUCE set and get
    bool isRThenTemporary(){
        return (GroupData2 & MASK_FOR_THEN_TEMPORARY_FLAG_IN_REQUEST_BUCKET);
    }
    void SetRThenTemporary(){
        GroupData2 |= MASK_FOR_THEN_TEMPORARY_FLAG_IN_REQUEST_BUCKET;
    }
    void clearRThenTemporary(){
        GroupData2 &= MASK_FOR_REMOVING_THEN_TEMPORARY_FLAG_IN_REQUEST_BUCKET;
    }
    // ELSE PROCESSED IN REDUCE set and get
    bool isRElseTemporary(){
        return (GroupData2 & MASK_FOR_ELSE_TEMPORARY_FLAG_IN_REQUEST_BUCKET);
    }
    void SetRElseTemporary(){
        GroupData2 |= MASK_FOR_ELSE_TEMPORARY_FLAG_IN_REQUEST_BUCKET;
    }
    void clearRElseTemporary(){
        GroupData2 &= MASK_FOR_REMOVING_ELSE_TEMPORARY_FLAG_IN_REQUEST_BUCKET;
    }
    // NODE INVERTED EDGE set and get
    bool isRNodeInverted(){
        return (GroupData2 &  MASK_FOR_R_NODE_INVERSION_IN_REQUEST_BUCKET);
    }
    void SetRNodeInverted(){
        GroupData2 |= MASK_FOR_R_NODE_INVERSION_IN_REQUEST_BUCKET;
    }
    void clearRNodeInverted(){
        GroupData2 &= MASK_FOR_REMOVING_R_NODE_INVERSION_FLAG_IN_REQUEST_BUCKET;
    }
    // F inverted EDGE set and get
    bool isFInverted(){
        return (GroupData2 & MASK_FOR_F_INVERTED_EDGE_IN_REQUEST_BUCKET);
    }
    void SetFInvertedEdge(){
        GroupData2 |= MASK_FOR_F_INVERTED_EDGE_IN_REQUEST_BUCKET;
    }
    void clearFInvertedEdge(){
        GroupData2 &=  MASK_FOR_REMOVING_F_INVERTED_EDGE_FLAG_IN_REQUEST_BUCKET;
    }
    // g inverted EDGE set and get
    bool isGInverted(){
        return (GroupData2 & MASK_FOR_G_INVERTED_EDGE_IN_REQUEST_BUCKET);
    }
    void SetGInvertedEdge(){
        GroupData2 |= MASK_FOR_G_INVERTED_EDGE_IN_REQUEST_BUCKET;
    }
    void clearGInvertedEdge(){
        GroupData2 &=  MASK_FOR_REMOVING_G_INVERTED_EDGE_FLAG_IN_REQUEST_BUCKET;
    }
    // H inverted EDGE set and get
    bool isHInverted(){
        return (GroupData2 & MASK_FOR_H_INVERTED_EDGE_IN_REQUEST_BUCKET);
    }
    void SetHInvertedEdge(){
        GroupData2 |= MASK_FOR_H_INVERTED_EDGE_IN_REQUEST_BUCKET;
    }
    void clearHInvertedEdge(){
        GroupData2 &=  MASK_FOR_REMOVING_H_INVERTED_EDGE_FLAG_IN_REQUEST_BUCKET;
    }
    // THEN EDGE INVERTED EDGE set and get
    bool isRThenInverted(){
        return (GroupData2 & MASK_THEN_POINTER_INVERTED_EDGE_IN_REQUEST_BUCKET);
    }
    void SetRThenInvertedEdge(){
        GroupData2 |= MASK_THEN_POINTER_INVERTED_EDGE_IN_REQUEST_BUCKET;
    }
    void clearRThenInvertedEdge(){
        GroupData2 &=  MASK_FOR_REMOVING_THEN_POINTER_INVERTED_EDGE_IN_REQUEST_BUCKET;
    }
    // ELSE EDGE INVERTED EDGE set and get
    bool isRElseInverted(){
        return (GroupData2 & MASK_ELSE_POINTER_INVERTED_EDGE_IN_REQUEST_BUCKET);
    }
    void SetRElseInvertedEdge(){
        GroupData2 |= MASK_ELSE_POINTER_INVERTED_EDGE_IN_REQUEST_BUCKET;
    }
    void clearRElseInvertedEdge(){
        GroupData2 &=  MASK_FOR_REMOVING_ELSE_POINTER_INVERTED_EDGE_IN_REQUEST_BUCKET;
    }
    // FORWARDED NODE set and get
    bool isForwarded(){
        return (GroupData2 & MASK_FORWARDED_NODE_IN_REQUEST_BUCKET);
    }
    // overloaded version
    bool isForwarded(BDDPointer <T,V> & bp){
        if((GroupData2 & MASK_FORWARDED_NODE_IN_REQUEST_BUCKET)){
            bp = getRElse();
            return true;
        }
        return false;
    }
    void SetForwarded(BDDPointer <T,V> bp){
        setRElse(bp);
        clearRElseTemporary();
        GroupData2 |= MASK_FORWARDED_NODE_IN_REQUEST_BUCKET;
    }
    void SetForwarded(){
        GroupData2 |= MASK_FORWARDED_NODE_IN_REQUEST_BUCKET;
    }

    void clearForwarded(){
        GroupData2 &=  MASK_FOR_REMOVIN_FORWARDED_NODE_IN_REQUEST_BUCKET;
    }
    // Master get and set for all flags
    void clearAllFlags() {
        GroupData2 = 0;
        GroupData1 = 0;
    }
    // as the package doesn't get or set flags for buckets that are not neither reserved nor swapped; I neglect to copy them
    void setFlags(FLG Flags){
        if(Flags > MASK_FOR_ALL_FLAGS_IN_REQUEST_BUCKET ){
            cout << "RequestBucket : setFlags" <<endl;
            exit(-1);
        }
        GroupData2 &= MASK_FOR_REMOVING_ALL_FLAGS_IN_REQUEST_BUCKET;   // remove old flags
        GroupData2 |=  Flags;           // add new flags
    }
    FLG getFlags(){
        return (GroupData2 & MASK_FOR_ALL_FLAGS_IN_REQUEST_BUCKET);
    }
    // dump info of the class
    void DumpToScreen(){
        cout<< showbase << "F : " << F << endl;
        cout<< showbase << "G : " << G << endl;
        cout<< showbase << "H : " << H << endl;
        cout<< showbase << "Hop Info : " << HopInfo << endl;
        cout<< showbase << "R Then : " << Then << endl;
        cout<< showbase << "R Else : " << Else << endl;
        cout<< showbase << "GroupData 1 : " << GroupData1 << endl;
        cout<< showbase << "GroupData 2 : " << GroupData2 << endl;
        cout << setbase (10) ;
    }
    void Dump(ofstream & ff){
        ff<< showbase << "F : " << (T) F << endl;
        ff<< showbase << "G : " << (T) G << endl;
        ff<< showbase << "H : " << (T) H << endl;
        ff<< showbase << "Hop Info : " << (T) HopInfo << endl;
        ff<< showbase << "R Then : " << (T) Then << endl;
        ff<< showbase << "R Else : " << (T) Else << endl;
        ff<< showbase << "GroupData 1 : " << (T) GroupData1 << endl;
        ff<< showbase << "GroupData 2 : " << (T) GroupData2 << endl;
        ff << setbase (10) ;
    }
    void DumpToFile(ofstream & ff){
        ff<<  (T) F << endl;
        ff<<  (T) G << endl;
        ff<<  (T) H << endl;
        ff<<  (T) HopInfo << endl;
        ff<<  (T) Then << endl;
        ff<<  (T) Else << endl;
        ff<<  (T) GroupData1 << endl;
        ff<<  (T) GroupData2 << endl;
    }
    void setFrecord(T F){
        this->F = F;
    }
    void setGrecord(T G){
        this->G = G;
    }
    void setHrecord(T H){
        this->H = H;
    }
    void setThenrecord(T Then){
        this->Then = Then;
    }
    void setElserecord(T Else){
        this->Else = Else;
    }
    void setHopInforecord(T HopInfo){
        this->HopInfo = HopInfo;
    }
    void setGroupData1record(T GroupData1){
        this->GroupData1 = GroupData1;
    }
    void setGroupData2record(T GroupData2){
        this->GroupData2 = GroupData2;
    }
};
#endif