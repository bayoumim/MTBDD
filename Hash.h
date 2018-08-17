#ifndef _HASH_CLASS
#define _HASH_CLASS
// this class represents the unique table in our package

#include "BDDUtil.h"
#include "Bucket.h"
#include "Segment.h"
#include "mutex.h"
#include "GCHashNode.h"
#include "Condition.h"
#include "IntegrityChecker.h"
//#include "GCQueue.h"
// Define options for hash resizing phases
enum
{
    H_RESIZE_INITIAL_PHASE,         			/* 0 */
    H_RESIZE_MAP_ID_PHASE,        			/* 1 */
    H_RESIZE_MAP_CHILD_POINTERS_PHASE,        		/* 2 */
    H_RESIZE_FINISHING_PHASE                            /* 3 */
};

//template 
// T and V are types of the fields in the BDDPointer,
// L: is the type of the level (Default: unsigned short)
// K : is the key type (default : int)
template<typename T,typename V,typename L,typename K,typename FLG>
class Hash{
protected:	
    // private members
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // Basic paramters : which is related to building tables and segments
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    double MaxUtilization;			// determine the utilization percentage of the table
    T inCapacity;				// determine the  input capacity of the table
    unsigned short ConcurrencyDegree;	// determine number of threads
    volatile T TotalCapacity; 		// determine the total capacity of the table
    Segment * Segments;			// pointer to the segments
    volatile T BucketSize;			// determine the size of the neighborhood
    volatile T SegmentSize;			// total number of segments
    volatile T BucketMask;			// used to mask key
    volatile T NodeCounter;			// keep track of the number of stored nodes
    BDDPointer <T,V> One;                  // pointer to one node
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // debugging purposes parameters
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    Mutex mutex;				// mutex for node counter and printing to console (cout)
    Mutex mutexFile;			// mutex for dumping to file
    unsigned short fileCounter;

    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // resizing parameters
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // shared parameters
    RWLock MasterResizeLock;				// lock for resizing parameters
    volatile bool resizeOnging;			// flag indicate wether the resizing is onging or not.
    volatile T NextBucketMask;			// the bucket mask of the next table after resizing
    volatile T NextTotalCapacity;			// the total capacity of the next table after resizing
    volatile unsigned char NoOfLookUpTables;	// no. of resizing operation
    unsigned char ResizeType;
    volatile bool ResizingFlag;			// flag used to indicate if the resizing is occured or not.
    volatile bool ResizingOccured;			// indicate whether the resizing is occured at least one time
    volatile T SegmentSizeForNewTable;		// total number of segments
    volatile bool InitialResizePhaseFlag;	// flag to indicate whether the first phase in resizing is onging or not.
    volatile bool FinalResizePhaseFlag;	// flag to indicate whether the first phase in resizing is onging or not.
    volatile char CurrentPhase;	// store the current phase in resizing operation
    volatile unsigned short CurrentSegment;	// store the current segment (used during resizing)
    volatile bool FirstSegmentFinishedMap;	// this flag used to indicate wether the first segment have been finished in mapping its element or not NOT
    Mutex FirstSegmentLock;	// lock for the first segment (whether it is finished or not)
    Condition *ResizingCondition; // condition to make threads sleep while resizing
    Condition *FirstPhaseCondition;
    Mutex FirstPhaseLock;			// lock for the second phase condition
    Condition *SecondPhaseCondition; 	// condition for the second phase in resize
    Mutex SecondPhaseLock;			// lock for the second phase condition
    Condition *ThirdPhaseCondition; 	// condition for the second phase in resize
    Mutex ThirdPhaseLock;			// lock for the second phase condition
    Condition *FourthPhaseCondition; 	// condition for the second phase in resize
    Mutex FourthPhaseLock;			// lock for the second phase condition
    volatile int SecondPhaseFinishedCurrentSegment;
    volatile int FinishedCurrentSegment;	// store no. of finished current segment (used during resizing). it indicates how many threads finished its 'third'
    // swap resizing parameters
    Bucket <T,V,L> * Table; 		// pointer to the table (a single chunk of data)
    Bucket <T,V,L> * newTable;		// pointer to new table
    Bucket <T,V,L> * TempTable;		// pointer to temporary table (it points to the old table, to be removed from the memory when the initialize resizing called)

    // incremental resizing parameters
    Bucket <T,V,L> ** TableAddressList; 		// hold addresses of other parts of the table (utilized when resizing is occurs to fetch the appropriete part of the unique table)
    T initialCapacityPower;	// the power of the initial capacity (TotalCapacity = 2^initialCapacityPower)
    volatile char TotalNoOfTableParts;

    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // GC parameters
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // GC parameters
    int GCEnabled;
    unsigned char CURRENT_MARK;	// this field is used ONLY, when the 'mark' GC is enabled, it holds the current mark
//	GCQueue * gcQueue;

    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // integrity checker parameters
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    unsigned char hashIntegrityCheck;
    IntegrityChecker <T,V,L,K,FLG>* integrityChecker;

/*
part in resizing. it is used to allow master gaurantee that ALL other threads finished thier third part before master make swap between old and new table (and other shared stuff)
    // hash single key
*/
    //============================================================================================================================================================
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // basic Functions
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    //============================================================================================================================================================

    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // initial capacity evaluation related functions
    //------------------------------------------------------------------------------------------------------------------------------------------------------------

    // get no. of bits to represent a number
    T getNoOfBits (T Number) {
        T bitsCount = 0;
        do {
            bitsCount ++;
            Number >>= 1;
        }
        while ( Number!= 0);
        return bitsCount;
    }
    // convert 'n' to '2^n'
    T getPowerOf2(T number) {
        return 1<<(number);
    }
    // approximate teh number to higher value of 2
    T ApproximateToBiggerPowerOf2(T number){
        initialCapacityPower = getNoOfBits(number);
        return getPowerOf2(initialCapacityPower);
    }


    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // hash code related functions
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    K hashSingleKey(K key)
    {
        key = ~key + (key << 15);
        key = key ^ (key >> 12);
        key = key + (key << 2);
        key = key ^ (key >> 4);
        key = (key + (key << 3)) + (key << 11);
        key = key ^ (key >> 16);
        return key;
    }
    // compute the hash string for certain node
    T getHashString(BDDNode <T,V,L> &node){
        K t = node.getThen().composePointer();
        K e = node.getElse().composePointer();
        K l = node.getLevel();
        t = hashSingleKey(t);
        e = hashSingleKey(t^e);
        l = hashSingleKey(l^e);
        return l;
    }
    // compute the bucket
    T getHashCode(T l){
        return l & (this->BucketMask); 	// round the key to be in the range
    }
    T getHashCodeWithLock(T l){
        T hashCode;
        MasterResizeLock.ReadLock();
        hashCode = l & (this->BucketMask);
        return hashCode; 	// round the key to be in the range
    }
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // segments related functions
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // calculate the corresponding segment to certain hash code
    short getSegment(T hashCode){
        short seg = floor((float)(hashCode/SegmentSize)) ;
        return seg;
    }

    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // hop information related functions
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // upadate info field (this is required when swapping is exploited)
    void updateInfo(T bucket, T oldlocation, T newlocation){
        // we don't have to lock this function, as the programmer will lock it from outside
        if(ResizeType == H_RESIZE_TYPE_SWAP){
            if( (oldlocation < bucket) || (newlocation < bucket) ){
                cout<< "error in update info function";
                return;
            }
            unsigned char oldshift = BucketSize - (oldlocation - bucket) - 1;
            unsigned char newshift = BucketSize - (newlocation - bucket) - 1;
            T temp = Table[bucket].getHopInfo();
            temp = (temp & (MASK_ALL_BITS_IN_HOP_INFO_IN_HASH_TABLE - ( 1 << oldshift ) ) ); // unset the old location
            temp = (temp | (1<<newshift) );	// set the new location
            Table[bucket].setHopInfo(temp);	// store the new value of Hop Information
            return;
        }
        else{
            T threshold;
            // T
            unsigned char partNo = getPartOfTable(bucket,threshold);
            T unbaisedLoc  = (bucket - threshold);
            if( (oldlocation < bucket) || (newlocation < bucket) ){
                cout<< "error in update info function";
                return;
            }
            unsigned char oldshift = (BucketSize - (oldlocation - bucket) - 1);
            unsigned char newshift = BucketSize - (newlocation - bucket) - 1;
            T temp = TableAddressList[partNo][unbaisedLoc].getHopInfo();
            temp = (temp & (MASK_ALL_BITS_IN_HOP_INFO_IN_HASH_TABLE - ( 1 << oldshift ) ) ); // unset the old location
            temp = (temp | (1<<newshift) );	// set the new location
            TableAddressList[partNo][unbaisedLoc].setHopInfo(temp);	// store the new value of Hop Information
            return;
        }
    }
    // add a new item to the bucket list
    void AddNewItemInfo(T bucket, T itemLocation){
        // debugging ADDNewItemInfo
        //cout << "bucket no. : " << bucket << endl;
        //cout << "insert element at. : " << itemLocation << endl;
        if(ResizeType == H_RESIZE_TYPE_SWAP){
            if (itemLocation < bucket){
                cout << "hash: Invalid input : AddNewItemInfo Failed" << endl;
                exit(-1);
            }
            T shift = BucketSize - (itemLocation - bucket ) - 1;
            T temp = Table[bucket].getHopInfo();
            shift = (1<<shift);
            //cout << "shifts" << hex << shift << setbase (10) << endl;
            temp = (temp | shift ); // set the old location
            Table[bucket].setHopInfo(temp);
            return;
        }
        else{
            T threshold;
            unsigned char partNo = getPartOfTable(bucket,threshold);
            T unbaisedLoc  = bucket - threshold;
            if (itemLocation < bucket){
                cout << "Invalid input : AddNewItemInfo Failed" << endl;
                exit(-1);
            }
            T shift = BucketSize - (itemLocation - bucket ) - 1;
            T temp = TableAddressList[partNo][unbaisedLoc].getHopInfo();
            shift = (1<<shift);
            temp = (temp | shift ); // set the old location
            TableAddressList[partNo][unbaisedLoc].setHopInfo(temp);
            return;
        }
    }

    // remove item from bucket list
    void removeItemInfo(T bucket, T itemLocation){
        // debugging removeItemInfo
        //cout << "bucket no. : " << bucket << endl;
        //cout << "insert element at. : " << itemLocation << endl;
        if(ResizeType == H_RESIZE_TYPE_SWAP){
            if (itemLocation < bucket)
                cout << "Invalid input : removeItemInfo Failed" << endl;
            T shift = BucketSize - (itemLocation - bucket ) - 1;
            T temp = Table[bucket].getHopInfo();
            shift = (1<<shift);
            shift = shift ^ MASK_ALL_BITS_IN_HOP_INFO_IN_HASH_TABLE;	// invert all bits
            //cout << "shifts" << hex << shift << setbase (10) << endl;
            temp = (temp & shift ); // reset the old location
            Table[bucket].setHopInfo(temp);
            return;
        }
        else{
            T threshold;
            unsigned char partNo = getPartOfTable(bucket,threshold);
            T unbaisedLoc  = bucket - threshold;
      /*      if (bucket == 1025){
                mutex.lock();
                cout << "removing item " << itemLocation <<  " from bucket " << bucket << "."  << endl;
                unsigned char partNo = getPartOfTable(itemLocation,threshold);
                T unbaisedLoc  = itemLocation - threshold;
                TableAddressList[partNo][unbaisedLoc].DumpToScreen();
                mutex.unlock();
            }*/
            if (itemLocation < bucket)
                cout << "Invalid input : removeItemInfo Failed" << endl;
            T shift = BucketSize - (itemLocation - bucket ) - 1;
            T temp = TableAddressList[partNo][unbaisedLoc].getHopInfo();
            shift = (1<<shift);
            shift = shift ^ 0xFFFFFFFF;	// invert all bits
            //cout << "shifts" << hex << shift << setbase (10) << endl;
            temp = (temp & shift ); // reset the old location
            TableAddressList[partNo][unbaisedLoc].setHopInfo(temp);
            return;
        }
    }

    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // Functions related to resizing in general
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
            // compute the hash function for the key (triple key)
    T getHashStringForNewTable (BDDNode <T,V,L> &key){
        K t = key.getThen().composePointer();
        K e = (K)key.getElse().composePointer();
        K l = (K)key.getLevel();
        t = hashSingleKey(t);
        e = hashSingleKey(t^e);
        l = hashSingleKey(l^e);
        return l;
    }
    T getHashCodeForNewTable(T l){
        return l & (this->NextBucketMask); 	// round the key to be in the range
    }
    // add a new item to the bucket list
    void AddNewItemInfoForNewTable(int bucket, int itemLocation){
        if (itemLocation < bucket){
            cout << "Invalid input : AddNewItemInfoForNewTable Failed" << endl;
            exit(-1);
        }
        unsigned int shift = BucketSize - (itemLocation - bucket ) - 1;

        if(ResizeType == H_RESIZE_TYPE_SWAP){	// if swap resizing is enabled
            unsigned int temp = newTable[bucket].getHopInfo();
            shift = (1<<shift);
            //cout << "shifts" << hex << shift << setbase (10) << endl;
            temp = (temp | shift ); // set the old location
            newTable[bucket].setHopInfo(temp);
        }
        else{
            unsigned char partNo;
            T unbaisedLoc;
            T threshold;
            if(itemLocation == 2050){
                mutex.lock();
                cout << "item will be mapped to location " << itemLocation << endl;
                partNo = getPartOfTable(bucket,threshold);
                unbaisedLoc  = bucket - threshold;
                TableAddressList[partNo][unbaisedLoc].DumpToScreen();
                mutex.unlock();
            }
            partNo = getPartOfTable(bucket,threshold);
            unbaisedLoc  = bucket - threshold;
            T temp = TableAddressList[partNo][unbaisedLoc].getHopInfo();
            shift = (1<<shift);
            //cout << "shifts" << hex << shift << setbase (10) << endl;
            temp = (temp | shift ); // set the old location
            TableAddressList[partNo][unbaisedLoc].setHopInfo(temp);
        }
        return;
    }
    // function to add item to a new table (this funciton called in resizing function)
    void findOrAddToNewTable(BDDNode <T,V,L> &node, int refCount, int newlocation, V localId){
        if(ResizeType == H_RESIZE_TYPE_SWAP){	// if swap resizing is enabled
            T hashString = getHashStringForNewTable(node);	// compute hash String
            T hashCode = getHashCodeForNewTable(hashString);	// compute hash code
            if (!newTable[newlocation].isEmpty()){	// if there is a node in this location , then get out
                cout << "hash : findOrAddToNewTable : node will overwrite another node " << endl;
                cout << "old node : (already stored and will be overwritten)" << endl;
                newTable[newlocation].DumpToScreen();
                cout << " new node" << endl;
                node.DumpToScreen();
                exit(-1);
            }
            newTable[newlocation].clearAllFlags();			// as this a new request, dated flag must be cleared
            newTable[newlocation].setNode(node);		// add your node
            newTable[newlocation].setReferenceCounter(refCount);		// add reference count
            newTable[newlocation].setLocalId(localId);
            AddNewItemInfoForNewTable (hashCode,newlocation);	// update info
        }
        else{
            T hashString = getHashStringForNewTable(node);	// compute hash String
            T hashCode = getHashCodeForNewTable(hashString);	// compute hash code
            unsigned char partNo;
            T unbaisedLoc;
            T threshold;
            partNo = getPartOfTable(newlocation,threshold);
            unbaisedLoc  = newlocation - threshold;

            if (!TableAddressList[partNo][unbaisedLoc].isEmpty()){	// if there is a node in this location , then get out
                cout << "hash : findOrAddToNewTable : node will overwrite another node " << endl;
                cout << "old node : (already stored and will be overwritten)" << endl;
                TableAddressList[partNo][unbaisedLoc].DumpToScreen();
                cout << " new node" << endl;
                node.DumpToScreen();
                exit(-1);
            }
            if (newlocation == 4096){
                cout << "hash : findOrAddToNewTable : node will be add at location : " << newlocation << endl;
            }

            TableAddressList[partNo][unbaisedLoc].clearAllFlags();			// as this a new request, dated flag must be cleared
            TableAddressList[partNo][unbaisedLoc].setNode(node);		// add your node
            TableAddressList[partNo][unbaisedLoc].setReferenceCounter(refCount);		// add reference count
            TableAddressList[partNo][unbaisedLoc].setLocalId(localId);
            AddNewItemInfoForNewTable (hashCode,newlocation);	// update info
        }
    }
    void findNearestEmptyLocationForNewTable(T start, T & location){
        if(ResizeType == H_RESIZE_TYPE_SWAP){	// if swap resizing is enabled
            if (start >= NextTotalCapacity) {	//if you reached the last place in the hash, return false
                cout << "Hash : enable to resize : findNearestEmptyLocationForNewTable 1" << endl;
                exit(-1);
            }

            while( !newTable[start].isEmpty() ){
                start++;

                if (start == NextTotalCapacity) {	//if you reached the last place in the hash, return a number bigger than the capacity of the table
                    cout << "Hash : enable to resize : findNearestEmptyLocationForNewTable 2" << endl;
                    exit(-1);
                }
            }
        }
        else{	// if incremental resizing is enabled
            if (start >= NextTotalCapacity) {	//if you reached the last place in the hash, return false
                cout << "Hash : enable to resize : findNearestEmptyLocationForNewTable 1" << endl;
                exit(-1);
            }
            unsigned char partNo;
            T unbaisedLoc;
            T threshold;
            partNo = getPartOfTable(start,threshold);
            unbaisedLoc  = start - threshold;

            while( !TableAddressList[partNo][unbaisedLoc].isEmpty() ){
                start++;
                partNo = getPartOfTable(start,threshold);
                unbaisedLoc  = start - threshold;

                if (start == NextTotalCapacity) {	//if you reached the last place in the hash, return a number bigger than the capacity of the table
                    cout << "Hash : enable to resize : findNearestEmptyLocationForNewTable 2" << endl;
                    exit(-1);
                }
            }
        }

        location = start;	// store the new value
        return;
    }

    // upadate info field (this is required when swapping is exploited)
    void updateInfoForNewTable(T bucket, T oldlocation, T newlocation){
        // we don't have to lock this function, as the programmer will lock it from outside
        if( (oldlocation < bucket) || (newlocation < bucket) ){
            cout<< "error in update info function : new table : hash";
            return;
        }
        unsigned short oldshift = BucketSize - (oldlocation - bucket) - 1;
        unsigned short newshift = BucketSize - (newlocation - bucket) - 1;

        if(ResizeType == H_RESIZE_TYPE_SWAP){	// if swap resizing is enabled
            T temp = newTable[bucket].getHopInfo();
            temp = (temp & (0xFFFFFFFF - ( 1 << oldshift ) ) ); // unset the old location
            temp = (temp | (1<<newshift) );	// set the new location
            newTable[bucket].setHopInfo(temp);	// store the new value of Hop Information
        }
        else{		// if incremental resizing is enabled
            unsigned char partNo;
            T unbaisedLoc;
            T threshold;
            partNo = getPartOfTable(bucket,threshold);
            unbaisedLoc  = bucket - threshold;

            T temp = TableAddressList[partNo][unbaisedLoc].getHopInfo();
            temp = (temp & (0xFFFFFFFF - ( 1 << oldshift ) ) ); // unset the old location
            temp = (temp | (1<<newshift) );	// set the new location
            TableAddressList[partNo][unbaisedLoc].setHopInfo(temp);	// store the new value of Hop Information
        }
        return;
    }

    // function to add item to a new table (this funciton called in resizing function)
    void findOrAddToNewTable(BDDNode <T,V,L> &node, int refCount, V localId){
        T hashString = getHashStringForNewTable(node);	// compute hash String
        T hashCode = getHashCodeForNewTable(hashString);	// compute hash code
        T emptyLoc;

        findNearestEmptyLocationForNewTable(hashCode,emptyLoc);

        T info1;
        T loc1;
        // hop themptyLoce empty location all the way toward its bucket
        T i;
        bool flag;
        T hashStringTemp;
        T hashCodeTemp;
        if(ResizeType == H_RESIZE_TYPE_SWAP){	// if swap resizing is enabled
                while ( (emptyLoc - hashCode) >= BucketSize){
                        flag = false;	// initialization for the next round
                        T start = emptyLoc - (BucketSize-1);
                        for(i =  start; i < emptyLoc ; i++) {
                                info1 = newTable[i].getHopInfo();	// get info for this bucket
                                while(getNearestNeighbor(i,info1,loc1)){
                                        BDDNode <T,V,L> tempNode = newTable[loc1].getNode();
                                        hashStringTemp = getHashStringForNewTable(tempNode);	// compute hash String
                                        hashCodeTemp = getHashCodeForNewTable(hashString);	// compute hash code

                                        if (hashCodeTemp == i){ // if the hashcode for the target bucket equal to the hashcode of the request
                                                if ((loc1 < emptyLoc)) {	// if the bucket isn't reserved, you can swap
                                                        flag = true;	//rise the flag
                                                        newTable[emptyLoc].setFlags(newTable[loc1].getFlags());
                                                        BDDNode <T,V,L> req = newTable[loc1].getNode();
                                                        newTable[emptyLoc].setNode(req);	// swap the nodes
                                                        newTable[emptyLoc].setReferenceCounter(newTable[loc1].getReferenceCounter());	// swap Refenerence counters
                                                        newTable[emptyLoc].setLocalId(newTable[loc1].getLocalId());

                                                        updateInfoForNewTable (i,loc1,emptyLoc);	// update info
                                                /*	if(node.getId() == 0x24000001){
                                                                mutex.lock();
                                                                cout << "hash : findOrAddToNewTable 1 : node with id : " << node.getId() << " and hash code : " << i << " will be move from location : " << loc1 << " to new location : " << emptyLoc << endl;
                                                                mutex.unlock();
                                                        }*/
                                                        emptyLoc = loc1;			// update the new empty location
                                                        break;
                                                }
                                                else {
                                                        continue;
                                                }
                                        }
                                        else{
                                                mutex.lock();
                                                cout << "hash : findOrAddToNewTable : error in swapping nodes" << endl;
                                                cout << "i =  " << i << " hashCodeTemp = : " << hashCodeTemp << " loc1 : " << loc1 << endl;
                                                newTable[loc1].DumpToScreen();
                                                mutex.unlock();
                                                exit(-1);
                                        }
                                }
                                if(flag){
                                        break;				// exit "for loop" as we reach the nearest bucket in this round
                                }
                        }
                        // here we should check the flag, because if it isn't been rised, we have no space and the table will need to be resized and you have to unlock the write lock
                        if(flag){
                                continue;				// continue , you have make one swap, so you have to check whether you are in the
                        }

                        if (( i == start + BucketSize-1 ) && (!flag)) {
                                mutex.lock();
                                cout << "Hash : enable to resize : findOrAddToNewTable" << endl;
                                mutex.unlock();
                                exit(-1);
                        }
                }
                if (hashCode == 2036 ){
                        mutex.lock();
                        cout << "hash : findOrAddToNewTable 2: node with hashCode : " << hashCode << " will be added at location " << emptyLoc << endl;
                        node.DumpToScreen();
                        mutex.unlock();
                }

                newTable[emptyLoc].clearAllFlags();			// as this a new request, dated flag must be cleared
                newTable[emptyLoc].setNode(node);		// add your node
                newTable[emptyLoc].setReferenceCounter(refCount);		// add reference count
                newTable[emptyLoc].setLocalId(localId);
                AddNewItemInfoForNewTable (hashCode,emptyLoc);	// update info
        }
        else{		// if incremental resizing is enabled
            unsigned char partNo,partNo1;
            T unbaisedLoc, unbaisedLoc1;
            T threshold, threshold1;
            partNo = getPartOfTable(emptyLoc,threshold);
            unbaisedLoc  = emptyLoc - threshold;
            while ( (emptyLoc - hashCode) >= BucketSize){
                cout << "hash: findOrAddToNewTable : nodes will be swapped " << endl;
                flag = false;	// initialization for the next round
                int start = emptyLoc - (BucketSize-1);
                for(i =  start; i < emptyLoc ; i++) {
                    partNo1 = getPartOfTable(emptyLoc,threshold1);
                    unbaisedLoc1  = emptyLoc - threshold1;
                    info1 = TableAddressList[partNo1][unbaisedLoc1].getHopInfo();	// get info for this bucket
                    while(getNearestNeighbor(i,info1,loc1)){
                        partNo1 = getPartOfTable(emptyLoc,threshold1);
                        unbaisedLoc1  = emptyLoc - threshold1;
                        BDDNode <T,V,L> tempNode = TableAddressList[partNo1][unbaisedLoc1].getNode();
                        hashStringTemp = getHashStringForNewTable(tempNode);	// compute hash String
                        hashCodeTemp = getHashCodeForNewTable(hashString);	// compute hash code

                        if (hashCodeTemp == i){ // if the hashcode for the target bucket equal to the hashcode of the request
                            if ((loc1 < emptyLoc) ) {	// if the bucket isn't reserved, you can swap
                                flag = true;	//rise the flag
                                partNo = getPartOfTable(emptyLoc,threshold);
                                unbaisedLoc  = emptyLoc - threshold;
                                TableAddressList[partNo][unbaisedLoc].setFlags(TableAddressList[partNo1][unbaisedLoc1].getFlags());
                                BDDNode <T,V,L> req = TableAddressList[partNo1][unbaisedLoc1].getNode();
                                TableAddressList[partNo][unbaisedLoc].setNode(req);	// swap the nodes
                                TableAddressList[partNo][unbaisedLoc].setReferenceCounter(TableAddressList[partNo1][unbaisedLoc1].getReferenceCounter());	// swap Refenerence counters
                                TableAddressList[partNo][unbaisedLoc].setLocalId(TableAddressList[partNo1][unbaisedLoc1].getLocalId());
                                unbaisedLoc = unbaisedLoc1;
                                partNo = partNo1;
                                emptyLoc = loc1;			// update the new empty location
                                updateInfoForNewTable (i,loc1,emptyLoc);	// update info
                                break;
                            }
                            else {
                                continue;
                            }
                        }
                        else{
                            mutex.lock();
                            cout << "hash : findOrAddToNewTable : error in swapping nodes" << endl;
                            cout << "i =  " << i << " hashCodeTemp = : " << hashCodeTemp << " loc1 : " << loc1 << endl;
                            TableAddressList[partNo1][unbaisedLoc1].DumpToScreen();
                            mutex.unlock();
                            exit(-1);
                        }
                    }
                    if(flag){
                        break;				// exit "for loop" as we reach the nearest bucket in this round
                    }
                }
                // here we should check the flag, because if it isn't been rised, we have no space and the table will need to be resized and you have to unlock the write lock
                if(flag){
                    continue;				// continue , you have make one swap, so you have to check whether you are in the
                }

                if (( i == start + BucketSize-1 ) && (!flag)) {
                    mutex.lock();
                    cout << "Hash : enable to resize : findOrAddToNewTable" << endl;
                    mutex.unlock();
                    exit(-1);
                }
            }
            if (hashCode == 2036 ){
                mutex.lock();
                cout << "hash : findOrAddToNewTable 2: node with hashCode : " << hashCode << " will be added at location " << emptyLoc << endl;
                node.DumpToScreen();
                mutex.unlock();
            }
            if (emptyLoc == 4096){
                cout << "hash : findOrAddToNewTable 2: node will be added at location " << emptyLoc << endl;
            }

            TableAddressList[partNo][unbaisedLoc].clearAllFlags();			// as this a new request, dated flag must be cleared
            TableAddressList[partNo][unbaisedLoc].setNode(node);		// add your node
            TableAddressList[partNo][unbaisedLoc].setReferenceCounter(refCount);		// add reference count
            TableAddressList[partNo][unbaisedLoc].setLocalId(localId);
            AddNewItemInfoForNewTable (hashCode,emptyLoc);	// update info
        }

    }

    void resize(unsigned char CurrentNoOfResizing){
        // prevent other thread from doing resizing while a thread do resize operation
        T temp ;
        T temp1;
        T i;
        Bucket <T,V,L> tempBucket;
        volatile bool masterThreadFlag = false;

        if(ResizeType == H_RESIZE_TYPE_SWAP){	// if swap resizing is enabled
                do{
                        MasterResizeLock.WriteLock();
                        if(!resizeOnging && (CurrentNoOfResizing != NoOfLookUpTables) ){ 	// if a thread enters the resize after the master thread is out, then this thread should leave
                                MasterResizeLock.Unlock();
                                return;
                        }

                        if (CurrentPhase == H_RESIZE_INITIAL_PHASE){	// if this is the first thread call resize method (master thread)
                                FirstPhaseLock.lock();
                                if(InitialResizePhaseFlag){	 // any slaves will be sleep until the master finishes
                                        MasterResizeLock.Unlock();
                                        FirstPhaseLock.unlock();
                                        continue;
                                }
                                else{	// if this the first time (so you are master)
                                        FirstPhaseLock.unlock();

                                        InitialResizePhaseFlag = true;	// any subsequent threads (slaves) will be sleep
                                        // create look up table
                                        mutex.lock();
                                        cout << "NoOfLookUpTables : " << (T)NoOfLookUpTables << endl;
                                        mutex.unlock();
                                        // update bucket mask
                                        temp = TotalCapacity -( BucketSize -1 );	// remove the marginal locations
                                        temp = temp << 1;
                                        temp1 = temp;
                                        NextBucketMask = temp1 - 1;	// packup the value
                                        temp = temp + BucketSize -1;	// add the marginal locations
                                        NextTotalCapacity = temp; 	// packup the value
                                        delete [] TempTable;

                                        if (temp < TotalCapacity){
                                                mutex.lock();
                                                cout << "can't resize the hash table." << endl;
                                                mutex.unlock();
                                                MasterResizeLock.Unlock();
                                                exit(-1);
                                        }
                                        SegmentSizeForNewTable = SegmentSize; // packup the value

                                        newTable = new Bucket <T,V,L> [temp];
                                        if (newTable == NULL) {
                                                mutex.lock();
                                                cout << "enable to allocate new table: resize : hash" << endl;
                                                mutex.unlock();
                                                MasterResizeLock.Unlock();
                                                exit(-1);
                                        }
                                        // aquire lock on all segments	(to guarantee that all other threads does not adding or fetching some node during resizing). Note that ALL other thread is block in this phase (which is desirable) by the mean of the lock (MasterResizeLock is lock)
                                        resizeOnging = true;
                                        ResizingFlag = true;	// indicate that at least one resizing operation have been done.
                                        mutex.lock();
                                        cout << "resize called" << endl;
                                        mutex.unlock();
                                        //	Dump();
                                        MasterResizeLock.Unlock();

                                        for (i = ConcurrencyDegree-1 ; i >= 0 ; i--) {
                                                Segments[i].WriteLock();
                                                if (i == 0 )
                                                    break;
                                        }

                                        MasterResizeLock.WriteLock();
                                        CurrentPhase = H_RESIZE_MAP_CHILD_POINTERS_PHASE; // go to next stage (note that; only the master thread will enters the initial phase)

                                        FirstPhaseLock.lock();
                                        InitialResizePhaseFlag = false;	// initialize the flag again (for subsequent resizing)
                                //	FirstPhaseCondition->notifyAll();
                                        FirstPhaseLock.unlock();

                                        MasterResizeLock.Unlock();
                                        break;
                                }

                        }
                        else	// if you miss the first phase; unlock and catch other phases
                                MasterResizeLock.Unlock();
                }
                while(true);

                unsigned short locatCurrentSegment;
                vector < T > marginNodeList;
                // Now, we will go through the third phase (ALL thread that enters resize will cooperate in this phase)
                // PHASE THREE: MAP ALL CHILD POINTERS
                do{
                        MasterResizeLock.WriteLock();	// aquire resize lock
                        if(!resizeOnging){ 	// if a thread enters the mapping ID section after the master thread is out of the resize function, then this thread should leave	(THIS is the exit way for all slave in phase three)
                                MasterResizeLock.Unlock();
                                return;
                        }

                        if (CurrentPhase == H_RESIZE_MAP_CHILD_POINTERS_PHASE){	// if this is the first thread call resize method (master thread)
                                locatCurrentSegment = ++CurrentSegment;	// get next segment
                                if (locatCurrentSegment <= ConcurrencyDegree){	// as long as we are in the range of segments
                                        if (locatCurrentSegment < ConcurrencyDegree){	// if you didn't reach the final segment
                                                MasterResizeLock.Unlock();	// unlock
                                        }
                                        // map id here (this section is accessed concurrently as the lock is released)
                                        // map child pointer using lookup table
                                        // mapping will be for the size of the current segment only
                                        T newBucket;
                                        // re-update nodes
                                        BDDNode <T,V,L> node;
                                        unsigned char refCount;
                                        bool datedMarkNodeFlag;
                                        T threshold = TotalCapacity -( BucketSize);	// remove margin
                                        T OldCapacity =  BucketMask +1;

                                        for (i = (locatCurrentSegment-1)*SegmentSize; i < min( locatCurrentSegment*SegmentSize,OldCapacity) ; i++) {
                                                // we will deals with buckets (in this segments of table), which means that; 'i' refer to the bucket not the location of data we map. location of mapped data will be determined by "hopinfo"
                                                //	if (i == 5369)
                                                //		cout << "" << endl;
                                                //	cout << i << endl;
                                                //if (i == 1012)
                                                //	cout << "manager : resize : ya lahwey" << endl;
                                                T hopinfo = Table[i].getHopInfo();	// get its hop information
                                                T nextNeighbor;
                                                while ( getNearestNeighbor(i, hopinfo, nextNeighbor) ){	// get the nearest neighbor
                                                        tempBucket = Table[nextNeighbor];
                                                        if (nextNeighbor == 491){
                                                            cout << "location : " << nextNeighbor << " with hash code : " << i << endl;
                                                        }
                                                        if (i == 491){
                                                            cout << "location : " << nextNeighbor << " with hash code : " << i << endl;
                                                        }
                                                        if (GCEnabled == H_GC_MARK_ENABLED)
                                                                datedMarkNodeFlag = ( tempBucket.getReferenceCounter() != CURRENT_MARK);	// if MARK is updated, then rise flag, else, it will be false
                                                        else if (GCEnabled == H_GC_INCREMENTAL_MARK_ENABLED)
                                                                datedMarkNodeFlag = ( tempBucket.getReferenceCounter() != CURRENT_MARK) && ( tempBucket.getReferenceCounter() != PERMENANT_NODE);	// if MARK is updated or permenant, then rise flag, else, it will be false
                                                        else
                                                                datedMarkNodeFlag = false ;
                                                        // skip empty, dated (if GC mark is enabled), resereved, and swapped node
                                                        if( (!datedMarkNodeFlag) && (!tempBucket.isEmpty())){
                                                                BDDNode <T,V,L> TempNode = tempBucket.getNode();
                                                                T hashString = getHashStringForNewTable(TempNode);
                                                                if (hashString == 1866059243)
                                                                    cout << " hash string : " << hashString << " with hash code : " << i << endl;
                                                                newBucket =  getHashCodeForNewTable(hashString); // rehash the node
                                                                T newLoc;	// new location of the node
                                                                if(i == newBucket)	// if the new bucket is different from old one
                                                                        newLoc = nextNeighbor;			// the new location is the same as old one
                                                                else if( (i | OldCapacity) == newBucket )
                                                                        newLoc = nextNeighbor | OldCapacity;	// shift the location of the node by the size of the old table
                                                                else{	// should never happens
                                                                        mutex.lock();
                                                                        cout << "hash: resize: error in mapping node while resizing 2" << endl;
                                                                        cout << "mapping at location : " << i << endl;
                                                              //          cout << "node with id : " << Then.getPointer() << " and old bucket : " << Then.getBucket() << " will be hashed to new bucket : " <<  newBucket << endl;
                                                                        mutex.unlock();
                                                                        exit(-1);
                                                                }

                                                                // add node to the new table
                                                                node = tempBucket.getNode();
                                                                refCount = tempBucket.getReferenceCounter();
                                                                V localid = tempBucket.getLocalId();
                                                                if(nextNeighbor > threshold){	// if you are at the marginal area
                                                                        if(nextNeighbor == newLoc){	// if the map will be in the same location (so, we will interacte with the first segment)
                                                                                // schedule these node for addition after "for" loop
                                                                                marginNodeList.push_back(nextNeighbor);	// store exact location of the node
                                                                        /*	Table[nextNeighbor].setNode(node);
                                                                                Table[nextNeighbor].setReferenceCounter(refCount);
                                                                                Table[nextNeighbor].setLocalId(localid);*/
                                                                        }
                                                                        else{	// if we mapp to a different location, so we will not interact with the first segment
                                                                                findOrAddToNewTable(node, refCount,newLoc,localid);

                                                                        }

                                                                }
                                                                else {	// if this not the marginal area
                                                                        findOrAddToNewTable(node, refCount,newLoc,localid);
                                                                }	// if statment
                                                        }	// if statment
                                                }	// while loop
                                        }	// for loop
                                //	cout << " i = " << i << endl;


                                        // you should NOT add marginal node UNLESS the first segment have been finished. the following code guarantee that the marginal nodes will added after the first segment finished.
                                        if (locatCurrentSegment == 1){	// if this the first segment	(the first segment finished its task)
                                                FirstSegmentLock.lock();
                                                FirstSegmentFinishedMap = true;
                                                ResizingCondition->notifyAll(); // wake up the last thread if it is sleep
                                                FirstSegmentLock.unlock();
                                        }
                                        else if (locatCurrentSegment == ConcurrencyDegree){	// if this the last segment
                                                do{
                                                        FirstSegmentLock.lock();
                                                        if(FirstSegmentFinishedMap){	// if the first segment have finished
                                                                FirstSegmentLock.unlock();
                                                                // insert marginal node (this is done after first segments finished)
                                                                for (i = 0; i < (T)marginNodeList.size(); i++){
                                                                        T loc = marginNodeList[i];
                                                                        node = Table[loc].getNode();
                                                                        refCount = Table[loc].getReferenceCounter();
                                                                        V localid = Table[loc].getLocalId();
                                                                        findOrAddToNewTable(node, refCount, localid );	// note that id, 'then' and 'else' pointers are updated in table (they doesn't only stored in the 'newTable' in the previous 'for' loop)
                                                                }
                                                                break;
                                                        }
                                                        else {	// if the first segment have not been finished, then waits
                                                                ResizingCondition->wait(); 	// wait until the first thread finished
                                                                FirstSegmentFinishedMap = false;
                                                                FirstSegmentLock.unlock();
                                                        }
                                                }
                                                while(true);
                                        }


                                        // As the table swapping (and other things) should be done after ALL segments have finished its work; So, we count how many segments have been finished.

                                        ThirdPhaseLock.lock();	// aquire resize lock
                                        int localFinishedCurrentSegment = ++FinishedCurrentSegment;

                                        if (localFinishedCurrentSegment == ConcurrencyDegree){	// if all other threads finished all segments
                                                FinishedCurrentSegment = 0;
                                                CurrentPhase = H_RESIZE_FINISHING_PHASE;	// go to next stage (final stage)
                                                ThirdPhaseCondition->notifyAll();
                                                ThirdPhaseLock.unlock();	// unlock
                                        }
                                        else{
                                                if (locatCurrentSegment == ConcurrencyDegree)	// if you reach the final segment, release it (remeber that you release all
                                                        ThirdPhaseCondition->wait();
                                                ThirdPhaseLock.unlock();	// unlock
                                        }

                                        if (locatCurrentSegment == ConcurrencyDegree){	// if you didn't reach the final segment
                                                MasterResizeLock.Unlock();	// unlock
                                        }

                                        // PHASE FOUR : DO SWAP AND OTHER STAFF (FINISHING)
                                        MasterResizeLock.WriteLock();	// aquire resize lock
                                        if(!resizeOnging){ 	// if a thread enters the mapping ID section after the master thread is out of the resize function, then this thread should leave	(THIS is the exit way for all slave in phase three)
                                                MasterResizeLock.Unlock();
                                                return;
                                        }

                                        if (CurrentPhase == H_RESIZE_FINISHING_PHASE){	// if this is the first thread call resize method (master thread)
                                                FourthPhaseLock.lock();
                                                if(FinalResizePhaseFlag){
                                                        FourthPhaseCondition->wait();
                                                        FourthPhaseLock.unlock();
                                                        MasterResizeLock.Unlock();
                                                }
                                                else{
                                                        // We want to swap tables and finish the resizing operation
                                                        if (locatCurrentSegment == ConcurrencyDegree){	// if you reach the final segment, release it (remeber that you release all other threads (slaves) EXCEPT the final one (master))
                                                                FinalResizePhaseFlag = true ;
                                                                FourthPhaseLock.unlock();

                                                                CurrentPhase = H_RESIZE_INITIAL_PHASE; // go to next stage (note that; only the master thread will enters this part, and ALL slaves threads will waiting and will not go to the next section automatically (as the master acquire the lock and will not release it until it finishes all neccessary tasks in this phase)
                                                                CurrentSegment = 0;

                                                                // delete old table, and reassign teh pointer to point to new table
                                                                TempTable = Table;
                                                                Table = newTable;

                                                                NoOfLookUpTables++;
                                                                ResizingOccured = true;
                                                                resizeOnging = false;
                                                                TotalCapacity = temp;
                                                                this->SegmentSize = floor((float)(TotalCapacity/ConcurrencyDegree)) + 1;
                                                                this->BucketMask = NextBucketMask;
                                                        //	Dump();
                                                                mutex.lock();
                                                                cout << "No. of resizing operations " << (unsigned short) NoOfLookUpTables << endl;
                                                                cout << "total capacity: " << TotalCapacity << endl;

                                                                cout << "resize ended" << endl;
                                                                mutex.unlock();
                                                                MasterResizeLock.Unlock();	// unlock (now other remaining thread can aquire the lock and hence, it would be able to exit this phase)
                                                                // release all segments
                                                                for (i = 0 ; i < ConcurrencyDegree; i++) {
                                                                        Segments[i].Unlock();
                                                                }

                                                                FourthPhaseLock.lock();
                                                                FinalResizePhaseFlag = false ;
                                                                FourthPhaseCondition->notifyAll();
                                                                FourthPhaseLock.unlock();

                                                                break;	// master exit this phase
                                                        }
                                                }
                                        }
                                        else{

                                                MasterResizeLock.Unlock();


                                        }
                                }
                                else{	// if any thread come after the MASTER (the one who mapped the last segment), then this is an error in the logic of the function, and should never happens
                                        mutex.lock();
                                        cout << "hash : resize : error invalid section 2" << endl;
                                        mutex.unlock();
                                        MasterResizeLock.Unlock();
                                        exit(-1);
                                }
                        }
                        else{	// if the phase is changed, then go to out
                            // this is should never execute, because all slave threads will exit from the beginning of the while loop
                      /*      mutex.lock();
                            cout << "hash : resize : error invalid section 3" << endl;
                            mutex.unlock();
                            exit(-1);*/
                            MasterResizeLock.Unlock();
                            break;	// slaves exit this phase
                        }
                }
                while(true);
        }
        else {	// if incremental resizing is enabled
            MasterResizeLock.WriteLock();
            if(!resizeOnging && (CurrentNoOfResizing != NoOfLookUpTables) ){ 	// if a thread enters the resize after the master thread is out, then this thread should leave
                MasterResizeLock.Unlock();
                return;
            }

            if (CurrentPhase == H_RESIZE_INITIAL_PHASE){	// if this is the first thread call resize method (master thread)
                FirstPhaseLock.lock();
                if(InitialResizePhaseFlag){	 // any slaves will be sleep until the master finishes
                    MasterResizeLock.Unlock();
                    FirstPhaseCondition->wait();
                    FirstPhaseLock.unlock();
                }
                else{	// if this the first time (so you are master)
                    FirstPhaseLock.unlock();

                    InitialResizePhaseFlag = true;	// any subsequent threads (slaves) will be sleep
                    // create look up table
                /*    mutex.lock();
                    cout << "hash : resize : you are in the first phase" << endl;
                    cout << "NoOfLookUpTables : " << (T)NoOfLookUpTables << endl;
                    mutex.unlock();*/
                    // update bucket mask
                    temp = TotalCapacity -( BucketSize -1 );	// remove the marginal locations
                    temp = temp << 1;
                    temp1 = temp;
                    NextBucketMask = temp1 - 1;	// packup the value
                    temp = temp + BucketSize -1;	// add the marginal locations
                    NextTotalCapacity = temp; 	// packup the value

                    if (temp < TotalCapacity){
                        mutex.lock();
                        cout << "can't resize the hash table." << endl;
                        mutex.unlock();
                        MasterResizeLock.Unlock();
                        exit(-1);
                    }
                    SegmentSizeForNewTable = SegmentSize; // packup the value

                    resizeOnging = true;
                    ResizingFlag = true;	// indicate that at least one resizing operation have been done.
                    mutex.lock();
                    cout << "resize called" << endl;
                    mutex.unlock();
                    //	Dump();
                    MasterResizeLock.Unlock();
                    // aquire lock on all segments	(to guarantee that all other threads does not adding or fetching some node during resizing). as in order to add you have to aquire atleast two locks (while we resize we aquire all locks, so we prevent any thread from adding a new table). Actually, when another thread detects that the resize is onging (by reading ongoing flag), it come to help us in resizing.

                    for (i = ConcurrencyDegree-1 ; i >= 0 ; i--) {
                        Segments[i].WriteLock();
                        if (i == 0 )
                            break;
                    }
                    masterThreadFlag = true;
            //        cout << "hash : resize : phase 1. masterThreadFlag = " << masterThreadFlag << endl;
                    MasterResizeLock.WriteLock();
                    CurrentPhase = H_RESIZE_MAP_CHILD_POINTERS_PHASE; // go to next stage (note that; only the master thread will enters the initial phase)
                    TotalNoOfTableParts++;			// increment no. of table parts
                    TableAddressList[(unsigned short)(TotalNoOfTableParts)] = TableAddressList[(unsigned short)(TotalNoOfTableParts - 1)];	// shift the marginal nodes
                    TableAddressList[(unsigned short)(TotalNoOfTableParts - 1)] = new Bucket<T,V,L> [TotalCapacity -( BucketSize -1 )];	// create new part
                    if (TableAddressList[(unsigned short)TotalNoOfTableParts] == NULL) {
                        mutex.lock();
                        cout << "enable to allocate new table: resize : hash" << endl;
                        mutex.unlock();
                        MasterResizeLock.Unlock();
                        exit(-1);
                    }


                    FirstPhaseLock.lock();
                    InitialResizePhaseFlag = false;	// initialize the flag again (for subsequent resizing)
                    FirstPhaseCondition->notifyAll();
                    FirstPhaseLock.unlock();

                    MasterResizeLock.Unlock();
                }
            }
            else	// if you miss the first phase; unlock and catch other phases
                MasterResizeLock.Unlock();

            unsigned short locatCurrentSegment;
            unsigned char partNo;
            T unbaisedLoc;
            T threshold;	// remove margin
      /*      mutex.lock();
            cout << "hash : resize : I'm here, phase 2.  masterThreadFlag = " << masterThreadFlag  << endl;
            mutex.unlock();*/
            vector < T > marginNotShiftedNodeLocationList;
            vector < T > marginNotShiftedNodeBucketList;
            vector < T > marginBucketList;
            vector < T > marginLocationList;

            // Now, we will go through the third phase (ALL thread that enters resize will cooperate in this phase)
            // PHASE THREE: MAP ALL CHILD POINTERS
            do{
                MasterResizeLock.WriteLock();	// aquire resize lock
                if(!resizeOnging){ 	// if a thread enters the mapping ID section after the master thread is out of the resize function, then this thread should leave	(THIS is the exit way for all slave in phase three)
                    MasterResizeLock.Unlock();

                 /*   mutex.lock();
                    cout << "hash : resize :phase 2. masterThreadFlag = " << masterThreadFlag << endl;
                    cout << "thread will exit" << endl;
                    mutex.unlock();*/
                    // allow the master thread to unlock all segments (as pthread restrict that the thread whcih lock the mutex MUST the one which unlock it)
                    if (masterThreadFlag){
                        // release all segments
                    /*    mutex.lock();
                        cout << "hash resize : unlocking segments " << endl;
                        mutex.unlock();*/
                        for (i = 0 ; i < ConcurrencyDegree; i++) {
                            Segments[i].Unlock();
                        }
                    }

                    return;
                }
                if (CurrentPhase == H_RESIZE_MAP_CHILD_POINTERS_PHASE){	// if this is the first thread call resize method (master thread)
                    locatCurrentSegment = ++CurrentSegment;	// get next segment
                    if (locatCurrentSegment <= ConcurrencyDegree){	// as long as we are in the range of segments
                        if (locatCurrentSegment < ConcurrencyDegree){	// if you didn't reach the final segment
                            MasterResizeLock.Unlock();	// unlock
                        }

                        T newBucket;
                        BDDNode <T,V,L> node;
                        unsigned char refCount;
                        bool datedMarkNodeFlag;

                        for (i = (locatCurrentSegment-1)*SegmentSize; i < min( locatCurrentSegment*SegmentSize,BucketMask+1) ; i++) {
                            partNo = getPartOfTable(i,threshold);
                         /*   if (i == 508){
                                mutex.lock();
                                cout << "hash:resize: ya lahwy" << endl;
                                mutex.unlock();
                            }*/
                            unbaisedLoc  = i - threshold;
                            // we will deals with buckets (in this segments of table), which means that; 'i' refer to the bucket not the location of data we map. location of mapped data will be determined by "hopinfo"
                            T hopinfo = TableAddressList[partNo][unbaisedLoc].getHopInfo();	// get its hop information
                            T nextNeighbor;
                            while ( getNearestNeighbor(i, hopinfo, nextNeighbor) ){	// get the nearest neighbor
                                partNo = getPartOfTable(nextNeighbor,threshold);
                                if(nextNeighbor > BucketMask){	// if you are at the marginal area, we just want to access the next part (as we move the marginal part by 1)
                                    partNo++;
                                }

                                unbaisedLoc  = nextNeighbor - threshold;
                                if (GCEnabled == H_GC_MARK_ENABLED)
                                    datedMarkNodeFlag = ( TableAddressList[partNo][unbaisedLoc].getReferenceCounter() != CURRENT_MARK);	// if MARK is updated, then rise flag, else, it will be false
                                else if (GCEnabled == H_GC_INCREMENTAL_MARK_ENABLED)
                                    datedMarkNodeFlag = ( TableAddressList[partNo][unbaisedLoc].getReferenceCounter() != CURRENT_MARK) && ( TableAddressList[partNo][unbaisedLoc].getReferenceCounter() != PERMENANT_NODE);	// if MARK is updated or permenant, then rise flag, else, it will be false
                                else
                                    datedMarkNodeFlag = false ;
                                // skip empty, dated (if GC mark is enabled), resereved, and swapped node
                                if( (!datedMarkNodeFlag) && (!TableAddressList[partNo][unbaisedLoc].isEmpty()) ){
                                    T newLoc;	// new location of the node
                                    BDDNode <T,V,L> TempNode = TableAddressList[partNo][unbaisedLoc].getNode();
                                    newBucket =  getHashCodeForNewTable(getHashStringForNewTable(TempNode)); // rehash the node
                                    if(i == newBucket)	// if the new bucket is different from old one
                                        newLoc = nextNeighbor;			// the new location is the same as old one
                                    else if( (i + BucketMask +1) == newBucket )
                                        newLoc = nextNeighbor + BucketMask +1;	// shift the location of the node by the size of the old table
                                    else{	// shnewBucketould never happens
                                        mutex.lock();
                                        cout << "hash: resize: error in mapping node while resizing 2" << endl;
                                        cout << "old bucket : " << i << " new bucket : "<< newBucket <<  endl;
                                        cout << "old location : " << nextNeighbor << " . new location : " << newLoc << endl;
                                        newLoc = 0;
                                        mutex.unlock();
                                        exit(-1);
                                    }
                                    if (nextNeighbor == 142){
                                        mutex.lock();
                                        cout << "hash : resize : item at location " << nextNeighbor << " and hash code " << i << " will be hash to " << newLoc << endl;
                                        mutex.unlock();
                                    }
                                    // add node to the new table
                                    node = TableAddressList[partNo][unbaisedLoc].getNode();
                                    refCount = TableAddressList[partNo][unbaisedLoc].getReferenceCounter();
                                    V localid = TableAddressList[partNo][unbaisedLoc].getLocalId();
                                    if(nextNeighbor > BucketMask){	// if you are at the marginal area
                                        if(nextNeighbor == newLoc){	// if the map will be in the same location (so, we will interacte with the first segment)
                                            // schedule these node for addition after "for" loop
                                            marginNotShiftedNodeLocationList.push_back(nextNeighbor);	// store non shifted location
                                            marginNotShiftedNodeBucketList.push_back(i);	// store non shifted bucket
                                        }
                                        else{	// if we mapp to a different location (note that the the marginal nodes are shifted to the end of the table so we don't have to move it (as it is already moved); however, the hopinfos should be updated), so we will not interact with the first segment
                                            AddNewItemInfoForNewTable(i+BucketMask+1,newLoc);
                                            marginBucketList.push_back(i);	// store new buckets (with out shifting)
                                            marginLocationList.push_back(nextNeighbor);	// store the old location
                                        }

                                    }
                                    else {	// if this not the marginal area
                                        if (newLoc != nextNeighbor){	// if the node will be moved
                                            findOrAddToNewTable(node, refCount,newLoc,localid);	// add it to new location
                                            TableAddressList[partNo][unbaisedLoc].clearAllFlags();	// clear flags of the old location
                                            removeItemInfo(i,nextNeighbor);	// remove the hopinfo for the old location
                                            TableAddressList[partNo][unbaisedLoc].setReferenceCounter(0);
                                        }
                                    }	// if statment
                                }	// if statment
                            }	// while loop
                        }	// for loop
                  /*      mutex.lock();
                        cout << " i = " << i << endl;
                        mutex.unlock();*/


                        // you should NOT add marginal node UNLESS the first segment have been finished. the following code guarantee that the marginal nodes will added after the first segment finished.
                        if (locatCurrentSegment == 1){	// if this the first segment	(the first segment finished its task)
                            FirstSegmentLock.lock();
                            FirstSegmentFinishedMap = true;
                            ResizingCondition->notifyAll(); // wake up the last thread if it is sleep
                            FirstSegmentLock.unlock();
                        }
                        if (locatCurrentSegment == ConcurrencyDegree){	// if this the last segment
                            FirstSegmentLock.lock();
                            if(FirstSegmentFinishedMap){	// if the first segment have finished
                                FirstSegmentLock.unlock();
                                // Shifted nodes
                                for (i = 0; i < marginBucketList.size(); i++){
                                    int loc = marginLocationList[i];		// get location
                                    partNo = getPartOfTable(loc,threshold);
                                    unbaisedLoc  = loc - threshold;
                                    int bucket = marginBucketList[i];
                                    if (TableAddressList[partNo][unbaisedLoc].isEmpty()){	// if the old location is still empty, then remove hop infos (as the node is shifted and no other node stored in this location)
                                        removeItemInfo(bucket,loc);	// remove the hopinfo for the old bucket
                                    }
                                    else{
                                        if((int)TableAddressList[partNo][unbaisedLoc].getLocalId() != bucket ){	// if the stored node doesn't hash to the same old bucket
                                            removeItemInfo(bucket,loc);	// remove the hopinfo for the old bucket
                                        }
                                    }
                                }
                                // non shifted nodes
                                // insert marginal node (this is done after first segments finished)
                                for (i = 0; i < marginNotShiftedNodeLocationList.size(); i++){
                                    T loc = marginNotShiftedNodeLocationList[i];
                                    partNo = getPartOfTable(loc,threshold);
                                    unbaisedLoc  = loc - threshold;
                                    if (TableAddressList[partNo][unbaisedLoc].isEmpty()){	// if the location is still empty, then store the node (the hop info will be the same)
                                        node = TableAddressList[partNo+1][unbaisedLoc].getNode();
                                        refCount = TableAddressList[partNo+1][unbaisedLoc].getReferenceCounter();
                                        TableAddressList[partNo][unbaisedLoc].setNode(node);	// copy the node from maginal area to this location (with flags and RC)
                                        TableAddressList[partNo][unbaisedLoc].setReferenceCounter(refCount);	// copy the node from maginal area to this location (with flags and RC)
                                        TableAddressList[partNo][unbaisedLoc].setLocalId(TableAddressList[partNo+1][unbaisedLoc].getLocalId());
                                    }
                                    else{
                                        T bucket = marginNotShiftedNodeBucketList[i];
                                        BDDNode <T,V,L> tempNode = TableAddressList[partNo][unbaisedLoc].getNode();
                                        T oldHashCode = getHashCode(getHashString(tempNode ));
                                        if((T)oldHashCode != bucket){	// if the storedashString(TableAddressList[partNo][unbaisedLoc])); node doesn't hash to the same old bucket
                                            removeItemInfo(bucket,loc);	// remove the hopinfo for the old bucket
                                        }
                                        // store the node to the same location , but in a different part
                                        node = TableAddressList[partNo+1][unbaisedLoc].getNode();
                                        refCount = TableAddressList[partNo+1][unbaisedLoc].getReferenceCounter();
                                        findOrAddToNewTable(node, refCount, TableAddressList[partNo+1][unbaisedLoc].getLocalId());	// note that id, 'then' and 'else' pointers are updated in table (they doesn't only stored in the 'newTable' in the previous 'for' loop)
                                    }

                                    // remove the node from the marginal area
                                    TableAddressList[partNo+1][unbaisedLoc].clearAllFlags();	// clear flags of the old location
                                    TableAddressList[partNo+1][unbaisedLoc].setReferenceCounter(0);

                                }
                            }
                            else {	// if the first segment have not been finished, then waits
                                ResizingCondition->wait(); 	// wait until the first thread finished
                                FirstSegmentFinishedMap = false;
                                FirstSegmentLock.unlock();
                            }
                        }

                        // As the table swapping (and other things) should be done after ALL segments have finished its work; So, we count how many segments have been finished.
                        ThirdPhaseLock.lock();	// aquire resize lock
                        int localFinishedCurrentSegment = ++FinishedCurrentSegment;

                        if (localFinishedCurrentSegment == ConcurrencyDegree){	// if all other threads finished all segments
                            FinishedCurrentSegment = 0;
                            CurrentPhase = H_RESIZE_FINISHING_PHASE;	// go to next stage (final stage)
                            ThirdPhaseCondition->notifyAll();
                            ThirdPhaseLock.unlock();	// unlock
                        }
                        else{
                            if (locatCurrentSegment == ConcurrencyDegree)	// if you reach the final segment, release it (remeber that you release all
                                ThirdPhaseCondition->wait();
                            ThirdPhaseLock.unlock();	// unlock
                        }

                        if (locatCurrentSegment == ConcurrencyDegree){	// if you didn't reach the final segment
                            MasterResizeLock.Unlock();	// unlock
                        }
                    }
                    else{	// if any thread come after the MASTER (the one who mapped the last segment), then this is an error in the logic of the function, and should never happens
                        mutex.lock();
                        cout << "hash : resize : error invalid section 2" << endl;
                        mutex.unlock();
                        MasterResizeLock.Unlock();
                        exit(-1);
                    }
                }
                else{	// if the phase is changed, then go to out
                    // this is should never execute, because all slave threads will exit from the beginning of the while loop
                 /*   mutex.lock();
                    cout << "hash : resize : error invalid section 3" << endl;
                    cout << "current phase : " << (T) CurrentPhase << endl;
                    mutex.unlock();
                    exit(-1);*/
                    MasterResizeLock.Unlock();
                    break;	// slaves exit this phase
                }
            }
            while(true);

     /*       mutex.lock();
            cout << "hash : resize : I'm here, phase 3.  masterThreadFlag = " << masterThreadFlag  << endl;
            mutex.unlock();*/

            // PHASE FOUR : DO SWAP AND OTHER STAFF (FINISHING)
            MasterResizeLock.WriteLock();	// aquire resize lock
            if(!resizeOnging){ 	// if a thread enters the mapping ID section after the master thread is out of the resize function, then this thread should leave	(THIS is the exit way for all slave in phase three)
                MasterResizeLock.Unlock();

             /*   mutex.lock();
                cout << "hash : resize :phase 3. masterThreadFlag = " << masterThreadFlag << endl;
                cout << "thread will exit" << endl;
                mutex.unlock();*/
                // allow the master thread to unlock all segments (as pthread restrict that the thread whcih lock the mutex MUST the one which unlock it)
                if (masterThreadFlag){
                    // release all segments
                /*    mutex.lock();
                    cout << "hash resize : unlocking segments " << endl;
                    mutex.unlock();*/
                    for (i = 0 ; i < ConcurrencyDegree; i++) {
                        Segments[i].Unlock();
                    }
                }

                return;
            }

            if (CurrentPhase == H_RESIZE_FINISHING_PHASE){	// if this is the first thread call resize method (master thread)
                FourthPhaseLock.lock();
                if(FinalResizePhaseFlag){
                    MasterResizeLock.Unlock();
                    FourthPhaseCondition->wait();
                    FourthPhaseLock.unlock();
                }
                else{
                    // We want to swap tables and finish the resizing operation
                    if (locatCurrentSegment == ConcurrencyDegree){	// if you reach the final segment, release it (remeber that you release all other threads (slaves) EXCEPT the final one (master))
                        FinalResizePhaseFlag = true ;
                        FourthPhaseLock.unlock();

                        CurrentPhase = H_RESIZE_INITIAL_PHASE; // go to next stage (note that; only the master thread will enters this part, and ALL slaves threads will waiting and will not go to the next section automatically (as the master acquire the lock and will not release it until it finishes all neccessary tasks in this phase)
                        CurrentSegment = 0;

                        NoOfLookUpTables++;
                        ResizingOccured = true;
                        resizeOnging = false;
                        temp = TotalCapacity -( BucketSize -1 );	// remove the marginal locations
                        temp = temp << 1;
                        temp = temp + BucketSize -1;	// add the marginal locations
                        TotalCapacity = temp;
                        this->SegmentSize = floor((float)(TotalCapacity/ConcurrencyDegree)) + 1;
                        this->BucketMask = NextBucketMask;
                        // do swap for the last two parts
                //	Dump();
                        mutex.lock();
                        cout << "No. of resizing operations " << (unsigned short) NoOfLookUpTables << endl;
                        cout << "total capacity: " << TotalCapacity << endl;
                        cout << "resize ended" << endl;
                        mutex.unlock();
                        MasterResizeLock.Unlock();	// unlock (now other remaining thread can aquire the lock and hence, it would be able to exit this phase)
 
                        FourthPhaseLock.lock();
                        FinalResizePhaseFlag = false ;
                        FourthPhaseCondition->notifyAll();
                        FourthPhaseLock.unlock();
                    }
                    else{
                        MasterResizeLock.Unlock();
                        FourthPhaseCondition->wait();
                        FourthPhaseLock.unlock();
                   }
                }
            }
            else{
                MasterResizeLock.Unlock();
            }
            
       /*     mutex.lock();
            cout << "hash : resize :phase 4. masterThreadFlag = " << masterThreadFlag << endl;
            mutex.unlock();*/
            // allow the master thread to unlock all segments (as pthread restrict that the thread whcih lock the mutex MUST the one which unlock it)
            if (masterThreadFlag){
                // release all segments
          /*      mutex.lock();
                cout << "hash resize : unlocking segments " << endl;
                mutex.unlock();*/
                for (i = 0 ; i < ConcurrencyDegree; i++) {
                    Segments[i].Unlock();
                }
            }
        }
    }
    // get resizing flag and resize couter atomically without UNLOCKING the resize lock
    void getResizingFlagAndCounterWithoutUnlocking(bool & flag, unsigned char & CurrentNoOfResizing){
            MasterResizeLock.ReadLock();
            flag = resizeOnging;
            CurrentNoOfResizing = NoOfLookUpTables;

    }
    void getResizingFlagAndCounter(bool & flag, unsigned char & CurrentNoOfResizing){
            flag = resizeOnging;
            CurrentNoOfResizing = NoOfLookUpTables;

    }

    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // Functions related to incremental resizing
    //------------------------------------------------------------------------------------------------------------------------------------------------------------

    int getPartNo(int LOverX){
            int partNo = 0;
            while(LOverX > 0){
                    partNo++;
                    LOverX >>=1;
            }
            return partNo;
    }

    // select the appropraite table
    unsigned char getPartOfTable(T location , T &threshold){
        threshold = location >> initialCapacityPower;	// divide the location by 'initial total capacity' (L/x)
        unsigned char partNo = getPartNo(threshold);		// partno. = get no. of bits needed to represent (L/x) -1
        threshold =  (1 << (initialCapacityPower) ) * (1 << (partNo-1) ); // threshold = x * 2^partNo
        return partNo;
    }
    // it takes 'location' of the node and returns 'part no' (determine which part of the table this location belongs to ) and the 'threshold (the start of this part)
    unsigned char getPartOfTable(T location , T &threshold, T & upperLimit, bool & lastPartFlag){
        threshold = location >> initialCapacityPower;	// divide the location by 'initial total capacity' (L/x)
        unsigned char partNo = getPartNo(threshold);		// partno. = get no. of bits needed to represent (L/x) -1

        threshold =  (1 << (initialCapacityPower) ) * (1 << (partNo-1) ); // threshold = x * 2^partNo
        if (partNo == 0){
            upperLimit = 1 << initialCapacityPower; // get the initial capacity
            lastPartFlag = false;
            // note that you will return with no acquired lock at all.
        }
        else {
            MasterResizeLock.ReadLock();
            if (partNo == TotalNoOfTableParts){ // if this is the last part
                upperLimit = TotalCapacity + 1 ;
                lastPartFlag = true;
                // return without unlock
            }
            else{
                upperLimit = threshold << 1;        // multiply threshold by 2
                lastPartFlag = false;
                MasterResizeLock.Unlock();
                // unlock before exit
            }
            // note you will return withour unlocking the master resize lock
        }
        return partNo;
    }
    // it takes 'location' of the node and returns 'part no' (determine which part of the table this location belongs to ) and the 'threshold (the start of this part)
    unsigned char getPartOfTable(T location , T &threshold, T & upperLimit){
        threshold = location >> initialCapacityPower;	// divide the location by 'initial total capacity' (L/x)
        unsigned char partNo = getPartNo(threshold);		// partno. = get no. of bits needed to represent (L/x) -1

        threshold =  (1 << (initialCapacityPower) ) * (1 << (partNo-1) ); // threshold = x * 2^partNo
        if (partNo == 0){
            upperLimit = 1 << initialCapacityPower; // get the initial capacity
            // note that you will return with no acquired lock at all.
        }
        else {
            if (partNo == TotalNoOfTableParts){ // if this is the last part
                upperLimit = TotalCapacity + 1 ;
            }
            else{
                upperLimit = threshold << 1;        // multiply threshold by 2
            }
            // note you will return withour unlocking the master resize lock
        }
        return partNo;
    }
    // it takes 'location' of the node and returns 'part no' (determine which part of the table this location belongs to ) and the 'threshold (the start of this part)
    unsigned char getPartOfTableWithoutLocking(T location , T &threshold, T & upperLimit, bool & lastPartFlag){
        threshold = location >> initialCapacityPower;	// divide the location by 'initial total capacity' (L/x)
        unsigned char partNo = getPartNo(threshold);		// partno. = get no. of bits needed to represent (L/x) -1

        threshold =  (1 << (initialCapacityPower) ) * (1 << (partNo-1) ); // threshold = x * 2^partNo
        upperLimit = TotalCapacity + 1 ;
        lastPartFlag = true;
        return partNo;
    }

    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // node addation Related Functions	(find nearest neighbor and contain without unlocking)
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // get the nearest neigbor for certain bucket based on its hop info (hop info is modified each time this function called )
    bool getNearestNeighbor(T hashcode, T &hopinfo, T &location){
            if(hopinfo == 0){	// if hop information is zero return false
                    return false;
            }
            // if it does contain data, find the nearest neighbor
            T x = BucketSize >> 1;  // divide by zero
            T highHalf,lowHalf;
            T hifo =hopinfo;
            T delta = hashcode + BucketSize -1;
            do{
                    highHalf = (hifo >> x ) & ((T)( ( 1 << (x+1) ) - 1));
                    lowHalf = hifo & ((T)( ( 1 << (x+1) ) - 1));
                    if (highHalf != 0){
                            delta-= x;
                            hifo = highHalf;
                    }
                    else if (lowHalf == 0 ){
                            break;
                    }
                    else
                            hifo = lowHalf;
                    x = x >> 1;	// divide by 2
            }
            while(x != 0);

            hopinfo = hopinfo ^ (1 << ( BucketSize - (delta - hashcode) - 1 ));	// update hopinformation
            location = delta;
            return true;
    }
    // get the index of the nearest location to "start", if you reach the end of the table return false , you have to unlock the segment, if resizing is onging , then unlock all segments and returns
    // start: input; the start location to search
    // location: output; store the location of the empty bucket
    // lock: input; determine the type of the lock
    // Seg: output; return the segment of that location.

    bool findNearestEmptyLocation(T start, T & location, unsigned short & Seg, bool & resizeFlag, unsigned char & PreviousNoOfResizing ){
        unsigned char CurrentNoOfResizing;
        bool datedMarkNodeFlag;	// if this flag is 'false' then nothing is affected with it.

        if (GCEnabled == H_GC_MARK_ENABLED)
            datedMarkNodeFlag = ( Table[start].getReferenceCounter() != CURRENT_MARK);	// if MARK is updated, then rise flag, else, it will be false
        else if (GCEnabled == H_GC_INCREMENTAL_MARK_ENABLED)
            datedMarkNodeFlag = ( Table[start].getReferenceCounter() != CURRENT_MARK) && ( Table[start].getReferenceCounter() != PERMENANT_NODE);	// if MARK is updated or permenant, then rise flag, else, it will be false
        else
            datedMarkNodeFlag = false ;
        do{ // repeat until you find an empty location (After locking)
            while( !( (Table[start].isEmpty() || datedMarkNodeFlag) ) ){
                start++;

                if (GCEnabled == H_GC_MARK_ENABLED)
                    datedMarkNodeFlag = ( Table[start].getReferenceCounter() != CURRENT_MARK);	// if the RC is equal to zero, then rise flag
                else if (GCEnabled == H_GC_INCREMENTAL_MARK_ENABLED)
                    datedMarkNodeFlag = ( Table[start].getReferenceCounter() != CURRENT_MARK) && ( Table[start].getReferenceCounter() != PERMENANT_NODE);	// if MARK is updated or permenant, then rise flag, else, it will be false
                else
                    datedMarkNodeFlag = false;	// if the RC is equal to zero, then rise flag
            }

            // try to acquire the segment and check for the resizing conditions
            getResizingFlagAndCounterWithoutUnlocking(resizeFlag,CurrentNoOfResizing); // check whether the resizing is going on
            if(resizeFlag || (PreviousNoOfResizing != CurrentNoOfResizing) ){	// if resize is onging
                    PreviousNoOfResizing = CurrentNoOfResizing;
                    return true;	// skip this round, and restart adding
            }
            Seg = getSegment(start);
            MasterResizeLock.Unlock();	// unlock the resizing lock
            // check if this location is still empty or dated
            Segments[Seg].WriteLock();		// aquire write lock in this segment
            if (Table[start].isEmpty()){ // if the location is empty or dated
                break;      // get out from searching
            }
            else
                Segments[Seg].Unlock(); // unlock as we have another round

            if (start == TotalCapacity) {	//if you reached the last place in the hash, return a number bigger than the capacity of the table
                return false;
            }
        }
        while(true);

        if ( (GCEnabled == H_GC_MARK_ENABLED) || (GCEnabled == H_GC_INCREMENTAL_MARK_ENABLED)){	// if GC is enabled, then check if the location is choosen due it is a dead node or not (dead node is the node with RF != CURRENT_MARK)
            if ( (!Table[start].isEmpty()) && datedMarkNodeFlag) { // if it is a stored node (not empty), with dated mark pointer to it, and we are have a 'write' lock on it, then we have a dead node that MUST BE removed
                // update hopinfo for the bucket store this node
                BDDNode <T,V,L> node1 = Table[start].getNode();
                T hashCode = getHashCode(getHashString(node1));	// get bucket
                unsigned short seg1;		// segment of the current bucket
                // we have the possiblity that hashCode and start are in a different segments , so we will have to lock them in order, as well as take care of the possibility that resizing may be onging

                getResizingFlagAndCounterWithoutUnlocking(resizeFlag,CurrentNoOfResizing); // check whether the resizing is going on
                if(resizeFlag || (PreviousNoOfResizing != CurrentNoOfResizing) ){	// if resize is onging
                    PreviousNoOfResizing = CurrentNoOfResizing;
                    Segments[Seg].Unlock();
                    return true;	// skip this round, and restart adding
                }
                seg1 = getSegment(hashCode);
                MasterResizeLock.Unlock();	// unlock the resizing lock

                if(seg1 != Seg){
                    if(seg1 > Seg)
                        cout << "============== hash : findNearestEmptyLocation : ERROR IN OBTAINING LOCK ======================" << endl;
                    Segments[seg1].WriteLock();		// lock the former segment
                }

        /*	if ( (hashIntegrityCheck == IC_VALIDATE_HASH_DURING_GC) ) {	// if GC map new node if it is exist.
                        if(integrityChecker ->CheckIfDeadNode( Table[start].getIdP() )){
                                cout << "Hash : findNearestEmptyLocation:  The node will be removed wrongly" << endl;
                                Table[start].DumpToScreen();
                                exit(-1);
                        }
                }*/
                removeItemInfo(hashCode,start );	// remove the hopinfo corresponding to 'start' in bucket  'hashcode'

                if(seg1 != Seg)
                    Segments[seg1].Unlock();		// unlock the former segment
            }
        }

        location = start;	// store the new value
        return true;
    }

    bool findNearestEmptyLocation(T start, T & location, unsigned short & Seg, bool & resizeFlag, unsigned char & PreviousNoOfResizing, bool & lastPartFlag ){
        unsigned char CurrentNoOfResizing;
        bool datedMarkNodeFlag;	// if this flag is 'false' then nothing is affected with it.
        T threshold;
        T upperLimit;
        // get the part no. and threshold, you don't have to unlock as the start always is not from the marginal area
        unsigned char partNo = getPartOfTable(start , threshold,  upperLimit, lastPartFlag);

        //
        T unbaisedLoc  = (start - threshold);		// remove the offset from 'start' location
        if (GCEnabled == H_GC_MARK_ENABLED)
            datedMarkNodeFlag = ( TableAddressList[partNo][unbaisedLoc].getReferenceCounter() != CURRENT_MARK);	// if MARK is updated, then rise flag, else, it will be false
        else if (GCEnabled == H_GC_INCREMENTAL_MARK_ENABLED)
            datedMarkNodeFlag = ( TableAddressList[partNo][unbaisedLoc].getReferenceCounter() != CURRENT_MARK) && ( TableAddressList[partNo][unbaisedLoc].getReferenceCounter() != PERMENANT_NODE);	// if MARK is updated or permenant, then rise flag, else, it will be false
        else
            datedMarkNodeFlag = false ;

        do{ // repeat until you find an empty location (After locking)
            while( !( (TableAddressList[partNo][unbaisedLoc].isEmpty() || datedMarkNodeFlag)  ) ){
                start++;
                if (start == upperLimit){   // if you reached the end of this part
                    // get the next part no. and threshold
                    partNo = getPartOfTable(start , threshold,  upperLimit, lastPartFlag);
                }

                unbaisedLoc  = start - threshold;
                if (GCEnabled == H_GC_MARK_ENABLED)
                    datedMarkNodeFlag = ( TableAddressList[partNo][unbaisedLoc].getReferenceCounter() != CURRENT_MARK);	// if the RC is equal to zero, then rise flag
                else if (GCEnabled == H_GC_INCREMENTAL_MARK_ENABLED)
                    datedMarkNodeFlag = ( TableAddressList[partNo][unbaisedLoc].getReferenceCounter() != CURRENT_MARK) && ( TableAddressList[partNo][unbaisedLoc].getReferenceCounter() != PERMENANT_NODE);	// if MARK is updated or permenant, then rise flag, else, it will be false
                else
                    datedMarkNodeFlag = false;	// if the RC is equal to zero, then rise flag
                if (start > TotalCapacity) {	//if you reached the last place in the hash, return a number bigger than the capacity of the table
                    if (lastPartFlag){
                        MasterResizeLock.Unlock();
                    }
                    return false;
                }
            }

            if (lastPartFlag){  // if you are in the marginal part, don't lock
                Seg = getSegment(start);
                MasterResizeLock.Unlock();
            }
            else{   // if you are not in the marginal area; then, check for resize
                getResizingFlagAndCounterWithoutUnlocking(resizeFlag,CurrentNoOfResizing); // check whether the resizing is going on
                if(resizeFlag || (PreviousNoOfResizing != CurrentNoOfResizing) ){	// if resize is onging
                        PreviousNoOfResizing = CurrentNoOfResizing;
                        return true;	// skip this round, and restart adding
                }
                Seg = getSegment(start);
                MasterResizeLock.Unlock();	// unlock the resizing lock
            }
            // check if this location is still empty or dated
            Segments[Seg].WriteLock();		// aquire write lock in this segment
            if (lastPartFlag){
                // aquire the master lock again (this happens to preserve the order of locking "Segments -> master resize ")
                getResizingFlagAndCounterWithoutUnlocking(resizeFlag,CurrentNoOfResizing); // check whether the resizing is going on
                if(resizeFlag || (PreviousNoOfResizing != CurrentNoOfResizing) ){	// if resize is onging
                        PreviousNoOfResizing = CurrentNoOfResizing;
                        return true;	// skip this round, and restart adding
                }
            }
            // try to acquire the segment and check for the resizing conditions
            if (TableAddressList[partNo][unbaisedLoc].isEmpty()){ // if the location is empty or dated
                break;      // get out from searching
            }
            else
                Segments[Seg].Unlock(); // unlock as we have another round

        }
        while (true);

        if ( (GCEnabled == H_GC_MARK_ENABLED) || (GCEnabled == H_GC_INCREMENTAL_MARK_ENABLED)){	// if GC is enabled, then check if the location is choosen due it is a dead node or not (dead node is the node with RF != CURRENT_MARK)
            if ( (!TableAddressList[partNo][unbaisedLoc].isEmpty()) && datedMarkNodeFlag ) { // if it is a stored node (not empty), with dated mark pointer to it, and we are have a 'write' lock on it, then we have a dead node that MUST BE removed
                // update hopinfo for the bucket store this node
                BDDNode <T,V,L> node1 = TableAddressList[partNo][unbaisedLoc].getNode();
                T hashCode = getHashCode(getHashString(node1));	// get bucket
                // we have the possiblity that hashCode and start are in a different segments , so we will have to lock them in order, as well as take care of the possibility that resizing may be onging

                unsigned short seg1;
                if (lastPartFlag){
                    seg1 = getSegment(hashCode);
                }
                else {
                    getResizingFlagAndCounterWithoutUnlocking(resizeFlag,CurrentNoOfResizing); // check whether the resizing is going on
                    if(resizeFlag || (PreviousNoOfResizing != CurrentNoOfResizing) ){	// if resize is onging
                        PreviousNoOfResizing = CurrentNoOfResizing;
                        Segments[Seg].Unlock();
                        return true;	// skip this round, and restart adding
                    }
                    seg1 = getSegment(hashCode);
                    MasterResizeLock.Unlock();	// unlock the resizing lock
                }
                if(seg1 != Seg){
                    if(seg1 > Seg)
                        cout << "============== hash : findNearestEmptyLocation : ERROR IN OBTAINING LOCK ======================" << endl;
                    Segments[seg1].WriteLock();		// lock the former segment
                }

        /*	if ( (hashIntegrityCheck == IC_VALIDATE_HASH_DURING_GC) ) {	// if GC map new node if it is exist.
                        if(integrityChecker ->CheckIfDeadNode( TableAddressList[partNo][unbaisedLoc].getIdP() )){
                                cout << "Hash : findNearestEmptyLocation:  The node will be removed wrongly" << endl;
                                TableAddressList[partNo][unbaisedLoc].DumpToScreen();
                                exit(-1);
                        }
                }*/
                removeItemInfo(hashCode,start );	// remove the hopinfo corresponding to 'start' in bucket  'hashcode'

                if(seg1 != Seg)
                    Segments[seg1].Unlock();		// unlock the former segment
            }
        }

        location = start;	// store the new value
        return true;
    }
    // do the same function of the above function + return an pointer to the node , but don't unlock the bucket of the hash code . // if any resizing occurs the output of contains will be false
    // node is input
    // bp : is output
    /// lock: input
    // finalSeg : output
    // nextSeg : output (used only when the lock is false)
    // nextNeighbor: location of the last location in the bucket
    bool containsWithoutUnlocking(BDDNode <T,V,L> & node, BDDPointer <T,V> &bp, bool lock, T &nextNeighbor, bool & resizeFlag, unsigned char & PreviousNoOfResizing ) {
        unsigned char CurrentNoOfResizing;
        bool flag2 = true;
        bool flag = true;
        T timeStamp;
        T hopinfo;	// get its hop information
        T hashString;
        T hashCode;
        unsigned short previousSeg;		// determine the segment
        unsigned short nextSeg ;			// store the segment to be locked
        if(ResizeType == H_RESIZE_TYPE_SWAP){	// if swap resize is enabled
            hashString = getHashString(node);	// compute hash String
            hashCode = getHashCode(hashString);	// compute hash code
            nextNeighbor = hashCode;		// initialize the nextNeighbor
            do{
                timeStamp = hopinfo = Table[hashCode].getHopInfo();	// get its hop information
                while ( getNearestNeighbor(hashCode, hopinfo, nextNeighbor) ){	// get the nearest neighbor
                    if(lock){
                        if (flag){  // if this time you swap segments
                            getResizingFlagAndCounterWithoutUnlocking( resizeFlag , CurrentNoOfResizing ); // check whether the resizing is going on
                            previousSeg = getSegment(nextNeighbor);     // get the segment
                            if(resizeFlag || (PreviousNoOfResizing != CurrentNoOfResizing) ){	// if resize is onging
                                PreviousNoOfResizing = CurrentNoOfResizing;
                                return false;	// skip this round, and restart adding
                            }
                            MasterResizeLock.Unlock();	// unlock the resizing lock
                            Segments[previousSeg].ReadLock();           // aquire read lock
                 //           cout << "segments : " << previousSeg <<  " is locked 1" << endl;
                            if (timeStamp != Table[hashCode].getHopInfo()){ // hif the hopinfo for bucket is changed, then you have to restart, note that you now get a lock on previous segments
                                flag = false;                               // the first time is finished
                                flag2 = false;                              // skip the error message at the end of the function
                                hopinfo = Table[hashCode].getHopInfo();
                     //           timeStamp = hopinfo;
                                nextSeg = previousSeg;
                                break;
                            }
                            else{
                                flag = false;                               // the first time is finished
                                nextSeg = previousSeg;
                            }
                        }
                        else {
                            getResizingFlagAndCounterWithoutUnlocking( resizeFlag , CurrentNoOfResizing ); // check whether the resizing is going on
                            nextSeg = getSegment(nextNeighbor);		// determine the segment of the next bucket
                            if(resizeFlag || (PreviousNoOfResizing != CurrentNoOfResizing) ){	// if resize is onging
                                PreviousNoOfResizing = CurrentNoOfResizing;
                  //              if (nextSeg != previousSeg){
                                Segments[previousSeg].Unlock();
                   //             }
                                return false;	// skip this round, and restart adding
                            }
                            MasterResizeLock.Unlock();	// unlock the resizing lock

                            if ((nextSeg != previousSeg)) {
                                Segments[previousSeg].Unlock();		// unlock the previous segment EXCEPT the first segment
              //                  cout << "segments : " << previousSeg <<  " is unlocked 2" << endl;
                                Segments[nextSeg].ReadLock();		// lock the new segment
              //                  cout << "segments : " << nextSeg <<  " is locked 3" << endl;
                                previousSeg = nextSeg;
                                if (timeStamp != Table[hashCode].getHopInfo()){
                       //             timeStamp = hopinfo = Table[hashCode].getHopInfo(); // update the time stamp
                                    flag2 = false;
                                    break;  // restart again
                                }
                             }
                            else{

                            }
                        }
                    }


                    bool datedMarkNodeFlag;	// if this flag is 'false' then nothing is affected with it.

                    // THIS OPTION SHOULD BE UPDATED, IF WE ENABLE THE FETCHING OF DATED NODE FROM HASH TABLE
                    if (GCEnabled == H_GC_MARK_ENABLED)
                            datedMarkNodeFlag = ( Table[nextNeighbor].getReferenceCounter() != CURRENT_MARK);	// if MARK is updated, then rise flag, else, it will be false
                    else if (GCEnabled == H_GC_INCREMENTAL_MARK_ENABLED)
                            datedMarkNodeFlag = ( Table[nextNeighbor].getReferenceCounter() != CURRENT_MARK) && ( Table[nextNeighbor].getReferenceCounter() != PERMENANT_NODE);	// if MARK is updated or permenant, then rise flag, else, it will be false
                    else
                            datedMarkNodeFlag = false ;

                    if ((!datedMarkNodeFlag) && Table[nextNeighbor].getNode().isEqual(node) && (!Table[nextNeighbor].isEmpty()) ){	// if you found the match (that it is not dated) store the pointer unlock the read lock then return true
                        bp.setLocalId(Table[nextNeighbor].getLocalId());	// store the id in the pointer
                        bp.setHashString(hashString);				// store the hash string
                        bp.clearTemporary();
                        bp.ClearInverted();

                        if (lock){
                           Segments[nextSeg].Unlock();
                           return true;
                        }
                        else {
                            return true;
                        }

                    }
                }
                // if you didn't find a match; then unlock the read lock then return false
                if(lock){
                    if(flag && flag2){  // if both flags are rised, then we didn't enter the while loop, and hence, we didn't lock any lock
                        return false;
                    }
                    else if (flag2 && (!flag)){ // we enter the while loop but we didn't find the key
                         Segments[nextSeg].Unlock();
                         return false;
                    }
                    else if ( (!flag2) && (!flag) ){
                        flag2 = true;
                    }
                }
                else
                    return false;
            }
            while(true);
        }
        else{	// if incremental resize is enabled
            bool lastPartFlag = false;
            T upperLimit;
            T threshold;
            hashString = getHashString(node);               // compute hash String
            // get the hash code with master resize lock, if resize is onging, then restart the whole process in find or add method
            // note that, in this case the function is returned while you acquire teh resize lock. Otherwise, you lock the resize lock
            if(lock){
                hashCode = getHashCodeWithLock(hashString);     // compute hash code
                if(resizeOnging){	// if resize is onging
                    PreviousNoOfResizing = NoOfLookUpTables;    // get no. of resizing
                    resizeFlag = resizeOnging;
                    return false;	// skip this round, and restart adding
                }
                else{
                    CurrentNoOfResizing = NoOfLookUpTables;
                    MasterResizeLock.Unlock();
                }
            }
            else{
                hashCode = getHashCode(hashString);     // compute hash code
            }
            // Here you don't have any aquired lock.
            unsigned char partNo;
            T unbaisedLoc;

            do{
                // get part no and threashold, note that you are in 'hashCode'; so, you will never have the resize lock
                partNo = getPartOfTable(hashCode,threshold, upperLimit, lastPartFlag);
                unbaisedLoc  = hashCode - threshold;
                timeStamp = hopinfo = TableAddressList[partNo][unbaisedLoc].getHopInfo();	// get its hop information
                // store the location for hash code
                unsigned char partNo1 = partNo;
                T unbaisedLoc1 = unbaisedLoc;
                while ( getNearestNeighbor(hashCode, hopinfo, nextNeighbor) ){	// get the nearest neighbor
                     // if you are in the same part, get next unbaised location
                    if(lock){
                        // get new part and threshold if there is any.
                        if(nextNeighbor >= upperLimit){ // if you finished this part, get the next part
                            // get the new part and threshold
                            partNo = getPartOfTable(nextNeighbor,threshold, upperLimit, lastPartFlag);
                            // check if the resize is onging or is already done
                            if (lastPartFlag){ // if it is not the last part, release the lock
                                if(resizeOnging || (PreviousNoOfResizing != NoOfLookUpTables ) ){	// if resize is onging
                                    PreviousNoOfResizing = NoOfLookUpTables;
                                    return false;	// skip this round, and restart adding
                                }
                            }
                        }
                        unbaisedLoc  = nextNeighbor - threshold;
                        if (flag){  // if this time you swap segments
                            if(lastPartFlag){   // if this os the last part , you don't have to check for resize
                                previousSeg = getSegment(nextNeighbor);     // get the segment
                                MasterResizeLock.Unlock();
                            }
                            else{   // if this the last part, you are already the resize master lock
                                getResizingFlagAndCounterWithoutUnlocking( resizeFlag , CurrentNoOfResizing ); // check whether the resizing is going on
                                previousSeg = getSegment(nextNeighbor);     // get the segment
                                if(resizeFlag || (PreviousNoOfResizing != NoOfLookUpTables) ){	// if resize is onging
                                    PreviousNoOfResizing = NoOfLookUpTables;
                                    return false;	// skip this round, and restart adding
                                }
                                MasterResizeLock.Unlock();	// unlock the resizing lock
                            }

                            Segments[previousSeg].ReadLock();           // aquire read lock
                            if (lastPartFlag){
                                // aquire the master lock again (this happens to preserve the order of locking "Segments -> master resize ")
                                getResizingFlagAndCounterWithoutUnlocking( resizeFlag , CurrentNoOfResizing ); // check whether the resizing is going on
                                if(resizeFlag || (PreviousNoOfResizing != NoOfLookUpTables) ){	// if resize is onging
                                    PreviousNoOfResizing = NoOfLookUpTables;
                                    return false;	// skip this round, and restart adding
                                }
                            }

                            if (timeStamp != TableAddressList[partNo1][unbaisedLoc1].getHopInfo()){ // if the hopinfo for bucket is changed, then you have to restart, note that you now get a lock on previous segments
                                flag = false;                               // the first time is finished
                                flag2 = false;                              // skip the error message at the end of the function
                                nextSeg = previousSeg;
                                if (lastPartFlag){  //
                                    MasterResizeLock.Unlock();	// unlock the resizing lock
                                }
                                break;
                            }
                            else{
                                flag = false;                               // the first time is finished
                                nextSeg = previousSeg;
                            }
                        }
                        else {
                            if(lastPartFlag){   // if this is the last part , you don't have to check for resize
                                nextSeg = getSegment(nextNeighbor);		// determine the segment of the next bucket
                            }
                            else{
                                getResizingFlagAndCounterWithoutUnlocking( resizeFlag , CurrentNoOfResizing ); // check whether the resizing is going on
                                nextSeg = getSegment(nextNeighbor);		// determine the segment of the next bucket
                                if(resizeFlag || (PreviousNoOfResizing != NoOfLookUpTables) ){	// if resize is onging
                                    PreviousNoOfResizing = NoOfLookUpTables;
                                    Segments[previousSeg].Unlock();
                                    return false;	// skip this round, and restart adding
                                }
                                MasterResizeLock.Unlock();	// unlock the resizing lock
                            }

                            if ((nextSeg != previousSeg)) {
                                Segments[previousSeg].Unlock();		// unlock the previous segment EXCEPT the first segment
                                Segments[nextSeg].ReadLock();		// lock the new segment
                                previousSeg = nextSeg;
                                if (timeStamp != TableAddressList[partNo1][unbaisedLoc1].getHopInfo()){
                                    flag2 = false;
                                    if (lastPartFlag){  //
                                        MasterResizeLock.Unlock();	// unlock the resizing lock
                                    }
                                    break;  // restart again
                                }
                            }
                            else{

                            }
                        }
                    }
                    else{
                        // get new part and threshold if there is any.
                        if(nextNeighbor >= upperLimit){ // if you finished this part, get the next part
                            partNo = getPartOfTable(nextNeighbor,threshold, upperLimit);
                        }
                        unbaisedLoc  = nextNeighbor - threshold;
                    }


                    bool datedMarkNodeFlag;	// if this flag is 'false' then nothing is affected with it.

                    if (GCEnabled == H_GC_MARK_ENABLED)
                        datedMarkNodeFlag = ( TableAddressList[partNo][unbaisedLoc].getReferenceCounter() != CURRENT_MARK);	// if MARK is updated, then rise flag, else, it will be false
                    else if (GCEnabled == H_GC_INCREMENTAL_MARK_ENABLED)
                        datedMarkNodeFlag = ( TableAddressList[partNo][unbaisedLoc].getReferenceCounter() != CURRENT_MARK) && ( TableAddressList[partNo][unbaisedLoc].getReferenceCounter() != PERMENANT_NODE);	// if MARK is updated or permenant, then rise flag, else, it will be false
                    else
                        datedMarkNodeFlag = false ;

                    if ((!datedMarkNodeFlag) && TableAddressList[partNo][unbaisedLoc].getNode().isEqual(node) && (!TableAddressList[partNo][unbaisedLoc].isEmpty()) ){	// if you found the match (that it is not dated) store the pointer unlock the read lock then return true
                        bp.setLocalId(TableAddressList[partNo][unbaisedLoc].getLocalId());	// store the id in the pointer
                        bp.setHashString(hashString);				// store the bucket
                        bp.clearTemporary();
                        bp.ClearInverted();

                        if (lock){
                            if (lastPartFlag){  //
                                MasterResizeLock.Unlock();	// unlock the resizing lock
                            }
                           Segments[nextSeg].Unlock();
                           return true;
                        }
                        else {
                            return true;
                        }
                    }
                }
                // if you didn't find a match; then unlock the read lock then return false
                if(lock){
                    if (lastPartFlag){  //
                        MasterResizeLock.Unlock();	// unlock the resizing lock
                    }
                    if(flag && flag2){  // if both flags are rised, then we didn't enter the while loop, and hence, we didn't lock any lock
                        return false;
                    }
                    else if (flag2 && (!flag)){ // we enter the while loop but we didn't find the key
                         Segments[nextSeg].Unlock();
                         return false;
                    }
                    else if ( (!flag2) && (!flag) ){
                        flag2 = true;
                    }
                }
                else
                    return false;
            }
            while(true);
        }
    }
    // get the next local id for this bucket
    V getNextId(T hashCode){
        if(ResizeType == H_RESIZE_TYPE_SWAP){

            T hopinfo = Table[hashCode].getHopInfo();	// get its hop information
            T nextNeighbor;
            T idMap = 0;        // bit map representation of the local ids in this bucket
            // obtain a bit map for ids
            while ( getNearestNeighbor(hashCode, hopinfo, nextNeighbor) ){	// get the nearest neighbor

                // the fetched node MUST NOT be empty or dated one
                bool datedMarkNodeFlag;
                if (GCEnabled == H_GC_MARK_ENABLED)
                        datedMarkNodeFlag = ( Table[nextNeighbor].getReferenceCounter() != CURRENT_MARK);	// if MARK is updated, then rise flag, else, it will be false
                else if (GCEnabled == H_GC_INCREMENTAL_MARK_ENABLED)
                        datedMarkNodeFlag = ( Table[nextNeighbor].getReferenceCounter() != CURRENT_MARK) && ( Table[nextNeighbor].getReferenceCounter() != PERMENANT_NODE);	// if MARK is updated or permenant, then rise flag, else, it will be false
                else
                        datedMarkNodeFlag = false ;

                if ( (!datedMarkNodeFlag) && (!Table[nextNeighbor].isEmpty()) ){	// if you found the match,  store the pointer unlock the read lock then return true
                    idMap |= (1<< Table[nextNeighbor].getLocalId());// assign bits according to id (every id has a location; so that, when the id is exist the location for it is set to one otherwise it is set to zero)
                }
            }
            // get the first available id
            unsigned char id = 0;
            while (idMap & 1){ // while the LSB in id map is one
                idMap >>=1;
                id++;
            }
            return id;
        }
        else{
            T threshold;
            unsigned char partNo = getPartOfTable(hashCode,threshold);
            T unbaisedLoc  = hashCode - threshold;
            T hopinfo = TableAddressList[partNo][unbaisedLoc].getHopInfo();	// get its hop information
            T nextNeighbor;
            T idMap = 0;        // bit map representation of the local ids in this bucket
            while ( getNearestNeighbor(hashCode, hopinfo, nextNeighbor) ){	// get the nearest neighbor
                partNo = getPartOfTable(nextNeighbor,threshold);
                unbaisedLoc  = nextNeighbor - threshold;
                // the fetched node MUST NOT be empty or dated one
                bool datedMarkNodeFlag;
                if (GCEnabled == H_GC_MARK_ENABLED)
                    datedMarkNodeFlag = ( TableAddressList[partNo][unbaisedLoc].getReferenceCounter() != CURRENT_MARK);	// if MARK is updated, then rise flag, else, it will be false
                else if (GCEnabled == H_GC_INCREMENTAL_MARK_ENABLED)
                    datedMarkNodeFlag = ( TableAddressList[partNo][unbaisedLoc].getReferenceCounter() != CURRENT_MARK) && ( TableAddressList[partNo][unbaisedLoc].getReferenceCounter() != PERMENANT_NODE);	// if MARK is updated or permenant, then rise flag, else, it will be false
                else
                    datedMarkNodeFlag = false ;

                if ( (!datedMarkNodeFlag) &&  (!TableAddressList[partNo][unbaisedLoc].isEmpty()) ){	// if you found the match,  store the pointer unlock the read lock then return true
                    idMap |= (1<< TableAddressList[partNo][unbaisedLoc].getLocalId());// assign bits according to id (every id has a location; so that, when the id is exist the location for it is set to one otherwise it is set to zero)
                }
            }
            // get the first available id
            unsigned char id = 0;
            while (idMap & 1){ // while the LSB in id map is one
                id++;
                idMap >>=1;
            }
            return id;
        }
    }
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // private validation related functions (validation by simulation and by isomorphic comparision)
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // calculate the value of a BDD with pointer bp and input 'inputvec', inv : indicate wether the value is inverted or not.
    char getDecision(BDDPointer <T,V> bp, char * inputvec, int &inv){
            // regulate pointer if neccessary
            if (bp.isInverted()){	// is the pointer is inverted, then regulate it.
    //		cout << "hash : getDecision : invert pointer" << endl;
                    bp = bp.invert();
                    inv = 1-inv;	// flip inverse
            }
            // terminal case
            if (One.isEqual(bp)){	// if the pointer points to one node, return '1' (terminal case)
    //		cout << "hash : getDecision : return '1'" << endl;
                    return '1';
            }
            // non-terminal case, recursively find then or else (according to inputvec)
            L level = getLevelFromID(bp);	// get the level

            char decision = inputvec[level - 1];	// get value for this value
    //	cout << "hash : getDecision : you are at level : " << level << " and your decision is :" << decision << endl;

            BDDNode <T,V,L> result;
            getInfo( bp, result);
            if (decision == 'X'){	// if value is 'don't care'; so, get 'else' and 'then' values
                    int inv1 = 0;
                    int inv2 = 0;
                    bp = result.getElse();
    //		cout << "hash : getDecision : X is detected" << endl;
    //		cout << "hash : getDecision : calculate else pointer" << endl;
                    char c1 =  getDecision(bp,inputvec,inv1);
                    bp = result.getThen();
    //		cout << "hash : getDecision : calculate then pointer" << endl;
                    char c2  = getDecision(bp,inputvec,inv2);
                    if ((c1 == 'X') || (c2 == 'X') )		// if one of the branches equals 'X' , then return 'x'
                            return 'X';

                    if(inv1 != 0)	// if the value is regular, return it as it is
                            c1 = invert(c1);

                    if(inv2 != 0)	// if the value is regular, return it as it is
                            c2 = invert(c2);

                    if (c1 == c2)		// if they are the same, return one of them
                            return c1;
                    else			// if they are different, returns 'x'
                            return 'X';
            }
            if(decision == '0'){	// if '0', choose 'else' pointer
                    bp = result.getElse();
                    return getDecision(bp,inputvec,inv);
            }
            else{			// if '1', choose 'then' pointer
                    bp = result.getThen();
                    return getDecision(bp,inputvec,inv);
            }
    }
    // invert the input
    char invert(char c){
        if (c == '0')
            return '1';
        else
            return '0';
    }


public:
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // Constructors
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // constructors
    Hash(){}
    Hash(T inCapacity,
            unsigned short ConcurrencyDegree,
            unsigned char MaxUtilization,
            unsigned char BucketSize,
            unsigned char hashIntegrityCheck,
            IntegrityChecker <T,V,L,K,FLG>*& integrityChecker,
            unsigned char GCEnabled /*,	GCQueue * gcQueue*/,
            unsigned char ResizeType,
            BDDPointer <T,V> One){
        
        this->One = One;
        ResizingFlag = false;
        this->ResizeType = ResizeType;
//	this->gcQueue = gcQueue;
        this->GCEnabled = GCEnabled;
        this->hashIntegrityCheck = hashIntegrityCheck;
        this->integrityChecker = integrityChecker;
        this->MaxUtilization = MaxUtilization;
        this->inCapacity = inCapacity;
        this->ConcurrencyDegree = ConcurrencyDegree;
        this->BucketSize = BucketSize;
        this->TotalCapacity = ApproximateToBiggerPowerOf2( ceil(inCapacity/MaxUtilization * 100));	// get the total capacity
        this->BucketMask = TotalCapacity - 1;
        FirstSegmentFinishedMap = false;
        // allocate and initialize segments
        this->Segments = new Segment[ConcurrencyDegree];
        NodeCounter = 0;
        TotalCapacity = TotalCapacity + BucketSize -1;
        TableAddressList = (Bucket <T,V,L> **) malloc( sizeof(Bucket <T,V,L>*) * HASH_NO_OF_LOOK_UP_TABLES);
        if(ResizeType == H_RESIZE_TYPE_SWAP){	// if swap resizing is enabled
                Table = new Bucket <T,V,L> [TotalCapacity];	// add a margin for the last element
        }
        else{	// if the swap resizing is chosen, then we make marginal nodes as a separate part, so that it can be moves from part list location to another as the resizing is performed (always marginal node will be the final part)
                TableAddressList[0]  = new Bucket <T,V,L> [TotalCapacity - (BucketSize -1)];	// add a margin for the last element
                TableAddressList[1]  = new Bucket <T,V,L> [BucketSize -1];	// add a margin for the last element
                TotalNoOfTableParts = 1;	// always contains the  total no. of parts - 1
        }

        this->SegmentSize = floor((float)(TotalCapacity/ConcurrencyDegree)) + 1;
        fileCounter = 0;

        // resizing parameter initializaiton
        ResizingCondition = new Condition(&FirstSegmentLock);
        FirstPhaseCondition = new Condition(&FirstPhaseLock);
        SecondPhaseCondition = new Condition(&SecondPhaseLock);
        ThirdPhaseCondition = new Condition(&ThirdPhaseLock);
        FourthPhaseCondition = new Condition(&FourthPhaseLock);
        NoOfLookUpTables = 0;
        resizeOnging = false;
        ResizingOccured = false;
        TempTable = NULL;
        CurrentPhase = H_RESIZE_INITIAL_PHASE;
        CurrentSegment = 0;
        FinishedCurrentSegment = 0;
        SecondPhaseFinishedCurrentSegment = 0;
        InitialResizePhaseFlag = false;
        FinalResizePhaseFlag = false;

        if (GCEnabled == H_GC_INCREMENTAL_MARK_ENABLED)
            CURRENT_MARK = INITIAL_MARK_FOR_INCREMENTAL_MARK_GC ;	// initialize mark , it is set to two , as reference counter in each bucket is
        else if (GCEnabled == H_GC_MARK_ENABLED)
            CURRENT_MARK = INITIAL_MARK_FOR_ORDINARY_MARK_GC ;	// initialize mark , it is set to zero , as reference counter in each bucket is initially set to 'zero',and before 'mark' GC is running, it will increment it to one, and the most important, the 'find nearest neighbor for the first time will check it with RC (so it have to be zero)

        cout << "segment size : " << SegmentSize << endl;
        cout << "hash size : " << TotalCapacity << endl;
//		Dump();
    }

    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // main method in this class (find or add)
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // find Or Add
    // store the node if it isn't already exists
    // Inputs: the node
    // pointer to the node
    // outputs: bool; true if the node is already exists, false : if the node is a new node
    // Reference Counter: is set to one when the node is add, and doesn't change when the node is fetched
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool findOrAdd(BDDNode <T,V,L>  &node , BDDPointer <T,V>  & bp){
        if(ResizeType == H_RESIZE_TYPE_SWAP){
            do {
                unsigned char CurrentNoOfResizing;
                bool resizeFlag;
                getResizingFlagAndCounterWithoutUnlocking(resizeFlag, CurrentNoOfResizing); // check whether the resizing is going on
                T hashString = getHashString(node);	// compute hash String
                T hashCode = getHashCode(hashString);	// compute hash code
                unsigned short seg = getSegment(hashCode);		// determine the segment

                unsigned char PreviousNoOfResizing = CurrentNoOfResizing;
                if(resizeFlag){	// if resize is onging
                    MasterResizeLock.Unlock();	// unlock the resizing lock
                    resize(CurrentNoOfResizing);
                    continue;	// skip this round, and restart adding
                }
                MasterResizeLock.Unlock();	// unlock the resizing lock
                bool lockt = true;
                unsigned short finalSeg = seg;
                T LastLoc;

                if ( containsWithoutUnlocking(node, bp, lockt, LastLoc, resizeFlag, CurrentNoOfResizing) ){ // if any resizing occurs the output of contains will be false, so there isn't any handling of resizing in this section
                    return true;	// the node is already there....
                }

                if(resizeFlag|| (PreviousNoOfResizing != CurrentNoOfResizing) ){	// if resize is onging	after contains method
                    if (resizeFlag){
                        MasterResizeLock.Unlock();	// unlock the resizing lock
                        resize(CurrentNoOfResizing);
                    }
                    else
                        MasterResizeLock.Unlock();	// unlock the resizing lock
                    continue;	// skip this round, and restart adding
                }

                // here the finalSeg is locked, and we can start search from lastLoc instead of "hashCode".

                // in order to reduce waiting during scanning the table to obtain an empty location, I utilize a read lock to search for an empty location. then, I aquire a write loc in order to modify this location. But first, I will have to check if this location remains empty after I release the read lock and before aquire the write lock. if the location is the same , this would be fine. if the location is changed, i start from the last location in hand and search again.
                T locR = hashCode;	// location obtained from read lock
                T locW = 0;		// location obtained from write lock (initialized with any value)
                bool flag = false;			// used to determine if the write and read lock are equivalent
                resizeFlag = false;
                finalSeg = seg;
                if(!findNearestEmptyLocation(locR, locW, finalSeg, resizeFlag, CurrentNoOfResizing)){	// aquire write lock
                    resize(CurrentNoOfResizing);
                    continue;				// skip the this loop
                }
                if(resizeFlag || (PreviousNoOfResizing != CurrentNoOfResizing) ){	// if resize is onging	after contains method
                    if (resizeFlag){
                        MasterResizeLock.Unlock();	// unlock the resizing lock
                        resize(CurrentNoOfResizing);
                    }
                    else
                        MasterResizeLock.Unlock();	// unlock the resizing lock
                    continue;	// skip this round, and restart adding
                }

                seg = finalSeg;

                // the lock is aquired (write lock). and the segment is "Seg"
                unsigned short seg1;		// determine the segment
                T info1;
                T loc1;
                // hop the empty location all the way toward its bucket
                T emptyLoc = locW;
                T i;
                bool resizeflag = false;
                seg1 = seg;
                unsigned short seg2 = seg;
                BDDNode <T,V,L>  nodeTemp;		// get the node
                T hashCodeTemp;
                while ( emptyLoc >= (BucketSize+ hashCode)){
                    flag = false;	// initialization for the next round
                    //cout << "you are far from the desired bucket, in Hash" << endl;
                    T start = emptyLoc - (BucketSize-1);
                    for(i =  start; i < emptyLoc ; i++) {
                        getResizingFlagAndCounterWithoutUnlocking(resizeFlag,CurrentNoOfResizing); // check whether the resizing is going on
                        if(resizeFlag || (PreviousNoOfResizing != CurrentNoOfResizing) ){	// if resize is onging	after contains method
                            Table[emptyLoc].clearAllFlags();	// clear any flags in the empty locaton, before unlocking , so that resizing doesn't copy it (as resizing is done after locking all locks)
                            Table[emptyLoc].setReferenceCounter(0);
                            Segments[seg].Unlock();		// unlock the latter segment

                            if (resizeFlag){
                                MasterResizeLock.Unlock();	// unlock the resizing lock
                                resize(CurrentNoOfResizing);
                            }
                            else
                                MasterResizeLock.Unlock();	// unlock the resizing lock
                            resizeflag = true;
                            break;	// quit the loop
                        }
                        seg1 = getSegment(i);
                        MasterResizeLock.Unlock();	// unlock the resizing lock

                        if(seg1 != seg){
                            if(seg1 > seg)
                                    cout << "========================= ERROR IN OBTAINING LOCK ============================" << endl;
                            Segments[seg1].WriteLock();		// lock the former segment
                        }

                        info1 = Table[i].getHopInfo();	// get info for this bucket
                        while(getNearestNeighbor(i,info1,loc1)){

                            getResizingFlagAndCounterWithoutUnlocking(resizeFlag,CurrentNoOfResizing); // check whether the resizing is going on
                            if(resizeFlag || (PreviousNoOfResizing != CurrentNoOfResizing) ){	// if resize is onging	after contains method
                                Segments[seg].Unlock();		// unlock the latter segment
                                if(seg1 != seg){		// if you have two locks
                                        Segments[seg1].Unlock();		// aquire write lock in this segment (unlock the older lock)
                                }
                                if (resizeFlag){
                                        MasterResizeLock.Unlock();	// unlock the resizing lock
                                        resize(CurrentNoOfResizing);
                                }
                                else
                                        MasterResizeLock.Unlock();	// unlock the resizing lock
                                resizeflag = true;
                                break;	// quit the loop
                            }
                            seg2 = getSegment(loc1);
                            MasterResizeLock.Unlock();	// unlock the resizing lock

                            nodeTemp = Table[loc1].getNode();		// get the node of the targeted bucket
                            hashCodeTemp = getHashCode(getHashString(nodeTemp));	// compute hash code

                            if (hashCodeTemp == i){ // if the hashcode for the target bucket equal to the hashcode of the request

                                if ((loc1 < emptyLoc) ) {	// if the bucket isn't reserved, you can swap
                                    flag = true;	//rise the flag
                              /*      if (loc1 == 423){
                                        cout << "node with local id : " << (T) Table[loc1].getLocalId()   << " will be moved from : " << loc1 << " to : " << emptyLoc << endl;
                                        cout << "hop info for bucket : " << i  << " is : " << Table[i].getHopInfo() << endl;
                                    }*/

                                    Table[emptyLoc].setFlags(Table[loc1].getFlags());
                                    BDDNode <T,V,L>  req = Table[loc1].getNode();
                                    Table[emptyLoc].setNode(req);	// swap the nodes
                                    Table[emptyLoc].setReferenceCounter(Table[loc1].getReferenceCounter());	// swap Refenerence counters
                                    Table[emptyLoc].setLocalId(Table[loc1].getLocalId());
                                    updateInfo (i,loc1,emptyLoc);	// update info
                                    emptyLoc = loc1;			// update the new empty location
                                    if ( (Table[emptyLoc].getLocalId() != Table[loc1].getLocalId()) || (!Table[emptyLoc].getNode().isEqual(req)) || Table[emptyLoc].getFlags() != Table[loc1].getFlags()){
                                        cout << "hash: find or add: requests are not swapped correctly" << endl;
                                        cout << "req at loc1" << endl;
                                        req.DumpToScreen();
                                        cout << "req at emptyLoc" << endl;
                                        Table[emptyLoc].DumpToScreen();
                                        exit(-1);
                                    }
                                    break;
                                }
                            }
                            else{
                                mutex.lock();
                                cout << "hash: find or add : error in swapping nodes" << endl;
                                cout << "i =  " << i << " hashCodeTemp = : " << hashCodeTemp << " loc1 : " << loc1 << endl;
                                cout << "emptyLoc : " << emptyLoc << " hashCode : " << hashCode << endl;
                                mutex.unlock();
                                exit(-1);
                            }
                        }
                        if(resizeflag){ // if resizing is done in the main loop, then skip this loop
                            break;
                        }
                        if((seg1 != seg) && (seg2 == seg) ){	// if you have loc1 and emptyLoc in the same segment, and i in another segment, then unlock the seg1 " segment of i"
                            Segments[seg1].Unlock();		// aquire write lock in this segment (unlock the older lock)
                            // you still have to aquire "seg"
                        }
                        else if((seg1 != seg) && (seg2 == seg1) ){	// if you have  emptyLoc in a segment, and loc1 and i in another segment, then unlock segment of emptyLoc "seg"
                            Segments[seg].Unlock();		// aquire write lock in this segment (unlock the older lock)
                            seg = seg1;		// update the segment (you have
                        }
                        if(flag){
                            break;				// exit "for loop" as we reach the nearest bucket in this round
                        }
                    }

                    if(resizeflag){ // if resizing is done in the main loop, then skip this loop
                            break;
                    }
                    if(flag){
                            continue;				// continue , you have make one swap, so you have to check whether you are in the neighborhood or not
                    }

                    // here we should check the flag, because if it isn't been rised, we have no space and the table will need to be resized and you have to unlock the write lock
                    if (( i == start + BucketSize-1 ) && (!flag)) {
                            // initialize empty location, as we are going to resize, some node may be replicated
                            Table[emptyLoc].clearAllFlags();
                            Table[emptyLoc].setReferenceCounter(0);
                            Segments[seg].Unlock();		// aquire read lock in this segment
                            mutex.lock();
                            cout << "hash : find or add: resize will be called for element with hash code : " << hashCode << endl;
            //		node.DumpToScreen();
                            cout << "total no. of stored nodes : "  << getNodeCount() << endl;
                            mutex.unlock();
                            resize(CurrentNoOfResizing);
                            resizeflag = true;
                            break;	// quit the loop
                    }
                }
                if(resizeflag){ // if resizing is done in the main loop, then skip this loop
                        continue;
                }
                // here we have the write lock again
                getResizingFlagAndCounterWithoutUnlocking(resizeFlag,CurrentNoOfResizing); // check whether the resizing is going on
                if(resizeFlag || (PreviousNoOfResizing != CurrentNoOfResizing) ){	// if resize is onging
                    Segments[seg].Unlock();		// unlock the latter segment

                    if (resizeFlag){
                        MasterResizeLock.Unlock();	// unlock the resizing lock
                        resize(CurrentNoOfResizing);
                    }
                    else
                        MasterResizeLock.Unlock();	// unlock the resizing lock
                    continue;	// skip this round, and restart adding
                }
                seg1 = getSegment(hashCode);
                MasterResizeLock.Unlock();	// unlock the resizing lock

                if(seg1 != seg){
                        Segments[seg1].WriteLock();		// lock the former segment
                }
                lockt = false;

                bool returnFlag;
                // the last argument "LastLoc"  has no meaning...
                finalSeg = seg;
                if (!containsWithoutUnlocking(node, bp, lockt, LastLoc, resizeFlag, CurrentNoOfResizing)){
                    if(!((PreviousNoOfResizing != CurrentNoOfResizing) || resizeFlag)){	// if resizing is onggoing, then skip adding this node to the next round
                        info1 = Table[hashCode].getHopInfo();	// get info for this bucket
                        // store the pointer for the new added node
                        V id = getNextId(hashCode);	// get next available id for this bucket
                        bp.setLocalId(id);
                        bp.setHashString(hashString);
                        bp.clearTemporary();
                        bp.ClearInverted();


                        mutexFile.lock();
                        NodeCounter++;
                        mutexFile.unlock();
                        Table[emptyLoc].clearAllFlags();	// remove any flags
                        Table[emptyLoc].setNode(node);		// add your node
                        Table[emptyLoc].setLocalId(id);

                        if (GCEnabled == H_GC_RF_ENABLED){	// set the reference counter to '1' only when RC GC is enabled. for marked
                                Table[emptyLoc].setReferenceCounter(1);			// as this a new node, set the RC with one
                        }
                        else if ( (GCEnabled == H_GC_MARK_ENABLED) || (GCEnabled == H_GC_INCREMENTAL_MARK_ENABLED) ) {
                                Table[emptyLoc].setReferenceCounter(CURRENT_MARK);			// as this a new node, set the mark to the current mark to avoid the dereferece by accident
                        }

                        AddNewItemInfo (hashCode,emptyLoc);	// update info
                        if(!Table[emptyLoc].getNode().isEqual(node)){
                            cout << "hash : find or add : node doesn't stored correctly" << endl;
                            cout << "Original node: node intended to be stored :" << endl;
                            node.DumpToScreen();
                            cout << "stored node :" << endl;
                            Table[emptyLoc].getNode().DumpToScreen();
                            cout << "bucket infos" << endl;
                            Table[emptyLoc].DumpToScreen();
                            exit(-1);
                        }
                        if ( (hashIntegrityCheck == IC_VALIDATE_WITH_DEPRECIATED_QUEUE_AFTER_APPLY) ||
                                (hashIntegrityCheck == IC_VALIDATE_WITH_DEPRECIATED_HASH) ||
                                (hashIntegrityCheck == IC_VALIDATE_WITH_DEPRECIATED_QUEUE) ||
                                (hashIntegrityCheck == IC_VALIDATE_WITH_DEPRECIATED_QUEUE_AFTER_REDUCE)) {	// if do check is enabled, then validated new node with depreciated one
                            integrityChecker ->validateUniqueTableNodeWithDepreciated(node,bp);
                        }
                        if( (hashString ==  1866059243)){
                            cout << "hash: find or add :: node with hashstring : " << hashString << " and hash code : " << hashCode << " will be added at location : " << emptyLoc << endl;
                            cout << "pointer :" << endl;
                            bp.DumpToScreen();
                            cout << endl << endl << "node" << endl;
                            node.DumpToScreen();
                            cout << endl << endl;
                            cout << "dump of emptyLoc" << endl;
                            Table[emptyLoc].DumpToScreen();
                            cout << endl << endl << "dump of hashCode" << endl;
                            Table[hashCode].DumpToScreen();
                        }
                   /*     if(emptyLoc == 423){
                            cout << "node is added at location : " << emptyLoc << endl;
                            Table[emptyLoc].DumpToScreen();
                        }*/

                        returnFlag = false;
                    }
                    else {
                        Table[emptyLoc].clearAllFlags();	// this location is empty, as you make a bubble within the hash table, you have to make it as empty , so that it can be utilized latter
                        if(seg1 != seg){
                            Segments[seg1].Unlock();		// aquire read lock in this segment
                        }
                        Segments[seg].Unlock();		// aquire read lock in this segment
                        if (resizeFlag){
                            MasterResizeLock.Unlock();	// unlock the resizing lock
                            resize(CurrentNoOfResizing);
                        }
                        else
                            MasterResizeLock.Unlock();	// unlock the resizing lock
                        continue;
                    }
                }
                else { // the data is already exist
                    returnFlag = true;
                    Table[emptyLoc].clearAllFlags();	// this location is empty, as you make a bubble within the hash table, you have to make it as empty , so that it can be utilized latter

                }
                if(seg1 != seg){
                    Segments[seg1].Unlock();		// aquire read lock in this segment
                }
                Segments[seg].Unlock();		// aquire read lock in this segment
                return returnFlag;
            }
            while(true);	// repeat for ever as long as resizing is performed
        }
        else{		// if incremental resize is enabled
            if ((node.getThen().getHashString() == 1219270945) && (node.getElse().getHashString() == 89633110) && (node.getLevel() == 34)){
                cout << "hash : find or add : qoergho" << endl;
            }
            do {
                unsigned char CurrentNoOfResizing;
                bool resizeFlag;
                getResizingFlagAndCounterWithoutUnlocking(resizeFlag, CurrentNoOfResizing); // check whether the resizing is going on
                T hashString = getHashString(node);	// compute hash String
                T hashCode = getHashCode(hashString);	// compute hash code
                unsigned short seg = getSegment(hashCode);		// determine the segment
                unsigned char PreviousNoOfResizing = CurrentNoOfResizing;
                if(resizeFlag){	// if resize is onging
                    MasterResizeLock.Unlock();	// unlock the resizing lock
                    resize(CurrentNoOfResizing);
                    continue;	// skip this round, and restart adding
                }
                MasterResizeLock.Unlock();	// unlock the resizing lock
                bool lockt = true;
                unsigned short finalSeg = seg;
                T LastLoc;

                if ( containsWithoutUnlocking(node, bp, lockt, LastLoc, resizeFlag, CurrentNoOfResizing) ){ // if any resizing occurs the output of contains will be false, so there isn't any handling of resizing in this section
                    MasterResizeLock.CheckConsistancy();
                    return true;	// the node is already there....
                }

                if(resizeFlag|| (PreviousNoOfResizing != CurrentNoOfResizing) ){	// if resize is onging	after contains method
                    if (resizeFlag){
                        MasterResizeLock.Unlock();	// unlock the resizing lock
                        resize(CurrentNoOfResizing);
                    }
                    else
                        MasterResizeLock.Unlock();	// unlock the resizing lock
                    continue;	// skip this round, and restart adding
                }
                // start to find an empty location
                T locR = hashCode;	// location obtained from read lock
                T locW = 0;		// location obtained from write lock (initialized with any value)
                bool flag = false;			// used to determine if the write and read lock are equivalent
                resizeFlag = false;
                finalSeg = seg;
                bool lastPartFlag;

                if(!findNearestEmptyLocation(locR, locW, finalSeg, resizeFlag, CurrentNoOfResizing,lastPartFlag)){	// aquire write lock
                    resize(CurrentNoOfResizing);
                    continue;				// skip the this loop
                }
                if(resizeFlag || (PreviousNoOfResizing != CurrentNoOfResizing) ){	// if resize is onging	after contains method
                    if (resizeFlag){
                        MasterResizeLock.Unlock();	// unlock the resizing lock
                        resize(CurrentNoOfResizing);
                    }
                    else
                        MasterResizeLock.Unlock();	// unlock the resizing lock
                    continue;	// skip this round, and restart adding
                }

                seg = finalSeg;

                // the lock is aquired (write lock). and the segment is "Seg"
                unsigned short seg1;		// determine the segment
                T info1;
                T loc1;
                // hop the empty location all the way toward its bucket
                T emptyLoc = locW;
                T i;
                bool resizeflag = false;
                seg1 = seg;
                unsigned short seg2 = seg;
                BDDNode <T,V,L>  nodeTemp;		// get the node
                T hashCodeTemp;
                unsigned char partNo,partNo1, partNo2;
                T unbaisedLoc, unbaisedLoc1, unbaisedLoc2;
                T threshold, threshold1;
                partNo = getPartOfTable(emptyLoc,threshold);
                unbaisedLoc  = emptyLoc - threshold;
                while ( emptyLoc >= (BucketSize+ hashCode)){
                    flag = false;	// initialization for the next round
                    //cout << "you are far from the desired bucket, in Hash" << endl;
                    T start = emptyLoc - (BucketSize-1);
                    for(i =  start; i < emptyLoc ; i++) {
                        if(lastPartFlag){   // if you are in the last part; so, you don't have to lock resize lock
                            seg1 = getSegment(i);
                        }
                        else {  // if you aren't in the last part, get the segment with resize lock
                            getResizingFlagAndCounterWithoutUnlocking(resizeFlag,CurrentNoOfResizing); // check whether the resizing is going on
                            if(resizeFlag || (PreviousNoOfResizing != CurrentNoOfResizing) ){	// if resize is onging	after contains method
                                TableAddressList[partNo][unbaisedLoc].clearAllFlags();	// clear any flags in the empty locaton, before unlocking , so that resizing doesn't copy it (as resizing is done after locking all locks)
                                TableAddressList[partNo][unbaisedLoc].setReferenceCounter(0);
                                Segments[seg].Unlock();		// unlock the latter segment

                                if (resizeFlag){
                                    MasterResizeLock.Unlock();	// unlock the resizing lock
                                    resize(CurrentNoOfResizing);
                                }
                                else
                                    MasterResizeLock.Unlock();	// unlock the resizing lock
                                resizeflag = true;
                                break;	// quit the loop
                            }
                            seg1 = getSegment(i);
                            MasterResizeLock.Unlock();	// unlock the resizing lock
                        }

                        if(seg1 != seg){
                            if(seg1 > seg){
                                cout << "========================= ERROR IN OBTAINING LOCK ============================" << endl;
                                exit(-1);
                            }
                            Segments[seg1].WriteLock();		// lock the former segment
                        }
                        unsigned char partNo1 = getPartOfTable(i,threshold1);
                        unbaisedLoc1  = i - threshold1;
                        info1 = TableAddressList[partNo1][unbaisedLoc1].getHopInfo();	// get info for this bucket
                        while(getNearestNeighbor(i,info1,loc1)){
                            if (loc1 < emptyLoc) {	// if the location is beyond the empty location
                                if(lastPartFlag){   // if you are in the last part; so, you don't have to lock resize lock
                                    seg2 = getSegment(loc1);
                                }
                                else {
                                    getResizingFlagAndCounterWithoutUnlocking(resizeFlag,CurrentNoOfResizing); // check whether the resizing is going on
                                    if(resizeFlag || (PreviousNoOfResizing != CurrentNoOfResizing) ){	// if resize is onging	after contains method
                                        Segments[seg].Unlock();		// unlock the latter segment
                                        if(seg1 != seg){		// if you have two locks
                                            Segments[seg1].Unlock();		// aquire write lock in this segment (unlock the older lock)
                                        }
                                        if (resizeFlag){
                                            MasterResizeLock.Unlock();	// unlock the resizing lock
                                            resize(CurrentNoOfResizing);
                                        }
                                        else
                                            MasterResizeLock.Unlock();	// unlock the resizing lock
                                        resizeflag = true;
                                        break;	// quit the loop
                                    }
                                    seg2 = getSegment(loc1);
                                    MasterResizeLock.Unlock();	// unlock the resizing lock
                                }

                                if (loc1 >= threshold){  // if loc is in part for 'i' and it is not with part of empty loc
                                    partNo2 = partNo;
                                    unbaisedLoc2 = loc1 - threshold;
                                }
                                else{       // if the loc is in part for 'emptyLoc'
                                    partNo2 = partNo1;
                                    unbaisedLoc2 = loc1 - threshold1;
                                }

                                nodeTemp = TableAddressList[partNo2][unbaisedLoc2].getNode();		// get the node of the targeted bucket
                                hashCodeTemp = getHashCode(getHashString(nodeTemp));	// compute hash code

                                if (hashCodeTemp == i){ // if the hashcode for the target bucket equal to the hashcode of the request
                                    flag = true;	//rise the flag
                                     if ( (loc1 == 111) || (loc1 == 135) || (loc1 == 136) || (loc1 == 246) || (loc1 == 142) ){
                                        mutex.lock();
                                        cout << "node with local id : " << (T) TableAddressList[partNo2][unbaisedLoc2].getLocalId()   << " will be moved from : " << loc1 << " to : " << emptyLoc << endl;
                                        cout << "hop info for bucket : " << i  << " is : " << TableAddressList[partNo1][unbaisedLoc1].getHopInfo() << endl;
                                        cout << "parameter" << endl;
                                        cout << "partNo : " << (T) partNo << " unbaisedLoc : " << unbaisedLoc << endl;
                                        cout << "partNo1 : " << (T) partNo1 << " unbaisedLoc1 : " << unbaisedLoc1 << endl;
                                        cout << "partNo2 : " << (T) partNo2 << " unbaisedLoc2 : " << unbaisedLoc2 << endl;
                                        mutex.unlock();
                                    }
                                    if (emptyLoc == 219){
                                        cout << "hash : find or add : node at location : " << loc1 << " will be swaped to location : " << emptyLoc << endl;
                                        cout << "swaped node " << endl;
                                        TableAddressList[partNo2][unbaisedLoc2].getNode().DumpToScreen();
                                    }
                                    if (loc1< threshold){   // if the 'loc' is not in the marginal area
                                        threshold = threshold1;
                                        if(lastPartFlag) {   // if the marginal part is locked
                                            MasterResizeLock.Unlock();
                                            lastPartFlag = false;
                                        }
                                    }

                                    TableAddressList[partNo][unbaisedLoc].setFlags(TableAddressList[partNo2][unbaisedLoc2].getFlags());
                                    BDDNode <T,V,L>  req = TableAddressList[partNo2][unbaisedLoc2].getNode();
                                    TableAddressList[partNo][unbaisedLoc].setNode(req);	// swap the nodes
                                    TableAddressList[partNo][unbaisedLoc].setReferenceCounter(TableAddressList[partNo2][unbaisedLoc2].getReferenceCounter());	// swap Refenerence counters
                                    TableAddressList[partNo][unbaisedLoc].setLocalId(TableAddressList[partNo2][unbaisedLoc2].getLocalId());
                                    updateInfo (i,loc1,emptyLoc);	// update info
                                    emptyLoc = loc1;			// update the new empty location
                                    unbaisedLoc = unbaisedLoc2;
                                    partNo = partNo2;
 
                                    if ((!TableAddressList[partNo][unbaisedLoc].getNode().isEqual(req)) || TableAddressList[partNo][unbaisedLoc].getFlags() != TableAddressList[partNo2][unbaisedLoc2].getFlags()){
                                        cout << "hash: find or add: requests are not swapped correctly" << endl;
                                        cout << "req at loc1" << endl;
                                        req.DumpToScreen();
                                        cout << "req at emptyLoc" << endl;
                                        TableAddressList[partNo][unbaisedLoc].DumpToScreen();
                                        exit(-1);
                                    }
                                    break;
                                }
                                else{
                                    mutex.lock();
                                    cout << "hash: find or add : error in swapping nodes" << endl;
                                    cout << "i =  " << i << " hashCodeTemp = : " << hashCodeTemp << " loc1 : " << loc1 << endl;
                                    cout << "emptyLoc : " << emptyLoc << " hashCode : " << hashCode << endl;
                                    cout << "bucket at loc1" << endl;
                                    TableAddressList[partNo2][unbaisedLoc2].DumpToScreen();
                                    cout << endl <<"node at loc1" << endl;
                                    TableAddressList[partNo2][unbaisedLoc2].getNode().DumpToScreen();
                                    cout << endl << "inserted node (node we want to insert) " << endl;
                                    node.DumpToScreen();
                                    cout << endl << "temp node" << endl;
                                    nodeTemp.DumpToScreen();
                                    mutex.unlock();
                                    exit(-1);
                                }
                            }
                        }
                        if(resizeflag){ // if resizing is done in the main loop, then skip this loop
                            break;
                        }
                        if((seg1 != seg) && (seg2 == seg) ){	// if you have loc1 and emptyLoc in the same segment, and i in another segment, then unlock the seg1 " segment of i"
                            Segments[seg1].Unlock();		// aquire write lock in this segment (unlock the older lock)
                            // you still have to aquire "seg"
                        }
                        else if((seg1 != seg) && (seg2 == seg1) ){	// if you have  emptyLoc in a segment, and loc1 and i in another segment, then unlock segment of emptyLoc "seg"
                            Segments[seg].Unlock();		// aquire write lock in this segment (unlock the older lock)
                            seg = seg1;		// update the segment (you have
                        }
                        if(flag){
                            break;				// exit "for loop" as we reach the nearest bucket in this round
                        }
                    }

                    if(resizeflag){ // if resizing is done in the main loop, then skip this loop
                        break;
                    }
                    if(flag){
                        continue;				// continue , you have make one swap, so you have to check whether you are in the neighborhood or not
                    }

                    // here we should check the flag, because if it isn't been rised, we have no space and the table will need to be resized and you have to unlock the write lock
                    if (( i == start + BucketSize-1 ) && (!flag)) {
                        // initialize empty location, as we are going to resize, some node may be replicated
                        TableAddressList[partNo][unbaisedLoc].clearAllFlags();
                        TableAddressList[partNo][unbaisedLoc].setReferenceCounter(0);
                        Segments[seg].Unlock();		// aquire read lock in this segment
                        mutex.lock();
                        cout << "hash : find or add: resize will be called for element with hash code : " << hashCode << endl;
                        cout << "total no. of stored nodes : "  << getNodeCount() << endl;
                        mutex.unlock();
                        if(lastPartFlag)    // if the marginal part is locked
                            MasterResizeLock.Unlock();
                        resize(CurrentNoOfResizing);
                        resizeflag = true;
                        break;	// quit the loop
                    }
                }
                if(resizeflag){ // if resizing is done in the main loop, then skip this loop
                    continue;
                }
                // here we have the write lock again
                if(lastPartFlag){    // if the marginal part is locked
                    seg1 = getSegment(hashCode);
                }
                else {
                    getResizingFlagAndCounterWithoutUnlocking(resizeFlag,CurrentNoOfResizing); // check whether the resizing is going on
                    if(resizeFlag || (PreviousNoOfResizing != CurrentNoOfResizing) ){	// if resize is onging
                        Segments[seg].Unlock();		// unlock the latter segment

                        if (resizeFlag){
                            MasterResizeLock.Unlock();	// unlock the resizing lock
                            resize(CurrentNoOfResizing);
                        }
                        else{
                            MasterResizeLock.Unlock();	// unlock the resizing lock
                        }
                        continue;	// skip this round, and restart adding
                    }
                    seg1 = getSegment(hashCode);
                    MasterResizeLock.Unlock();	// unlock the resizing lock
                }


                if(seg1 != seg){
                    Segments[seg1].WriteLock();		// lock the former segment
                }
                lockt = false;

                bool returnFlag;
                // the last argument "LastLoc"  has no meaning...
                finalSeg = seg;
                if (!containsWithoutUnlocking(node, bp, lockt, LastLoc, resizeFlag, CurrentNoOfResizing)){
                    if(!((PreviousNoOfResizing != CurrentNoOfResizing) || resizeFlag)){	// if resizing is onggoing, then skip adding this node to the next round
                    /*    if((emptyLoc == 219) || (emptyLoc == 217) ){
                            mutex.lock();
                            cout << "hash: find or add : node with hash code " << hashCode << " will be added at location : " << emptyLoc << endl;
                            node.DumpToScreen();
                            mutex.unlock();
                        }
                        if (hashString == 523892222){
                            mutex.lock();
                            cout << "hash : find or add: aoeaw " << endl;
                            mutex.unlock();
                        }*/
                        // store the pointer for the new added node
                        V id = getNextId(hashCode);	// get next available id for this bucket
                        bp.setLocalId(id);
                        bp.setHashString(hashString);
                        bp.clearTemporary();
                        bp.ClearInverted();

                        mutexFile.lock();
                        NodeCounter++;
                        mutexFile.unlock();

                        TableAddressList[partNo][unbaisedLoc].clearAllFlags();	// remove any flags
                        TableAddressList[partNo][unbaisedLoc].setNode(node);		// add your node
                        TableAddressList[partNo][unbaisedLoc].setLocalId(id);
                        
                        // integrity checking purposes
                        if ( (hashIntegrityCheck == IC_VALIDATE_HASH) ||
                                (hashIntegrityCheck == IC_VALIDATE_QUEUE) ||
                                (hashIntegrityCheck == IC_VALIDATE_QUEUE_AFTER_APPLY) ||
                                (hashIntegrityCheck == IC_VALIDATE_QUEUE_AFTER_REDUCE) ) {	// if do check is enabled, then validated new node
                            integrityChecker -> validateUniqueTableNode(node,bp);
                        }
                        else if ( hashIntegrityCheck == IC_VALIDATE_HASH_TABLE_AFTER_REDUCE ){
                            integrityChecker -> validateUniqueTableNodeAfterReduce(node,bp);
                        }


                        if (GCEnabled == H_GC_RF_ENABLED){	// set the reference counter to '1' only when RC GC is enabled. for marked
                            TableAddressList[partNo][unbaisedLoc].setReferenceCounter(1);			// as this a new node, set the RC with one
                        }
                        else if ( (GCEnabled == H_GC_MARK_ENABLED) || (GCEnabled == H_GC_INCREMENTAL_MARK_ENABLED) ) {
                            TableAddressList[partNo][unbaisedLoc].setReferenceCounter(CURRENT_MARK);			// as this a new node, set the mark to the current mark to avoid the dereferece by accident
                        }

                        AddNewItemInfo (hashCode,emptyLoc);	// update info
                        if(!TableAddressList[partNo][unbaisedLoc].getNode().isEqual(node)){
                            cout << "hash : find or add : node doesn't stored correctly" << endl;
                            exit(-1);
                        }
                        if(!TableAddressList[partNo][unbaisedLoc].getNode().isEqual(node)){
                            cout << "hash : find or add : node doesn't stored correctly" << endl;
                            cout << "Original node: node intended to be stored :" << endl;
                            node.DumpToScreen();
                            cout << "stored node :" << endl;
                            TableAddressList[partNo][unbaisedLoc].getNode().DumpToScreen();
                            cout << "bucket infos" << endl;
                            TableAddressList[partNo][unbaisedLoc].DumpToScreen();
                            exit(-1);
                        }
                        if( (hashString ==  134730432) /* || (emptyLoc == 4096)*/ ){
                            cout << "hash: find or add :: node with hashstring : " << hashString << " and hash code : " << hashCode << " will be added at location : " << emptyLoc << endl;
                            cout << "pointer :" << endl;
                            bp.DumpToScreen();
                            cout << endl << endl << "node" << endl;
                            node.DumpToScreen();
                            cout << endl << endl;
                            cout << "dump of emptyLoc" << endl;
                            TableAddressList[partNo][unbaisedLoc].DumpToScreen();
                            cout << endl << endl << "dump of hashCode" << endl;
                            partNo1 = getPartOfTable(hashCode,threshold1);
                            unbaisedLoc1  = hashCode - threshold1;
                            TableAddressList[partNo1][unbaisedLoc1].DumpToScreen();
                        }

                        returnFlag = false;
                    }
                    else {
                        TableAddressList[partNo][unbaisedLoc].clearAllFlags();	// this location is empty, as you make a bubble within the hash table, you have to make it as empty , so that it can be utilized latter
                        if(seg1 != seg){
                            Segments[seg1].Unlock();		// aquire read lock in this segment
                        }
                        Segments[seg].Unlock();		// aquire read lock in this segment
                        if (resizeFlag){
                            MasterResizeLock.Unlock();	// unlock the resizing lock
                            resize(CurrentNoOfResizing);
                        }
                        else
                            MasterResizeLock.Unlock();	// unlock the resizing lock
                        continue;
                    }
                }
                else { // the data is already exist
                    returnFlag = true;
                    TableAddressList[partNo][unbaisedLoc].clearAllFlags();	// this location is empty, as you make a bubble within the hash table, you have to make it as empty , so that it can be utilized latter
                }

                if(lastPartFlag)    // if the marginal part is locked
                    MasterResizeLock.Unlock();

                if(seg1 != seg){
                    Segments[seg1].Unlock();		// aquire read lock in this segment
                }
                Segments[seg].Unlock();		// aquire read lock in this segment
                MasterResizeLock.CheckConsistancy();
                return returnFlag;
            }
            while(true);	// repeat for ever as long as resizing is performed
        }
    }

    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // get the corresponding bdd node for pointer bp (getInfo, contains)
    //------------------------------------------------------------------------------------------------------------------------------------------------------------

// THIS FUNCTION RETURN NODE CORRESPONDING TO CERTAIN POINTER, it doesn't remove the node (it just read it) (it will be more similar to contain the second version)
    bool getInfo (BDDPointer <T,V> & bp, BDDNode  <T,V,L> &node) {
        bool datedMarkNodeFlag;

        if(ResizeType == H_RESIZE_TYPE_SWAP){
                T hashCode;
                hashCode = getHashCode(bp.getHashString());	// get hash code stored in the pointer

                // get the bucket
                T hopinfo = Table[hashCode].getHopInfo();	// get its hop information
                T nextNeighbor;

                while ( getNearestNeighbor(hashCode, hopinfo, nextNeighbor) ){	// get the nearest neighbor
                        if (GCEnabled == H_GC_MARK_ENABLED)
                                datedMarkNodeFlag = ( Table[nextNeighbor].getReferenceCounter() != CURRENT_MARK);	// if MARK is updated, then rise flag, else, it will be false
                        else if (GCEnabled == H_GC_INCREMENTAL_MARK_ENABLED)
                                datedMarkNodeFlag = ( Table[nextNeighbor].getReferenceCounter() != CURRENT_MARK) && ( Table[nextNeighbor].getReferenceCounter() != PERMENANT_NODE);	// if MARK is updated or permenant, then rise flag, else, it will be false
                        else
                                datedMarkNodeFlag = false ;


                        // the fetched node MUST NOT be empty or dated one
                        if ( (!datedMarkNodeFlag) && (Table[nextNeighbor].getLocalId() == bp.getLocalId()) && (!Table[nextNeighbor].isEmpty()) ){	// if you found the match,  store the pointer unlock the read lock then return true
                                node = Table[nextNeighbor].getNode();	// store the id in the pointer
                                return true;
                        }
                }
                // if you didn't find a match; then unlock the read lock then return false
                cout << "can't find the node: hash, getInfo" << endl; // this is an impossible situation if every thing is all right
                cout << "No. of lookup Tables : " << (T) NoOfLookUpTables << endl;
                cout << "Total Capacity: " << TotalCapacity << endl;
                bp.DumpToScreen();
                Dump();
                exit(-1);
                return false;
        }
        else{
            T hashString = bp.getHashString();	// compute hash String
            T hashCode = getHashCode(hashString);	// compute hash code
            T threshold;
            unsigned char partNo = getPartOfTable(hashCode,threshold);
            T unbaisedLoc  = hashCode - threshold;
            T hopinfo = TableAddressList[partNo][unbaisedLoc].getHopInfo();	// get its hop information
            T nextNeighbor;
            while ( getNearestNeighbor(hashCode, hopinfo, nextNeighbor) ){	// get the nearest neighbor
                partNo = getPartOfTable(nextNeighbor,threshold);
                unbaisedLoc  = nextNeighbor - threshold;
                if (GCEnabled == H_GC_MARK_ENABLED)
                    datedMarkNodeFlag = ( TableAddressList[partNo][unbaisedLoc].getReferenceCounter() != CURRENT_MARK);	// if MARK is updated, then rise flag, else, it will be false
                else if (GCEnabled == H_GC_INCREMENTAL_MARK_ENABLED)
                    datedMarkNodeFlag = ( TableAddressList[partNo][unbaisedLoc].getReferenceCounter() != CURRENT_MARK) && ( TableAddressList[partNo][unbaisedLoc].getReferenceCounter() != PERMENANT_NODE);	// if MARK is updated or permenant, then rise flag, else, it will be false
                else
                    datedMarkNodeFlag = false ;

                // the fetched node MUST NOT be empty or dated one
                if ( (!datedMarkNodeFlag) && (TableAddressList[partNo][unbaisedLoc].getLocalId() == bp.getLocalId()) && (!TableAddressList[partNo][unbaisedLoc].isEmpty()) ){	// if you found the match,  store the pointer unlock the read lock then return true
                    node = TableAddressList[partNo][unbaisedLoc].getNode();	// store the id in the pointer
                    return true;
                }
            }
            // if you didn't find a match; then unlock the read lock then return false
            cout << "can't find the node: hash, getInfo" << endl; // this is an impossible situation if every thing is all right
            cout << "No. of lookup Tables : " << (T) NoOfLookUpTables << endl;
            cout << "Total Capacity: " << TotalCapacity << endl;
            bp.DumpToScreen();
    //	Dump();
            exit(-1);
            return false;
        }


    }
// THIS FUNCTION RETURN NODE CORRESPONDING TO CERTAIN POINTER, it doesn't remove the node (it just read it) (it will be more similar to contain the second version) // FINISHED
    bool contains(BDDNode <T,V,L> node, BDDPointer <T,V> &bp) {
        T hashString = getHashString(node);	// compute hash String
        T hashCode = getHashCode(hashString);	// compute hash code
        if(ResizeType == H_RESIZE_TYPE_SWAP){
            T hopinfo = Table[hashCode].getHopInfo();	// get its hop information
            T nextNeighbor;
            bool datedMarkNodeFlag;
            while ( getNearestNeighbor(hashCode, hopinfo, nextNeighbor) ){	// get the nearest neighbor
                    // the fetched node MUST NOT be empty or dated one
                    if (GCEnabled == H_GC_MARK_ENABLED)
                            datedMarkNodeFlag = ( Table[nextNeighbor].getReferenceCounter() != CURRENT_MARK);	// if MARK is updated, then rise flag, else, it will be false
                    else if (GCEnabled == H_GC_INCREMENTAL_MARK_ENABLED)
                            datedMarkNodeFlag = ( Table[nextNeighbor].getReferenceCounter() != CURRENT_MARK) && ( Table[nextNeighbor].getReferenceCounter() != PERMENANT_NODE);	// if MARK is updated or permenant, then rise flag, else, it will be false
                    else
                            datedMarkNodeFlag = false ;

                    if ( (!datedMarkNodeFlag) && (Table[nextNeighbor].getNode().isEqual(node)) && (!Table[nextNeighbor].isEmpty()) ){	// if you found the match,  store the pointer unlock the read lock then return true
                            bp.setLocalId(Table[nextNeighbor].getLocalId());				// get local id
                            bp.setHashString(getHashString(Table[nextNeighbor].getNode()));	// get hash string for this node
                            bp.clearTemporary();
                            bp.ClearInverted();
                            return true;
                    }
            }

            cout << "can't find the node: hash, contains" << endl; // this is an impossible situation if every thing is all right
    //        DumpHashTable();
            exit(-1);
            return false;
        }
        else{
                T threshold;
                unsigned char partNo = getPartOfTable(hashCode,threshold);
                T unbaisedLoc  = hashCode - threshold;
                T hopinfo = TableAddressList[partNo][unbaisedLoc].getHopInfo();	// get its hop information
                T nextNeighbor;
                bool datedMarkNodeFlag;
                while ( getNearestNeighbor(hashCode, hopinfo, nextNeighbor) ){	// get the nearest neighbor
                        // the fetched node MUST NOT be empty or dated one
                        partNo = getPartOfTable(nextNeighbor,threshold);
                        unbaisedLoc  = nextNeighbor - threshold;
                        if (GCEnabled == H_GC_MARK_ENABLED)
                                datedMarkNodeFlag = ( TableAddressList[partNo][unbaisedLoc].getReferenceCounter() != CURRENT_MARK);	// if MARK is updated, then rise flag, else, it will be false
                        else if (GCEnabled == H_GC_INCREMENTAL_MARK_ENABLED)
                                datedMarkNodeFlag = ( TableAddressList[partNo][unbaisedLoc].getReferenceCounter() != CURRENT_MARK) && ( TableAddressList[partNo][unbaisedLoc].getReferenceCounter() != PERMENANT_NODE);	// if MARK is updated or permenant, then rise flag, else, it will be false
                        else
                                datedMarkNodeFlag = false ;

                        if ( (!datedMarkNodeFlag) && (TableAddressList[partNo][unbaisedLoc].getNode().isEqual(node)) && (!TableAddressList[partNo][unbaisedLoc].isEmpty()) ){	// if you found the match,  store the pointer unlock the read lock then return true
                                bp.setLocalId(TableAddressList[partNo][unbaisedLoc].getLocalId());				// get local id
                                bp.setHashString(getHashString(TableAddressList[partNo][unbaisedLoc].getNode()));	// get hash string for this node
                                bp.clearTemporary();
                                bp.ClearInverted();
                                return true;
                        }
                }

            cout << "can't find the node: hash, contains" << endl; // this is an impossible situation if every thing is all right
         //   DumpHashTable();
            exit(-1);
            return false;
        }
    }

    // check wether the pointer still in hash table or not, it is used in hashQueue (contain without locking function), when teh computed table is enabled
    bool contains (BDDPointer <T,V> & bp) {
/*	if (bp.getPointer() == 0x24000005){
                cout << "Hash: getInfo : node with id " << bp.getPointer() << " will be marked. "<< endl;
        }*/

        if(ResizeType == H_RESIZE_TYPE_SWAP){
            T hashString = bp.getHashString();	// compute hash String
            T hashCode = getHashCode(hashString);	// compute hash code

            T hopinfo = Table[hashCode].getHopInfo();	// get its hop information
            T nextNeighbor;
            while ( getNearestNeighbor(hashCode, hopinfo, nextNeighbor) ){	// get the nearest neighbor

                // the fetched node MUST NOT be empty or dated one
                bool datedMarkNodeFlag;
                if (GCEnabled == H_GC_MARK_ENABLED)
                        datedMarkNodeFlag = ( Table[nextNeighbor].getReferenceCounter() != CURRENT_MARK);	// if MARK is updated, then rise flag, else, it will be false
                else if (GCEnabled == H_GC_INCREMENTAL_MARK_ENABLED)
                        datedMarkNodeFlag = ( Table[nextNeighbor].getReferenceCounter() != CURRENT_MARK) && ( Table[nextNeighbor].getReferenceCounter() != PERMENANT_NODE);	// if MARK is updated or permenant, then rise flag, else, it will be false
                else
                        datedMarkNodeFlag = false ;

                if ( (!datedMarkNodeFlag) && (bp.getLocalId() == Table[nextNeighbor].getLocalId()) && (!Table[nextNeighbor].isEmpty()) ){	// if you found the match,  store the pointer unlock the read lock then return true
                        return true;
                }
            }
            return false;
        }
        else{
            T hashString = bp.getHashString();	// compute hash String
            T hashCode = getHashCode(hashString);	// compute hash code

            T threshold;
            unsigned char partNo = getPartOfTable(hashCode,threshold);
            T unbaisedLoc  = hashCode - threshold;
            T hopinfo = TableAddressList[partNo][unbaisedLoc].getHopInfo();	// get its hop information
            T nextNeighbor;
            while ( getNearestNeighbor(hashCode, hopinfo, nextNeighbor) ){	// get the nearest neighbor
                partNo = getPartOfTable(nextNeighbor,threshold);
                unbaisedLoc  = nextNeighbor - threshold;
                // the fetched node MUST NOT be empty or dated one
                bool datedMarkNodeFlag;
                if (GCEnabled == H_GC_MARK_ENABLED)
                    datedMarkNodeFlag = ( TableAddressList[partNo][unbaisedLoc].getReferenceCounter() != CURRENT_MARK);	// if MARK is updated, then rise flag, else, it will be false
                else if (GCEnabled == H_GC_INCREMENTAL_MARK_ENABLED)
                    datedMarkNodeFlag = ( TableAddressList[partNo][unbaisedLoc].getReferenceCounter() != CURRENT_MARK) && ( TableAddressList[partNo][unbaisedLoc].getReferenceCounter() != PERMENANT_NODE);	// if MARK is updated or permenant, then rise flag, else, it will be false
                else
                    datedMarkNodeFlag = false ;

                if ( (!datedMarkNodeFlag) && (bp.getLocalId() == TableAddressList[partNo][unbaisedLoc].getLocalId()) && (!TableAddressList[partNo][unbaisedLoc].isEmpty()) ){	// if you found the match,  store the pointer unlock the read lock then return true
                    return true;
                }
            }
            return false;
        }

    }
    // this function returns the level of the a node with pointer 'bp
    L getLevelFromID(BDDPointer <T,V> bp){
    /*	if (bp.getPointer() == 0x24000005){
                    cout << "Hash: getInfo : node with id " << bp.getPointer() << " will be marked. "<< endl;
            }*/

        if(ResizeType == H_RESIZE_TYPE_SWAP){
            T hashString = bp.getHashString();	// compute hash String
            T hashCode = getHashCode(hashString);	// compute hash code

            T hopinfo = Table[hashCode].getHopInfo();	// get its hop information
            T nextNeighbor;
            while ( getNearestNeighbor(hashCode, hopinfo, nextNeighbor) ){	// get the nearest neighbor

                // the fetched node MUST NOT be empty or dated one
                bool datedMarkNodeFlag;
                if (GCEnabled == H_GC_MARK_ENABLED)
                        datedMarkNodeFlag = ( Table[nextNeighbor].getReferenceCounter() != CURRENT_MARK);	// if MARK is updated, then rise flag, else, it will be false
                else if (GCEnabled == H_GC_INCREMENTAL_MARK_ENABLED)
                        datedMarkNodeFlag = ( Table[nextNeighbor].getReferenceCounter() != CURRENT_MARK) && ( Table[nextNeighbor].getReferenceCounter() != PERMENANT_NODE);	// if MARK is updated or permenant, then rise flag, else, it will be false
                else
                        datedMarkNodeFlag = false ;

                if ( (!datedMarkNodeFlag) && (bp.getLocalId() == Table[nextNeighbor].getLocalId()) && (!Table[nextNeighbor].isEmpty()) ){	// if you found the match,  store the pointer unlock the read lock then return true
                        return Table[nextNeighbor].getLevel();
                }
            }
            cout << "can't find the node: hash, getLevelFromID" << endl; // this is an impossible situation if every thing is all right
            cout << " hashCode : " << hashCode << endl;
            cout << " last nextNeighbor : " << nextNeighbor<< endl;
            bp.DumpToScreen();
         //   DumpHashTable();
            exit(-1);
        }
        else{
            T hashString = bp.getHashString();	// compute hash String
            T hashCode = getHashCode(hashString);	// compute hash code
            T threshold;
            unsigned char partNo = getPartOfTable(hashCode,threshold);
            T unbaisedLoc  = hashCode - threshold;
            T hopinfo = TableAddressList[partNo][unbaisedLoc].getHopInfo();	// get its hop information
            T nextNeighbor;
            while ( getNearestNeighbor(hashCode, hopinfo, nextNeighbor) ){	// get the nearest neighbor
                partNo = getPartOfTable(nextNeighbor,threshold);
                unbaisedLoc  = nextNeighbor - threshold;
                // the fetched node MUST NOT be empty or dated one
                bool datedMarkNodeFlag;
                if (GCEnabled == H_GC_MARK_ENABLED)
                    datedMarkNodeFlag = ( TableAddressList[partNo][unbaisedLoc].getReferenceCounter() != CURRENT_MARK);	// if MARK is updated, then rise flag, else, it will be false
                else if (GCEnabled == H_GC_INCREMENTAL_MARK_ENABLED)
                    datedMarkNodeFlag = ( TableAddressList[partNo][unbaisedLoc].getReferenceCounter() != CURRENT_MARK) && ( TableAddressList[partNo][unbaisedLoc].getReferenceCounter() != PERMENANT_NODE);	// if MARK is updated or permenant, then rise flag, else, it will be false
                else
                    datedMarkNodeFlag = false;
                if ( (!datedMarkNodeFlag) && (bp.getLocalId() == TableAddressList[partNo][unbaisedLoc].getLocalId()) && (!TableAddressList[partNo][unbaisedLoc].isEmpty()) ){	// if you found the match,  store the pointer unlock the read lock then return true
                    return TableAddressList[partNo][unbaisedLoc].getLevel();
                }
            }
            cout << "can't find the node: hash, getLevelFromID" << endl; // this is an impossible situation if every thing is all right
            cout << " hashCode : " << hashCode << endl;
            cout << " last nextNeighbor : " << nextNeighbor<< endl;
            bp.DumpToScreen();
         //   DumpHashTable();
            exit(-1);
        }
    }
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // GC related functions (RC, and mark)
    //------------------------------------------------------------------------------------------------------------------------------------------------------------

    // update mark of node with pointer 'bp' and its children (recursively),
    void UpdateMark(BDDPointer <T,V> bp, bool PermenantFlag){
            // THE FOLLOWING CONDITION SHOULD NEVER BEEN HAPPENS
            if (bp.isTemporary()){	// is the pointer is temporary, then fire an error message
                    cout << "hash, UpdateMark : temporary pointer" << endl; // this is an impossible situation if every thing is all right
                    cout << "Total Capacity: " << TotalCapacity << endl;
                    bp.DumpToScreen();
                    exit(-1);
                    return ;
            }
            if (bp.isInverted())	// is the pointer is inverted, then regulate it.
                    bp = bp.invert();

/*		if (bp.getPointer() == 0x2200002a){
                    cout << "Hash: UpdateMark : node with id " << bp.getPointer() << " will be marked as " << PermenantFlag << endl;
            }*/

            T hashString = bp.getHashString();	// compute hash String
            T hashCode = getHashCode(hashString);	// compute hash code


            if(ResizeType == H_RESIZE_TYPE_SWAP){
    //		mutex.lock();
                    Bucket <T,V,L>tempBucket = Table[hashCode];	// get the bucket
                    T hopinfo = tempBucket.getHopInfo();	// get its hop information
                    T nextNeighbor;
                    while ( getNearestNeighbor(hashCode, hopinfo, nextNeighbor) ){	// get the nearest neighbor
                            tempBucket = Table[nextNeighbor];

                            if ((bp.getLocalId() == tempBucket.getLocalId()) && (!tempBucket.isEmpty()) ){	// if you found the match,  store the pointer unlock the read lock then return true
                                    if (GCEnabled == H_GC_MARK_ENABLED){	// for ordinary mark GC
                                            if( (tempBucket.getReferenceCounter () == CURRENT_MARK)){	// note that we use RC field in 'bucket' as 'mark' field. (the location is the same, but the meaning is different from RC and MARK GC)
            //					mutex.unlock();
                                                    return;	// if the current mark of the node is up-to-date, then return (end case)
                                            }
                                            else if (One.isEqual(bp)){	// if this the terminal node, and it's 'mark' is not up-to-date, then update it, and return.
            //					NodeCounter++;
                                            /*	if ( (hashIntegrityCheck == IC_VALIDATE_HASH_DURING_GC) ) {	// if GC map new node if it is exist.
                                                            if(!integrityChecker ->CheckIfDeadNode(bp)){	// if it is not exist in the GCed raw table
                                                                    cout << "Hash : UpdateRC : The node will be marked wrongly 1" << endl;
                                                                    Table[nextNeighbor].DumpToScreen();
                                                                    exit(-1);
                                                            }
                                                    }*/
                                                    Table[nextNeighbor].setReferenceCounter (CURRENT_MARK);
            //					mutex.unlock();
                                                    return;
                                            }
                                            // mark this node, then update the 'else' and 'then' pointers recursively
            //				NodeCounter++;
                            /*		if ( (hashIntegrityCheck == IC_VALIDATE_HASH_DURING_GC) ) {	// if GC map new node if it is exist.
                                                    if(!integrityChecker ->CheckIfDeadNode(bp)){	// if it is not exist in the GCed raw table
                                                            cout << "Hash : UpdateRC : The node will be marked wrongly 2" << endl;
                                                            Table[nextNeighbor].DumpToScreen();
                                                            exit(-1);
                                                    }
                                            }*/

                                            Table[nextNeighbor].setReferenceCounter (CURRENT_MARK);
                                    }
                                    else if (GCEnabled == H_GC_INCREMENTAL_MARK_ENABLED){	// for incremental mark GC
                                            if(PermenantFlag){	// if it is a permenant node (happens in incremental GC)
                                                    if( (tempBucket.getReferenceCounter () == PERMENANT_NODE)){	// note that we use RC field in 'bucket' as 'mark' field. (the location is the same, but the meaning is different from RC and MARK GC)
                    //					mutex.unlock();
                                                            return;	// if the current mark of the node is up-to-date, then return (end case)
                                                    }
                                                    else if (One.isEqual(bp)){	// if this the terminal node, and it's 'mark' is not up-to-date, then update it, and return.
                    //					NodeCounter++;
                                                    /*	if ( (hashIntegrityCheck == IC_VALIDATE_HASH_DURING_GC) ) {	// if GC map new node if it is exist.
                                                                    if(!integrityChecker ->CheckIfDeadNode(bp)){	// if it is not exist in the GCed raw table
                                                                            cout << "Hash : UpdateRC : The node will be marked wrongly 3" << endl;
                                                                            Table[nextNeighbor].DumpToScreen();
                                                                            exit(-1);
                                                                    }
                                                            }*/
                                                            Table[nextNeighbor].setReferenceCounter (PERMENANT_NODE);
                    //					mutex.unlock();
                                                            return;
                                                    }
                                                    // mark this node, then update the 'else' and 'then' pointers recursively
                    //				NodeCounter++;
                                    /*		if ( (hashIntegrityCheck == IC_VALIDATE_HASH_DURING_GC) ) {	// if GC map new node if it is exist.
                                                            if(!integrityChecker ->CheckIfDeadNode(bp)){	// if it is not exist in the GCed raw table
                                                                    cout << "Hash : UpdateRC : The node will be marked wrongly 4" << endl;
                                                                    Table[nextNeighbor].DumpToScreen();
                                                                    exit(-1);
                                                            }
                                                    }*/
                                                    Table[nextNeighbor].setReferenceCounter (PERMENANT_NODE);
                                            }
                                            else { // if it is a temporary node (in incremental mark), or any node in ordinary mark GC
                                                    if( (tempBucket.getReferenceCounter () == CURRENT_MARK) || (tempBucket.getReferenceCounter () == PERMENANT_NODE)){	// note that we use RC field in 'bucket' as 'mark' field. (the location is the same, but the meaning is different from RC and MARK GC)
                    //					mutex.unlock();
                                                            return;	// if the current mark of the node is up-to-date, then return (end case)
                                                    }
                                                    else if (One.isEqual(bp)){	// if this the terminal node, and it's 'mark' is not up-to-date, then update it, and return.
                    //					NodeCounter++;
                    //					Table[nextNeighbor].setReferenceCounter (CURRENT_MARK);
                    //					mutex.unlock();
                                                            return;
                                                    }
                                                    // mark this node, then update the 'else' and 'then' pointers recursively
                    //				NodeCounter++;
                                    /*		if ( (hashIntegrityCheck == IC_VALIDATE_HASH_DURING_GC) ) {	// if GC map new node if it is exist.
                                                            if(!integrityChecker ->CheckIfDeadNode(bp)){	// if it is not exist in the GCed raw table
                                                                    cout << "Hash : UpdateRC : The node will be marked wrongly 4" << endl;
                                                                    Table[nextNeighbor].DumpToScreen();
                                                                    exit(-1);
                                                            }
                                                    }*/
                                                    Table[nextNeighbor].setReferenceCounter (CURRENT_MARK);
                                            }
                                    }

                                    // here the 'mark' is not updated, and the node is not terminal, so we recursively mark the children of this node
                                    BDDPointer  <T,V> Then = tempBucket.getThen();	// get 'then'
                                    BDDPointer  <T,V> Else = tempBucket.getElse();	// get 'else
    //				mutex.unlock();

                                    UpdateMark(Then,PermenantFlag);	// mark 'then'
                                    UpdateMark(Else,PermenantFlag);	// mark 'else'
                                    return;
                            }
                    }

                    cout << "can't find the node: hash, UpdateMark" << endl; // this is an impossible situation if every thing is all right
                    cout << "Total Capacity: " << TotalCapacity << endl;
                    bp.DumpToScreen();
                    exit(-1);
                    return ;
            }
            else{
    //		mutex.lock();
                    T threshold;
                    unsigned char partNo = getPartOfTable(hashCode,threshold);
                    T unbaisedLoc  = hashCode - threshold;
                    Bucket  <T,V,L> tempBucket = TableAddressList[partNo][unbaisedLoc];	// get the bucket
                    T hopinfo = tempBucket.getHopInfo();	// get its hop information
                    T nextNeighbor;
                    while ( getNearestNeighbor(hashCode, hopinfo, nextNeighbor) ){	// get the nearest neighbor
                            partNo = getPartOfTable(nextNeighbor,threshold);
                            unbaisedLoc  = nextNeighbor - threshold;
                            tempBucket = TableAddressList[partNo][unbaisedLoc];

                            if ((bp.getLocalId() == tempBucket.getLocalId()) && (!tempBucket.isEmpty()) ){	// if you found the match,  store the pointer unlock the read lock then return true
                                    if (GCEnabled == H_GC_MARK_ENABLED){	// for ordinary mark GC
                                            if( (tempBucket.getReferenceCounter () == CURRENT_MARK)){	// note that we use RC field in 'bucket' as 'mark' field. (the location is the same, but the meaning is different from RC and MARK GC)
            //					mutex.unlock();
                                                    return;	// if the current mark of the node is up-to-date, then return (end case)
                                            }
                                            else if (One.isEqual(bp)){	// if this the terminal node, and it's 'mark' is not up-to-date, then update it, and return.
            //					NodeCounter++;
                            /*			if ( (hashIntegrityCheck == IC_VALIDATE_HASH_DURING_GC) ) {	// if GC map new node if it is exist.
                                                            if(!integrityChecker ->CheckIfDeadNode(bp)){	// if it is not exist in the GCed raw TableAddressList[partNo]
                                                                    cout << "Hash : UpdateRC : The node will be marked wrongly 1" << endl;
                                                                    TableAddressList[partNo][unbaisedLoc].DumpToScreen();
                                                                    exit(-1);
                                                            }
                                                    }*/
                                                    TableAddressList[partNo][unbaisedLoc].setReferenceCounter (CURRENT_MARK);
            //					mutex.unlock();
                                                    return;
                                            }
                                            // mark this node, then update the 'else' and 'then' pointers recursively
            //				NodeCounter++;
                    /*			if ( (hashIntegrityCheck == IC_VALIDATE_HASH_DURING_GC) ) {	// if GC map new node if it is exist.
                                                    if(!integrityChecker ->CheckIfDeadNode(bp)){	// if it is not exist in the GCed raw TableAddressList[partNo]
                                                            cout << "Hash : UpdateRC : The node will be marked wrongly 2" << endl;
                                                            TableAddressList[partNo][unbaisedLoc].DumpToScreen();
                                                            exit(-1);
                                                    }
                                            }*/

                                            TableAddressList[partNo][unbaisedLoc].setReferenceCounter (CURRENT_MARK);
                                    }
                                    else if (GCEnabled == H_GC_INCREMENTAL_MARK_ENABLED){	// for incremental mark GC
                                            if(PermenantFlag){	// if it is a permenant node (happens in incremental GC)
                                                    if( (tempBucket.getReferenceCounter () == PERMENANT_NODE)){	// note that we use RC field in 'bucket' as 'mark' field. (the location is the same, but the meaning is different from RC and MARK GC)
                    //					mutex.unlock();
                                                            return;	// if the current mark of the node is up-to-date, then return (end case)
                                                    }
                                                    else if (One.isEqual(bp)){	// if this the terminal node, and it's 'mark' is not up-to-date, then update it, and return.
                    //					NodeCounter++;
                            /*				if ( (hashIntegrityCheck == IC_VALIDATE_HASH_DURING_GC) ) {	// if GC map new node if it is exist.
                                                                    if(!integrityChecker ->CheckIfDeadNode(bp)){	// if it is not exist in the GCed raw table
                                                                            cout << "Hash : UpdateRC : The node will be marked wrongly 3" << endl;
                                                                            TableAddressList[partNo][unbaisedLoc].DumpToScreen();
                                                                            exit(-1);
                                                                    }
                                                            }*/
                                                            TableAddressList[partNo][unbaisedLoc].setReferenceCounter (PERMENANT_NODE);
                    //					mutex.unlock();
                                                            return;
                                                    }
                                                    // mark this node, then update the 'else' and 'then' pointers recursively
                    //				NodeCounter++;
                            /*			if ( (hashIntegrityCheck == IC_VALIDATE_HASH_DURING_GC) ) {	// if GC map new node if it is exist.
                                                            if(!integrityChecker ->CheckIfDeadNode(bp)){	// if it is not exist in the GCed raw table
                                                                    cout << "Hash : UpdateRC : The node will be marked wrongly 4" << endl;
                                                                    TableAddressList[partNo][unbaisedLoc].DumpToScreen();
                                                                    exit(-1);
                                                            }
                                                    }*/
                                                    TableAddressList[partNo][unbaisedLoc].setReferenceCounter (PERMENANT_NODE);
                                            }
                                            else { // if it is a temporary node (in incremental mark), or any node in ordinary mark GC
                                                    if( (tempBucket.getReferenceCounter () == CURRENT_MARK) || (tempBucket.getReferenceCounter () == PERMENANT_NODE)){	// note that we use RC field in 'bucket' as 'mark' field. (the location is the same, but the meaning is different from RC and MARK GC)
                    //					mutex.unlock();
                                                            return;	// if the current mark of the node is up-to-date, then return (end case)
                                                    }
                                                    else if (One.isEqual(bp)){	// if this the terminal node, and it's 'mark' is not up-to-date, then update it, and return.
                    //					NodeCounter++;
                    //					TableAddressList[partNo][unbaisedLoc].setReferenceCounter (CURRENT_MARK);
                    //					mutex.unlock();
                                                            return;
                                                    }
                                                    // mark this node, then update the 'else' and 'then' pointers recursively
                    //				NodeCounter++;
                            /*			if ( (hashIntegrityCheck == IC_VALIDATE_HASH_DURING_GC) ) {	// if GC map new node if it is exist.
                                                            if(!integrityChecker ->CheckIfDeadNode(bp)){	// if it is not exist in the GCed raw table
                                                                    cout << "Hash : UpdateRC : The node will be marked wrongly 4" << endl;
                                                                    TableAddressList[partNo][unbaisedLoc].DumpToScreen();
                                                                    exit(-1);
                                                            }
                                                    }*/
                                                    TableAddressList[partNo][unbaisedLoc].setReferenceCounter (CURRENT_MARK);
                                            }
                                    }

                                    // here the 'mark' is not updated, and the node is not terminal, so we recursively mark the children of this node
                                    BDDPointer <T,V> Then = tempBucket.getThen();	// get 'then'
                                    BDDPointer <T,V> Else = tempBucket.getElse();	// get 'else
    //				mutex.unlock();

                                    UpdateMark(Then,PermenantFlag);	// mark 'then'
                                    UpdateMark(Else,PermenantFlag);	// mark 'else'
                                    return;
                            }
                    }
                    cout << "can't find the node: hash, UpdateMark" << endl; // this is an impossible situation if every thing is all right
                    cout << "Total Capacity: " << TotalCapacity << endl;
                    bp.DumpToScreen();
                    exit(-1);
                    return ;
            }
    }

    // this function used to update RC for a node (this function called in GCThread). this function contains no lock (as there is no add operaton, so the bucket are indpendent) execept when update teh hopinfo (as there are different node share teh same bucket hopinfo, and hence it need to be protected from data race).
    void  UpdateRC(GCHashNode <T,V> & gcNode){
            int count = gcNode.getCount(); // get the incremental count
            int count1 = count;
            BDDPointer <T,V> bp = gcNode.getBP(); // get pointer

    /*	if (bp.getPointer() == 0x23001fc3){
                    cout << "PRINTING POINTER" << endl;

            }*/

            T hashString = bp.getHashString();	// compute hash String
            T hashCode = getHashCode(hashString);	// compute hash code

            if(ResizeType == H_RESIZE_TYPE_SWAP){
                    unsigned short finalSeg = getSegment(hashCode);
                    Segments[finalSeg].ReadLock();
                    Bucket <T,V,L>  tempBucket = Table[hashCode];	// get the bucket
                    T hopinfo = tempBucket.getHopInfo();	// get its hop information
                    Segments[finalSeg].Unlock();

                    T nextNeighbor;
                    while ( getNearestNeighbor(hashCode, hopinfo, nextNeighbor) ){	// get the nearest neighbor
                            finalSeg = getSegment(nextNeighbor);
                            Segments[finalSeg].ReadLock();

                            if ((bp.getLocalId() == Table[nextNeighbor].getLocalId()) && (!Table[nextNeighbor].isEmpty()) ){	// if you found the match,
                                    Segments[finalSeg].Unlock();
                                    Segments[finalSeg].WriteLock();
                                    count = count + Table[nextNeighbor].getReferenceCounter();		// update count
                                    Table[nextNeighbor].setReferenceCounter(count);		// set new count
                                    Segments[finalSeg].Unlock();
                            /*	if (bp.getPointer() == 0x17000023){
                                            cout << "Hash: UpdateRC : node with id " << bp.getPointer() << " has reference count : " << count << endl;
                                            Table[nextNeighbor].DumpToScreen();
                                    }*/
                                    if(count == 0){	// if this a dead node, then dereference 'then' and 'else' pointers
                                    /*	cout<<"node is removed : UpdateRC : hash"<< endl;
                                            cout << "count: " << count1 << endl;
                                            bp.DumpToScreen();*/
                            /*		if (bp.getPointer() == 0xA0000DF){
                                                    cout << "Hash: UpdateRC : node with id " << bp.getPointer() << " will be Dereferenced" << endl;

                                            }*/
                            //		cout << "Hash: UpdateRC : node with id " << bp.getPointer() << " will be Dereferenced" << endl;

                            /*		if ( (hashIntegrityCheck == IC_VALIDATE_HASH_DURING_GC) ) {	// if GC map new node if it is exist.
                                                    if(integrityChecker ->CheckIfDeadNode(bp)){
                                                            cout << "Hash : UpdateRC : The node will be removed wrongly" << endl;
                                                            bp.DumpToScreen();
                                                            exit(-1);
                                                    }
                                            }*/

                                            mutexFile.lock();
                                            NodeCounter--;
                                            mutexFile.unlock();


                                            GCHashNode <T,V> gcNode1;
                                            gcNode1.setBP(Table[nextNeighbor].getThen());
                                            gcNode1.setCount(-1);
                                            UpdateRC(gcNode1);

                                            bp = Table[nextNeighbor].getElse();
                                            if (bp.isInverted())	// is the pointer is inverted, then regulate it.
                                                    bp = bp.invert();
                                            gcNode1.setBP(bp);
                                            UpdateRC(gcNode1);

                                            Table[nextNeighbor].clearAllFlags();		// clear all flags, note that you don't have to clear RC, as it is stored previous in 'count'
                                            // update hopinfo, we will use lock as we have the possibility of having more than one node sharing the same hopinfo
                                            unsigned short finalSeg = getSegment(hashCode);
                                            Segments[finalSeg].WriteLock();
                                            removeItemInfo(hashCode,nextNeighbor);
                                            Segments[finalSeg].Unlock();
                                    }
                                    return;
                            }
                            Segments[finalSeg].Unlock();
                    }

                    cout << "can't find the node: hash, UpdateRC" << endl; // this is an impossible situation if every thing is all right
                    cout << "count: " << count1 << endl;
                    bp.DumpToScreen();
                    exit(-1);
                    return ;
            }
            else{
                    unsigned short finalSeg = getSegment(hashCode);
                    Segments[finalSeg].ReadLock();
                    T threshold;
                    unsigned char partNo = getPartOfTable(hashCode,threshold);
                    T unbaisedLoc  = hashCode - threshold;
                    Bucket <T,V,L> tempBucket = TableAddressList[partNo][unbaisedLoc];	// get the bucket
                    T hopinfo = tempBucket.getHopInfo();	// get its hop information
                    Segments[finalSeg].Unlock();

                    T nextNeighbor;
                    while ( getNearestNeighbor(hashCode, hopinfo, nextNeighbor) ){	// get the nearest neighbor
                            finalSeg = getSegment(nextNeighbor);
                            Segments[finalSeg].ReadLock();
                            partNo = getPartOfTable(nextNeighbor,threshold);
                            unbaisedLoc  = nextNeighbor - threshold;

                            if ((bp.getLocalId() == TableAddressList[partNo][unbaisedLoc].getLocalId() ) && (!TableAddressList[partNo][unbaisedLoc].isEmpty()) ){	// if you found the match,
                                    Segments[finalSeg].Unlock();
                                    Segments[finalSeg].WriteLock();
                                    count = count + TableAddressList[partNo][unbaisedLoc].getReferenceCounter();		// update count
                                    TableAddressList[partNo][unbaisedLoc].setReferenceCounter(count);		// set new count
                                    Segments[finalSeg].Unlock();
                            /*	if (bp.getPointer() == 0x17000023){
                                            cout << "Hash: UpdateRC : node with id " << bp.getPointer() << " has reference count : " << count << endl;
                                            TableAddressList[partNo][unbaisedLoc].DumpToScreen();
                                    }*/
                                    if(count == 0){	// if this a dead node, then dereference 'then' and 'else' pointers
                                    /*	cout<<"node is removed : UpdateRC : hash"<< endl;
                                            cout << "count: " << count1 << endl;
                                            bp.DumpToScreen();*/
                    /*			if (bp.getPointer() == 0xA0000DF){
                                                    cout << "Hash: UpdateRC : node with id " << bp.getPointer() << " will be Dereferenced" << endl;

                                            }*/
                            //		cout << "Hash: UpdateRC : node with id " << bp.getPointer() << " will be Dereferenced" << endl;

                            /*		if ( (hashIntegrityCheck == IC_VALIDATE_HASH_DURING_GC) ) {	// if GC map new node if it is exist.
                                                    if(integrityChecker ->CheckIfDeadNode(bp)){
                                                            cout << "Hash : UpdateRC : The node will be removed wrongly" << endl;
                                                            bp.DumpToScreen();
                                                            exit(-1);
                                                    }
                                            }*/

                                            mutexFile.lock();
                                            NodeCounter--;
                                            mutexFile.unlock();


                                            GCHashNode <T,V> gcNode1;
                                            gcNode1.setBP(TableAddressList[partNo][unbaisedLoc].getThen());
                                            gcNode1.setCount(-1);
                                            UpdateRC(gcNode1);

                                            bp = TableAddressList[partNo][unbaisedLoc].getElse();
                                            if (bp.isInverted())	// is the pointer is inverted, then regulate it.
                                                    bp = bp.invert();
                                            gcNode1.setBP(bp);
                                            UpdateRC(gcNode1);

                                            TableAddressList[partNo][unbaisedLoc].clearAllFlags();		// clear all flags, note that you don't have to clear RC, as it is stored previous in 'count'
                                            // update hopinfo, we will use lock as we have the possibility of having more than one node sharing the same hopinfo
                                            int finalSeg = getSegment(hashCode);
                                            Segments[finalSeg].WriteLock();
                                            removeItemInfo(hashCode,nextNeighbor);
                                            Segments[finalSeg].Unlock();
                                    }
                                    return;
                            }
                            Segments[finalSeg].Unlock();
                    }

                    cout << "can't find the node: hash, UpdateRC" << endl; // this is an impossible situation if every thing is all right
                    cout << "count: " << count1 << endl;
                    bp.DumpToScreen();
                    exit(-1);
                    return ;
            }
    }

    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // public methods for bdd validaiton (external interfaces for bdd simulation and validation )
    //------------------------------------------------------------------------------------------------------------------------------------------------------------

    // simulate A BDD for certain input
    char SimulateBDD(BDDPointer <T,V> bp, char * inputvec){
            int inv = 0; // flag to indicate whether the ouput will be inversed or not (0 : will not inversed, 1 : will inversed)
            // recursively traverse BDD to get the output
            char output = getDecision(bp,inputvec,inv);
            // if the output is 'don't care', just return it.
            if(output == 'X')
                    return output;

            if(inv == 0)	// if the value is regular, return it as it is
                    return output;
            else{		// if it is inverted, return the inverse (as the ouput is always '1' , so the inverse is always '0')
                    return invert(output);
            }
    }

    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // get hash internal parameters (for debugging and internal validation purposes)
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // get number of nodes
    T getNodeCount(){
            T temp;
            mutexFile.lock();
            temp = NodeCounter;
            mutexFile.unlock();
            return temp;
    }
    // dump hash table (called usually when something wrong happens for debugging purposes)
    void Dump() {
            char fName[40];
            mutexFile.lock();
            fileCounter++;
            sprintf(fName, "hash Dump%d.txt",fileCounter);	// set file name
            ofstream outClientFile( fName, ios::out );
            Dump(outClientFile);
            mutexFile.unlock();


    }

    // dump all information of hash table
    void Dump(ofstream & ff) {
            ff << "======================================================================="<< endl;
            ff << "   				Table Information			" << endl;
            ff << "Max. Utilization : " << MaxUtilization << endl;
            ff << "input Capacity : " << inCapacity << endl;
            ff << "Concurrency Degree : " << ConcurrencyDegree << endl;
            ff << "Bucket Size : " << BucketSize << endl;
            ff << "Segment Size : " << SegmentSize << endl;
            ff << "Total Capacity : " << TotalCapacity << endl;
            T i;
            ff << "======================================================================="<< endl;
            ff << "   				DATA					" << endl;
            ff << "======================================================================="<< endl;
            // dump all table
            unsigned short seg1, Seg;
            Seg = 0;
            Segments[Seg].ReadLock();		// aquire read lock in this segment
            for(i = 0; i < TotalCapacity; i++){
                    ff << "element no. " << i << endl;
                    seg1 = getSegment(i);

                    if (seg1 != Seg) {
                            Segments[Seg].Unlock();	// unlock the older segment
                            Seg = seg1;
                            Segments[Seg].ReadLock();		// aquire read lock in this segment
                    }


                    Table[i].Dump(ff);
            }
            Segments[Seg].Unlock();	// unlock the older segment
    }

    // dump hash table (unique table) to a file, for integrity purposes ...
    void DumpHashTable(){
        char fName[40];
        sprintf(fName, "Hash Table Dump.txt");	// set file name
        ofstream outClientFile( fName, ios::out );
        T i;

        outClientFile << TotalCapacity << endl;
        outClientFile << BucketMask << endl;

        for (i = 0; i < TotalCapacity; i++){
            if(ResizeType == H_RESIZE_TYPE_SWAP){
                Table[i].DumpToFile(outClientFile);
            }
            else{
                T threshold;
                unsigned char partNo = getPartOfTable(i,threshold);
                T unbaisedLoc  = i - threshold;
                TableAddressList[partNo][unbaisedLoc].DumpToFile(outClientFile);	// get the bucket
            }
        }
    }
    void DumpHashTable(int fileNo){
        char fName[40];
        sprintf(fName, "Hash Table Dump %d .txt",fileNo);	// set file name
        ofstream outClientFile( fName, ios::out );
        T i;

        outClientFile << TotalCapacity << endl;
        outClientFile << BucketMask << endl;

        for (i = 0; i < TotalCapacity; i++){
            if(ResizeType == H_RESIZE_TYPE_SWAP){
                Table[i].DumpToFile(outClientFile);
            }
            else{
                T threshold;
                unsigned char partNo = getPartOfTable(i,threshold);
                T unbaisedLoc  = i - threshold;
                TableAddressList[partNo][unbaisedLoc].DumpToFile(outClientFile);	// get the bucket
            }
        }
    }
    void DumpTotalCapacity(){
        char fName[40];
        sprintf(fName, "Hash Total Capacity.txt");	// set file name
        ofstream outClientFile( fName, ios::out );
        outClientFile << TotalCapacity << endl;
        outClientFile << BucketMask << endl;

    }
    // dump GCed hash table (unique table) to a file, for integrity purposes ... (it contains Flags also)
    void DumpGCedHashTable(int fileNo){
            char fName[40];
            sprintf(fName, "GCed Hash Table Dump %d .txt",fileNo);	// set file name
            ofstream outClientFile( fName, ios::out );
            int i;

            outClientFile << TotalCapacity << endl;
            outClientFile << BucketMask << endl;

            for (i = 0; i < TotalCapacity; i++){
            /*	outClientFile << Table[i].getHopInfo() << endl;

                    outClientFile << Table[i].getIdP().getPointer() << endl;
                    outClientFile << Table[i].getIdP().getBucket() << endl;

                    outClientFile << Table[i].getLevel() << endl;
                    // then
                    outClientFile << Table[i].getThenP().getPointer() << endl;
                    outClientFile << Table[i].getThenP().getBucket() << endl;
                    // else
                    outClientFile << Table[i].getElseP().getPointer() << endl;
                    outClientFile << Table[i].getElseP().getBucket() << endl;
                    // flag
                    outClientFile << (unsigned int)Table[i].getFlags() << endl;*/
            }
    }
    void checkSegmentsConsistancy(){
        T i;
        for (i = 0; i< ConcurrencyDegree; i++){
            Segments[i].checkSegmentsConsistancy();
        }
    }
};
#endif