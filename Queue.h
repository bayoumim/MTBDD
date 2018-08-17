// this class implement a queue that store a unique set of keys (so it is some how very close to hash table)

#ifndef _QUEUE_CLASS
#define _QUEUE_CLASS

#include "BDDUtil.h"
#include "Request.h"
#include "RWLock.h"
#include "HashQueue.h"
#include "mutex.h"
#include "PointerQueue.h"
// Define options for GC (mark), 
enum
{
    Q_GC_MARK_F,         		/* 0 */
    Q_GC_MARK_G,         		/* 1 */
    Q_GC_MARK_H,         		/* 2 */
    Q_GC_MARK_THEN,         	/* 3 */
    Q_GC_MARK_ELSE			/* 4 */
};

template<typename T,typename V,typename L,typename K,typename FLG>
class Queue{
private:
    HashQueue <T,V,L,K,FLG>* hashTable;
    volatile T *head;	// COUNTER corresponding to requests
    volatile T *nodeCount;	// counter corresponding to nodes
    Mutex *HeadLock;
    vector < PointerQueue<T,V> > bucketList;
    L VariableCount;
    unsigned char hashIntegrityCheck;
    IntegrityChecker <T,V,L,K,FLG> * integrityChecker;
    unsigned char GCEnabled;
//	GCQueue * gcQueue;
    Hash <T,V,L,K,FLG>* UniqueTable;	// pointer to unique table
    volatile unsigned char markLevel;		// the current level of the mark GC
    vector < BDDPointer <T,V> > MarkedList;
public:
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // constructor
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    Queue(T inCapacity,
            unsigned short concurrencyDegree,
            double MaxUtilization,
            unsigned char BucketSize,
            bool enableComputedTable,
            L VariableCount,
            unsigned char hashIntegrityCheck,
            IntegrityChecker <T,V,L,K,FLG> *& integrityChecker,
            unsigned char GCEnabled,
            /*GCQueue * gcQueue,*/ Hash <T,V,L,K,FLG> * UniqueTable) {

        int i;
        this->UniqueTable = UniqueTable;
//	this->gcQueue = gcQueue;
        this->GCEnabled = GCEnabled;
        this->hashIntegrityCheck = hashIntegrityCheck;
        this->integrityChecker = integrityChecker;
        this-> VariableCount = VariableCount;

        nodeCount = new T [ VariableCount ];
        head = new T [ VariableCount ];
        HeadLock = new Mutex[ VariableCount ];	// create lock for every level

        for (i = 0; i < VariableCount ; i++) {
            nodeCount[i] = -1;
            head[i] = -1;
            bucketList.push_back( *(new PointerQueue <T,V>()) );
        }
        hashTable = new HashQueue <T,V,L,K,FLG> (inCapacity, concurrencyDegree, MaxUtilization, BucketSize, enableComputedTable, VariableCount, hashIntegrityCheck, integrityChecker, GCEnabled /*, gcQueue*/ , UniqueTable);
        markLevel = 0;
    }
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // Methods used in APPLY
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // enqueue request
    bool findOrEnqRequest(Request <T,V,L> &req, BDDPointer <T,V> &bp, unsigned char &flag, L currentLevel){
        // store the pointer in the list
        RequestKey <T,V> key = req.getKey();
        L level = hashTable->getTopVarIndex(key) - 1;

        if(level>=currentLevel){
            cout << "queue : findOrEnqRequest : error in calculating the level of the key" << endl;
            key.DumpToScreen();
            hashTable->getTopVarIndex(key);
            exit(-1);
        }

        if ((hashTable->findOrAdd(req,bp,flag,level))){	// if hash table already have this request, update the pointer and return
       /*     if (flag == HQRZ_NODE_NOT_ADDED)
                cout << "queue : findOrEnqRequest : no of node in level : " << level << " is " <<  nodeCount[level] << endl << "no of head is : " << head[level] << endl;;*/
            return true;	// the element is successfully enqueued (in this case, we just returned as the request is already there )
        }
        if(bp.getHashString() == 235957760){
            cout << "queue : findOrEnqRequest: pointer with hash string : " << bp.getHashString() << " inqueued" << endl;
        }
        if(bp.isTemporary()){	// if the pointer is temporary -> store it in hte list, otherwise, return (regular pointers only happens if computed table is enabled
            HeadLock[level].lock();		// aquire read lock
            ++head[level];		// increment head pointer
            ++nodeCount[level];
            bucketList[level].push(bp);	// insert the new bucket
            HeadLock[level].unlock();		// unlock
        }
        else {
            cout << "================================================================================================================================" << endl;
            cout << "COMPUTED TABLE IS ENABLED" << endl;
            cout << "================================================================================================================================" << endl;
        }
        return true;
    }
    bool ForcedFindOrEnqRequest(Request <T,V,L> &req, BDDPointer <T,V> &bp, unsigned char &flag, L currentLevel){
        // store the pointer in the list
        RequestKey <T,V> key = req.getKey();
        L level = hashTable->getTopVarIndex(key) - 1;

        if(level>=currentLevel){
            cout << "queue : findOrEnqRequest : error in calculating the level of the key" << endl;
            key.DumpToScreen();
            hashTable->getTopVarIndex(key);
            exit(-1);
        }

        if ((hashTable->ForcedFindOrAdd(req,bp,flag,level))){	// if hash table already have this request, update the pointer and return
       /*     if (flag == HQRZ_NODE_NOT_ADDED)
                cout << "queue : findOrEnqRequest : no of node in level : " << level << " is " <<  nodeCount[level] << endl << "no of head is : " << head[level] << endl;;*/
            return true;	// the element is successfully enqueued (in this case, we just returned as the request is already there )
        }
        if(bp.getHashString() == 235957760){
            cout << "queue : findOrEnqRequest: pointer with hash string : " << bp.getHashString() << " inqueued" << endl;
        }
        if(bp.isTemporary()){	// if the pointer is temporary -> store it in hte list, otherwise, return (regular pointers only happens if computed table is enabled
            HeadLock[level].lock();		// aquire read lock
            ++head[level];		// increment head pointer
            ++nodeCount[level];
            bucketList[level].push(bp);	// insert the new bucket
            HeadLock[level].unlock();		// unlock
        }
        else {
            cout << "================================================================================================================================" << endl;
            cout << "COMPUTED TABLE IS ENABLED" << endl;
            cout << "================================================================================================================================" << endl;
        }
        return true;
    }
    // dequeue request
    // fetch request based on certian pointer, the request will not be removed from hash table , this method is used in dequeing request in apply method
     bool dequeRequest (Request <T,V,L> &req, L level, bool &ThenProcessedFlag, bool &ElseProcessedFlag) {
        HeadLock[level].lock();		// aquire read lock
        if (head[level] == (T)(-1)){
            HeadLock[level].unlock();		// unlock
            return false;
        }
        BDDPointer <T,V> bp;
        bp = bucketList[level].getElementFromFront();	// get the pointer to request
        if(bp.getHashString() == 235957760){
            cout << "queue : dequeRequest: pointer with hash string : " << bp.getHashString() << " will be dequed" << endl;
        }
        head[level] --;
        HeadLock[level].unlock();		// unlock
        return hashTable-> fetchRequest (bp, req, ThenProcessedFlag, ElseProcessedFlag);	// fetch request;
    }
    // overloaded version of putRequest function
    bool putRequest(BDDPointer <T,V> & Fx, BDDPointer <T,V> & Gx, BDDPointer <T,V> & Hx, BDDNode <T,V,L> &R, bool ThenProcessedFlag, bool ElseProcessedFlag){
        Request <T,V,L> req (Fx,Gx,Hx,R);
        return putRequest (req, ThenProcessedFlag, ElseProcessedFlag);
    }
    // this function overwrite an already exist request (after some update to its temporary node), it would be similar to contains, but when you find data, overwrite it.
    bool putRequest (Request <T,V,L> &req, bool ThenProcessedFlag, bool ElseProcessedFlag) {
        return hashTable->putRequest (req, ThenProcessedFlag, ElseProcessedFlag);
    }
    bool hasSpace(L level){
        return hashTable->hasSpace(level);
    }
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // Methods used in REDUCE
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    bool putOldRequest(Request <T,V,L> &req, bool ExistNodeFlag, BDDPointer <T,V> & bp)  {
        return hashTable->putOldRequest (req, ExistNodeFlag,bp);
    }
    // it fetch a temporary node from queue based on its pointer, (the fetched node will remain in the queue as well)
    bool fetchNode (BDDPointer <T,V> &bp, BDDNode <T,V,L> &node){
        bool flag =  hashTable->fetchNode(bp,node);
        return flag;
    }
    // fetch request based on certian pointer, the request will not be removed from hash table , this method is used in dequeing request in apply method
     bool dequeRequestWithoutLocking (Request <T,V,L> &req, L level, bool &ThenProcessedFlag, bool &ElseProcessedFlag) {
        HeadLock[level].lock();		// aquire read lock
        if (head[level] == (T)(-1)){
                HeadLock[level].unlock();		// unlock
                return false;
        }
        BDDPointer <T,V> bp;
        bp = bucketList[level].getElementFromFront();	// get the pointer to request

        if(bp.getHashString() == 235957760){
            cout << "queue : dequeRequestWithoutLocking: pointer with hash string : " << bp.getHashString() << " will be dequed" << endl;
        }
        head[level] --;
        HeadLock[level].unlock();		// unlock
        if(bp.getHashString() == 172438488){
            cout << "queue : dequeRequestWithoutLocking" << endl;
        }
        return hashTable-> fetchRequestWithoutLocking (bp,req, ThenProcessedFlag, ElseProcessedFlag);	// fetch request;
    }
    void freeOneNode(BDDPointer <T,V> bp, L level){
        hashTable->freeOneNode();
        bucketList[level].remove(bp);
        HeadLock[level].lock();		// aquire read lock
        nodeCount[level]--;
        HeadLock[level].unlock();		// unlock
    }
    bool putRequestWithoutProcessing (Request <T,V,L> &req, bool thenPointerFinishedFlag, bool elsePointerFinishedFlag){
        return hashTable->putRequestWithoutProcessing(req, thenPointerFinishedFlag, elsePointerFinishedFlag);
    }
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // Methods used in MANAGER
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // this function reload the queue (its effect is the same as you don't denque any element), it is used at teh beginning of reduce function
    void ReLoad () {
        int i;
        for (i =0 ; i < VariableCount ; i++) {
            head[i] = nodeCount[i];
            bucketList[i].reLoad();
        }
    }
    // this function reset the queue (its effect is the same as you don't enque any element), it is used at teh end of reduce function
    void Reset () {
        int i;
        for (i =0 ; i < VariableCount ; i++) {
            bucketList[i].reset();
            head[i] = nodeCount[i] = -1;
        }
    }
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // get internal parameters
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    void DumpToScreen(){
        int i;
        for (i =0 ; i < VariableCount ; i++) {
            cout << "Queue no. : " << i << endl;
            cout << "node Count : " << nodeCount[i] << endl;
            cout << "head : " << head[i] << endl;
        }
        hashTable->DumpToScreen();
    }
    void Dump(ofstream & ff){
        int i;
        for (i =0 ; i < VariableCount ; i++) {
            ff << "node Count : " << nodeCount[i] << endl;
            ff << "head : " << head[i] << endl;
        }
        hashTable->Dump(ff);
    }
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // auxillary functons
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // this is the mature version of get element
    // get element with pointer bp, this used at the end of BFIte method in BFS.
    void getElement(BDDPointer <T,V> & bp ){
        BDDNode <T,V,L> node;		// temporary node
        fetchDatedNode (bp, node);		// get temporary node
        if (node.isForwarded (bp)){		// if the node is forwarded, then, get pointer to to node in hash table corrresponding to temporary node
        /*	if (GCEnabled == H_GC_RF_ENABLED){	// if GC is enabled then; increase the reference the forwarded node pointers, (this used to increment pointers for Gatelist and for Operation list in operation schedular)
                        gcQueue->Ref(bp);
                }*/
        }
        return ;
    }
    // calculate the top variable index of three nodes (depends on their id)
    L getTopVarIndex (BDDPointer <T,V> F, BDDPointer <T,V> G, BDDPointer <T,V> H) {
        return this->hashTable->getTopVarIndex(F,G,H);
    }
    void validate(){
        L i;
        for (i = 0; i < VariableCount ; i++){
            cout << "queue : validate: level : " << i << " has nodes count: " << nodeCount[i] << endl;
            if(nodeCount[i] != (T)-1){
                cout << "queue : validate : error" << endl;
                cout << "queue at level : " << i << " still contains nodes : " << nodeCount[i] << " nodes" << endl;
                exit(-1);
            }

        }
    }
    T getNodeCount() {
        return hashTable->getNodeCount();
    }
    void resetFreeNodeCounter(){
        hashTable->resetFreeNodeCounter();
    }
    T getFreedNodeCount(){
        return hashTable->getFreedNodeCount();
    }
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // get hash internal parameters (for debugging and internal validation purposes)
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    bool fetchDatedNode (BDDPointer <T,V> &bp, BDDNode <T,V,L> &node){
        return hashTable->fetchNode(bp, node);
    }
    // dump hash table (unique table) to a file, for integrity purposes ...
    void DumpQueue(int fileNo){
        hashTable->DumpQueue(fileNo);
    }
    // dump hash table (unique table) to a file, for integrity purposes ...
    void DumpQueueAfterApply(int fileNo){
        hashTable->DumpQueueAfterApply(fileNo);
    }
    // dump hash table (unique table) to a file, for integrity purposes ...
    void DumpQueueAfterReduce (int fileNo){
        hashTable->DumpQueueAfterReduce (fileNo);
    }
};
#endif 
