#ifndef _HASH_QUEUE_CLASS
#define _HASH_QUEUE_CLASS

#include "BDDUtil.h"
#include "Bucket.h"
#include "Segment.h"
#include "Request.h"
#include "RequestBucket.h"
#include "mutex.h"
#include "IntegrityChecker.h"
//#include "GCQueue.h"
#include "Hash.h"

enum
{
   HQRZ_NODE_ADDED_CORRECTLY,         	/* 0 */
   HQRZ_NODE_NOT_ADDED,        		/* 1 */
   HQRZ_NODE_ADDED_LIMIT_REACHED      	/* 2 */
};

//template
template<typename T,typename V,typename L,typename K,typename FLG>
class HashQueue{
private:
// private memebers
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // Basic paramters : which is related to building tables and segments
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    double MaxUtilization;                      // determine the utilization percentage of the table
    T inCapacity;                               // determine the  input capacity of the table
    unsigned short  ConcurrencyDegree;          // determine number of threads
    T TotalCapacity;                            // determine the total capacity of the table
    unsigned char BucketSize;                   // determine the size of the neighborhood
    T SegmentSize;                              // total number of segments
    RequestBucket <T,V,L,FLG>* Table;           // pointer to the table
    Segment * Segments;                         // pointer to the segments
    T BucketMask;                               // used to mask keybool enableComputedTable
    bool enableComputedTable;                   // flag for enabling or disabling the coomputed table
    Mutex mutex;
    volatile T NodeCounter;                     // counter to keep track of the stored requests
    T fileCounter;                              // counter for dumping data to file
    T *levelStake;                              // array indicate the stake of every level
    L VariableCount;                            // number of variable
    Hash <T,V,L,K,FLG> * UniqueTable;		// pointer to unique table
    T NoOfNodesFreed;
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // Integrity Check related paramters
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    unsigned char hashIntegrityCheck;           // indicate the type of integrity check
    IntegrityChecker <T,V,L,K,FLG> * integrityChecker;
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // GC related parameters
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    int GCEnabled;
//	GCQueue * gcQueue;

    ///////////////////////////////////////////////////////////////////////////////////////
    //			Private Functions
    ///////////////////////////////////////////////////////////////////////////////////////


    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // Hashing and segments related functions
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // hash single key
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

    // compute the hash function for the key (triple key)
    K getHashString(RequestKey<T,V> & key){
        K t = key.getF().composePointer();
        K e = key.getG().composePointer();
        K l = key.getH().composePointer();
        t = hashSingleKey(t);
        e = hashSingleKey(t^e);
        l = hashSingleKey(l^e);
        return l;
    }
    // compute the bucket
    T getHashCode(T l){
        return l & (this->BucketMask);
    }
    // calculate the corresponding segment to certain hash code
    unsigned short getSegment(T hashCode){
        unsigned short seg = floor((float)(hashCode/SegmentSize)) ;
        return seg;
    }

    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // hop information related functions
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // get no. of bits to represent a number
    T getNoOfBits (T Number) {
        Number++;	// take the terminal level into your account
        unsigned char bitsCount = 0;
        do {
            bitsCount ++;
            Number >>= 1;
        }
        while ( Number != 0);
        return bitsCount;
    }
    // convert 'n' to '2^n'
    T getPowerOf2(T number) {
        return (1<<number);
    }
    // approximate teh number to higher value of 2
    T ApproximateToBiggerPowerOf2(T number){
        return getPowerOf2(getNoOfBits(number));
    }
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // hop information related functions
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // upadate info field (this is required when swapping is exploited)
    void updateInfo(T bucket, T oldlocation, T newlocation){
        // we don't have to lock this function, as the programmer will lock it from outside
        if( (oldlocation < bucket) || (newlocation < bucket) || ( (unsigned char)(oldlocation - bucket) > ( BucketSize -1) ) || ( (unsigned char)(newlocation - bucket) > ( BucketSize -1) ) ){
            cout<< "error in update info function";
            cout << "bucket : " << bucket << endl;
            cout << "new location : " << newlocation << endl;
            cout << "old location : " << oldlocation << endl;
            return;
        }
        unsigned char oldshift = BucketSize - (oldlocation - bucket) - 1;
        unsigned char newshift = BucketSize - (newlocation - bucket) - 1;
        T temp = Table[bucket].getHopInfo();
        temp = (temp & (0xFFFFFFFF - ( 1 << oldshift ) ) ); // unset the old location
        temp = (temp | (1<<newshift) );	// set the new location
        Table[bucket].setHopInfo(temp);	// store the new value of Hop Information
        if (bucket == 5984){
            cout << "hashqueue: updateInfo: bucket no.  : "<< bucket << " after modification "<< endl;
            Table[bucket].DumpToScreen();
        }
        return;
    }
    // remove item from bucket list
    void removeItemInfo(T bucket, T itemLocation){
        if (itemLocation < bucket)
                cout << "Invalid input : removeItemInfo Failed" << endl;
        T shift = BucketSize - (itemLocation - bucket ) - 1;
        T temp = Table[bucket].getHopInfo();
        shift = (1<<shift);
        shift = shift ^ 0xFFFFFFFF;	// invert all bits
        //cout << "shifts" << hex << shift << setbase (10) << endl;
        temp = (temp & shift ); // reset the old location
        Table[bucket].setHopInfo(temp);
    /*    if (bucket == 5984){
            cout << "hashqueue: removeItemInfo: bucket no.  : "<< bucket << " after modification "<< endl;
            Table[bucket].DumpToScreen();
        }*/
        return;
    }
    // get the nearest neigbor for certain bucket based on its hop info (hop info is modified each time this function called ), return true if it find an element, and false otherwise
    // get the nearest neigbor for certain bucket based on its hop info (hop info is modified each time this function called )
    bool getNearestNeighbor(T hashcode, T &hopinfo, T &location){
        if(hopinfo == 0){	// if hop information is zero return false
            return false;
        }
        // if it does contain data, find the nearest neighbor
        T x ;
        T highHalf,lowHalf;
        T hifo ;
        T delta ;

        if (enableComputedTable){	// if the computed table is enabled
            do{
                x = BucketSize >>1; // divide by 2
                hifo = hopinfo;
                delta = hashcode + BucketSize -1;
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
                    x = x >> 1 ;	// divide by 2
                }
                while(x != 0);
                hopinfo = hopinfo ^ (1 << ( BucketSize - (delta - hashcode) - 1 ));	// update hopinformation
                location = delta;

                if (Table[location].isDated()){	// if the value is dated
                    x = hopinfo;	// update x with the new hash info
                }
                else {	// if it is a new value
                    return true;	// return true
                }
                if (x == 0)
                    break;
            }
            while(true);

            return false;	// return false
        }
        else {	// if the computed table is disabled
            x = BucketSize /2;
            hifo = hopinfo;
            delta = hashcode + BucketSize -1;

            do{
                highHalf = (hifo >> x ) & ((T)( ( 1 << (x+1) ) - 1));
                lowHalf = hifo & ((T)( (1 << (x+1) ) - 1));
                if (highHalf != 0){
                    delta-= x;
                    hifo = highHalf;
                }
                else if (lowHalf == 0 ){
                    break;
                }
                else
                    hifo = lowHalf;
                x = x>>1;	// divide by 2
            }
            while(x != 0);
            hopinfo = hopinfo ^ (1 << ( BucketSize - (delta - hashcode) - 1 ));	// update hopinformation
            location = delta;
            return true;
        }
    }
    // get the nearest neigbor for certain bucket based on its hop info (hop info is modified each time this function called ), even this node is dated
    bool getDatedNearestNeighbor(T hashcode, T &hopinfo, T &location){
        if(hopinfo == 0){	// if hop information is zero return false
            return false;
        }
        // if it does contain data, find the nearest neighbor
        T x = BucketSize >> 1;  // divide by 2
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
        return true; //REMOVE THIS AT THE END
    }
    // add a new item to the bucket list
    void AddNewItemInfo(T bucket, T itemLocation){
        // debugging ADDNewItemInfo
        //cout << "bucket no. : " << bucket << endl;
        //cout << "insert element at. : " << itemLocation << endl;
        if (itemLocation < bucket)
            cout << "hashqueue: Invalid input : AddNewItemInfo Failed" << endl;
        T shift = BucketSize - (itemLocation - bucket ) - 1;
        T temp = Table[bucket].getHopInfo();
        shift = (1<<shift);
        //cout << "shifts" << hex << shift << setbase (10) << endl;
        temp = (temp | shift ); // set the  location
        Table[bucket].setHopInfo(temp);
   /*     if (bucket == 5984){
            cout << "hashqueue: AddNewItemInfo: bucket no.  : "<< bucket << " after modification "<< endl;
            Table[bucket].DumpToScreen();
        }*/
        return;
    }
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // find or add related functions
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // get the next local id for this bucket
    V getNextId(T hashCode){
        T hopinfo = Table[hashCode].getHopInfo();	// get its hop information
        T nextNeighbor;
        T idMap = 0;        // bit map representation of the local ids in this bucket
        // obtain a bit map for ids
        while ( getNearestNeighbor(hashCode, hopinfo, nextNeighbor) ){	// get the nearest neighbor
            if ( (!Table[nextNeighbor].isEmpty()) ){	// if you found the match,  store the pointer unlock the read lock then return true
                idMap |= (1<< Table[nextNeighbor].getRLocalId());// assign bits according to id (every id has a location; so that, when the id is exist the location for it is set to one otherwise it is set to zero)
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
    // get the index of the nearest location to "start", you will have to unlock the segment after call this function
    bool findNearestEmptyLocation(T start, T & location, unsigned short & Seg){
        if (start >= TotalCapacity) {	//if you reached the last place in the hash, return false
            return false;
        }

        do{ // repeat until you find an empty location (After locking)
            while( !( ( Table[start].isEmpty() || Table[start].isDated() ) ) ) { // if the bucket is full or not dated (new version) and not reserved
                start++;
                if (start >= TotalCapacity) {	//if you reached the last place in the hash, return a number bigger than the capacity of the table
                    return false;
                }
            }
            Seg = getSegment(start);    // get segment of the location

            // check if this location is still empty or dated
            Segments[Seg].WriteLock();		// aquire write lock in this segment
            if (Table[start].isEmpty() || Table[start].isDated() ){ // if the location is empty or dated
                break;      // get out from searching
            }
            else
                Segments[Seg].Unlock(); // unlock as we have another round
        }
        while(true);

        if ( (Table[start].isDated()) &&(!Table[start].isEmpty()) ) { // if it is a stored node (not empty), dated, and we are have a 'write' lock on it, then we have a dead node that MUST BE removed
            // update hopinfo for the bucket store this node
            RequestKey <T,V> key1 = Table[start].getKey();
            T hashCode = getHashCode(getHashString(key1));	// get bucket
            // we have the possiblity that hashCode and start are in a different segments , so we will have to lock them in order, as well as take care of the possibility that resizing may be onging
            unsigned short seg1 = getSegment(hashCode);
            if(seg1 != Seg){
                if(seg1 > Seg){
                    cout << "============== hashQueue : findNearestEmptyLocation : ERROR IN OBTAINING LOCK ======================" << endl;
                    exit(-1);
                }
                Segments[seg1].WriteLock();		// lock the former segment
            }
            removeItemInfo(hashCode,start );	// remove the hopinfo corresponding to 'start' in bucket  'hashcode'
            if(seg1 != Seg)
                Segments[seg1].Unlock();		// unlock the former segment
        }
        location = start;	// store the new value
        return true;
    }
    // do the same function of the above function + return an pointer to the node + if the computed feature is enabled (it search for dated values, and renew it), it returns without unlocking if the computed table doesn't computed. it also lock the first and second segments only when teh lock flag is rised
    bool containsWithoutUnlocking(RequestKey <T,V> &key, BDDPointer <T,V> &bp, bool lock){
        T hashString = getHashString(key);	// compute hash String
        T hashCode = getHashCode(hashString);	// compute hash code
        T nextNeighbor;		// initialize the nextNeighbor
        unsigned short previousSeg ;		// determine the segment
        unsigned short nextSeg ;			// store the segment to be locked
        T hopinfo ;	// get its hop information
        bool flag2 = true;
        bool flag = true;
        T timeStamp;
        timeStamp = hopinfo = Table[hashCode].getHopInfo();	// get its hop information
        do {
            while ( getDatedNearestNeighbor(hashCode, hopinfo, nextNeighbor) ){	// get the nearest neighbor
                if(lock){
                    if (flag){  // if this time you swap segments
                        previousSeg = getSegment(nextNeighbor);     // get the segment
                        Segments[previousSeg].ReadLock();           // aquire read lock
                        if (timeStamp != Table[hashCode].getHopInfo()){ // hif the hopinfo for bucket is changed, then you have to restart, note that you now get a lock on previous segments
                            flag = false;                               // the first time is finished
                            flag2 = false;                              // skip the error message at the end of the function
                            hopinfo = Table[hashCode].getHopInfo();
                            timeStamp = hopinfo;
                            nextSeg = previousSeg;
                            break;
                        }
                        else{
                            flag = false;                               // the first time is finished
                            nextSeg = previousSeg;
                        }
                    }
                    else {
                        nextSeg = getSegment(nextNeighbor);		// determine the segment of the next bucket
                        if ((nextSeg != previousSeg)) {
                            Segments[previousSeg].Unlock();		// unlock the previous segment EXCEPT the first segment
                            Segments[nextSeg].ReadLock();		// lock the new segment
                            previousSeg = nextSeg;
                            if (timeStamp != Table[hashCode].getHopInfo()){
                                timeStamp = hopinfo = Table[hashCode].getHopInfo(); // update the time stamp
                                flag2 = false;
                                break;  // restart again
                            }
                         }
                        else{

                        }
                    }
                }
                if (( Table[nextNeighbor].getKey().isEqual(key)) ){	// if you found the match store the pointer unlock the read lock then return true
                    if(enableComputedTable){	// if computed table is enabled
                        if((!Table[nextNeighbor].isDated()) && (!Table[nextNeighbor].isEmpty())){	// if it is a new node.
                            bp.setHashString(hashString);
                            bp.setLocalId(Table[nextNeighbor].getRLocalId());
                            bp.setTemporary();   // set temporary
                            bp.ClearInverted();
                        }
                        else if((Table[nextNeighbor].isDated()) && (!Table[nextNeighbor].isEmpty())){	// if it is a dated node
                            if (!Table[nextNeighbor].isForwarded(bp)){	// if the node is not forwarded
                                cout << "manager : containsWithoutUnlocking : error in forwarded node" << endl;
                                exit(-1);
                            }
                            else {	// if the node is forwarded
                                if (!UniqueTable->contains(bp)){	// if this node doesn't still exist in the hash table, skip this request
                                    return false;	// skip this node
                                }
                            }
                        }
                        else	{
                            cout << "manager : containsWithoutUnlocking : error in dated node" << endl;
                            exit(-1);
                        }
                    }
                    else{	// if computed table is disabled
                        if((!Table[nextNeighbor].isDated()) && (!Table[nextNeighbor].isEmpty())){	// check whether it is a new or dated node, if new .....
                            bp.setHashString(hashString);
                            bp.setLocalId(Table[nextNeighbor].getRLocalId());
                            bp.setTemporary();   // set temporary
                            bp.ClearInverted();
                        }
                        else	// if this a dated node
                            continue;	// skip it

                    }

                    if (lock){
                       Segments[nextSeg].Unlock();
                       return true;
                    }
                    else {
                        return true;
                    }

                }	// if statment
            }	// while statment
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
public:
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // Constructors
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    HashQueue () {}
    HashQueue ( T inCapacity,
                unsigned short ConcurrencyDegree,
                double MaxUtilization,
                unsigned char BucketSize,
                bool enableComputedTable,
                unsigned char VariableCount,
                unsigned char hashIntegrityCheck ,
                IntegrityChecker <T,V,L,K,FLG>  *& integrityChecker ,
                unsigned char GCEnabled,
                /*	GCQueue * gcQueue ,*/Hash <T,V,L,K,FLG> * UniqueTable){
        
        this->UniqueTable = UniqueTable;
//	this->gcQueue = gcQueue;
        this->GCEnabled = GCEnabled;
        this->hashIntegrityCheck = hashIntegrityCheck;
        this->integrityChecker = integrityChecker;
        this->enableComputedTable = enableComputedTable;
        this->MaxUtilization = MaxUtilization;
        this->inCapacity = inCapacity;
        this->ConcurrencyDegree = ConcurrencyDegree;
        this->BucketSize = BucketSize;
        this->TotalCapacity = ApproximateToBiggerPowerOf2( ceil(inCapacity/MaxUtilization * 100));	// get the total capacity
        this->BucketMask = TotalCapacity - 1;
        int i;
        this->Segments = new Segment[ConcurrencyDegree];

        for (i = 0 ; i < ConcurrencyDegree; i++) {
            Segments[i].Init();
        }
        TotalCapacity = TotalCapacity + BucketSize -1;
        this->SegmentSize = floor((float)(TotalCapacity/ConcurrencyDegree)) + 1;
//	cout << "segment size : " << SegmentSize << endl;
	cout << "Queue size : " << TotalCapacity << endl;
        Table = new RequestBucket <T,V,L,FLG>[TotalCapacity];	// add a margin for the last element
        NodeCounter = 0;
        fileCounter = 0;
        NoOfNodesFreed = 0;

        //
        this->VariableCount = VariableCount;
        this->levelStake = new T[VariableCount];
        T sum = 0 ;
        float c = 2;
        float r = 0.7;
        float d = r * TotalCapacity / (VariableCount/2 + VariableCount/(c-1)) * 2 * c/(c-1);
        float m = (VariableCount-1)/d/(1-1/c)*2;
        float p = -(VariableCount-1)/(c-1);
        int y;

        for (i = 0 ; i < VariableCount; i++) {
            y = VariableCount - 1 -i;
            sum += (y-p)/m;
            levelStake[y] = (T) sum;
      //      cout << "sum for level i " << y << " is " << sum << endl;
       //     cout << "no of node for level " << y << " is " << (y-p)/m << endl;
        }
    }
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // get internal parameters (used in BFS threads)
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    T getLevelStake(T level){
        return levelStake[level];
    }
    void freeOneNode(){
        mutex.lock();
        NoOfNodesFreed++;
        NodeCounter--;
        mutex.unlock();
    }
    T getFreedNodeCount(){
        T temp;
        mutex.lock();
        temp = NoOfNodesFreed++;
        mutex.unlock();
        return temp;
    }
    void resetFreeNodeCounter(){
        NoOfNodesFreed = 0;
    }
    // return the hash code for some key (used to determine the neighborhood of any key)
    T calHashCode(RequestKey <T,V> & key){
        return  getHashCode(key);
    }
    T getNodeCount() {
        T temp;
        mutex.lock();
        temp = NodeCounter;
        mutex.unlock();
        return temp;
    }
    bool hasSpace(L level){
        if ((getNodeCount() >= levelStake[level])){
            return false;
        }
        else {
            return true;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // METHODS USED IN APPLY
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // find or add request
    // store the node if it isn't already exists
    // Inputs:
    // node : the node
    // currentLevel : inidicate the current level in apply method
    // bp : pointer to the node
    // outputFlags : indicates the following
    // 0 : node added or fetched properly
    // 1 : node doesn't added because there is no empty location for it.
    // 2 : node added but the maximum utilization for this level is reach its maximum
    // outputs: bool; true if the node is already exists, false : if the node is a new node

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool findOrAdd(Request <T,V,L> &node, BDDPointer <T,V> &bp, unsigned char &outputFlags, L &currentLevel){
        outputFlags = HQRZ_NODE_ADDED_CORRECTLY;
     /*   if( (node.getF().getHashString()  == 992888251) && (node.getG().getHashString()  == 0) &&(node.getH().getHashString()  == 1188803641))
            cout << "HashQueue : find or add 1: id : " << endl;*/
        if ((getNodeCount() >= levelStake[currentLevel])){
            outputFlags = HQRZ_NODE_NOT_ADDED;
            return true;	// this value has no meaning as the node is neither added nor fetched
        }

        RequestKey <T,V> key = node.getKey();		// get the key
        T hashString = getHashString(key);	// compute hash String
        T hashCode = getHashCode(hashString);	// compute hash code
        unsigned short seg = getSegment(hashCode);		// determine the segment
        RequestKey <T,V> keyTemp;		// get the key
        T hashCodeTemp;
        // if the node is already exist fetch it and return
        bool lockt = true;
        unsigned short finalSeg;
        // the last argument don't have any meaning....
        if (containsWithoutUnlocking(key, bp, lockt)){
            return true;
        }

        // in order to reduce waiting during scanning the table to obtain an empty location, I utilize a read lock to search for an empty location. then, I aquire a write loc in order to modify this location. But first, I will have to check if this location remains empty after I release the read lock and before aquire the write lock. if the location is the same , this would be fine. if the location is changed, i start from the last location in hand and search again.
        T locR = hashCode;	// location obtained from read lock
        T locW = 0;		// location obtained from write lock (initialized with any value)
        bool flag = false;			// used to determine if the write and read lock are equivalent

        // find nearest empty location and return a segment of this location (segment is write locked)
        if(!findNearestEmptyLocation(locR, locW, seg)){	// aquire write lock
            outputFlags = HQRZ_NODE_NOT_ADDED;
            cout << "the bucket is full you have to resize the table: Hash. In Find Nearest Neighbor write\n";
            cout << "program terminated" << endl;
            if ( getNodeCount() == 6003)
                cout << "" << endl;
            Segments[seg].Unlock();		// Unlock the segment from read lock
            return true;	// this value has no meaning as the node is neither added nor fetched
        }

        // here we have a write lock aquired
        flag = false;					// I will use flage here for another purpose
        unsigned short seg1;		// determine the segment
        T info1;
        T loc1;
        // hop the empty location all the way toward its bucket
        T emptyLoc = locW;
        T i;
        seg1 = seg;
        unsigned short seg2 = seg;
        while ( emptyLoc >= (BucketSize+ hashCode)){
            flag = false;	// initialization for the next round
            //cout << "you are far from the desired bucket: in HashQueue" << endl;
            T start = emptyLoc - (BucketSize-1);
            for(i =  start; i < emptyLoc ; i++) {
                seg1 = getSegment(i);

                if(seg1 != seg){
                    Segments[seg1].WriteLock();		// lock the former segment
                }
                info1 = Table[i].getHopInfo();	// get info for this bucket (if the computed table is enabled)

                while(getDatedNearestNeighbor(i,info1,loc1)){
                    seg2 = getSegment(loc1);
                    keyTemp = Table[loc1].getRequest().getKey();		// get the key of the targeted bucket
                    hashCodeTemp = getHashCode(getHashString(keyTemp));	// compute hash code

                    if (hashCodeTemp == i){ // if the hashcode for the target bucket equal to the hashcode of the request
                        if ( (loc1 < emptyLoc) ) {	// if the bucket isn't reserved, you can swap
                            flag = true;	//rise the flag
                            updateInfo (i,loc1,emptyLoc);	// update info
                            Table[emptyLoc].setFlags(Table[loc1].getFlags());
                            Request <T,V,L> req = Table[loc1].getRequest();
                            Table[emptyLoc].setRequest(req);	// swap the nodes
                            Table[emptyLoc].setRLocalId(Table[loc1].getRLocalId());
                            emptyLoc = loc1;			// update the new empty location
                            if ((!Table[emptyLoc].getRequest().isEqual(req)) || Table[emptyLoc].getFlags() != Table[loc1].getFlags()){
                                cout << "hashqeue: find or add: requests are not swapped correctly" << endl;
                                cout << "req at loc1" << endl;
                                req.DumpToScreen();
                                cout << "req at emptyLoc" << endl;
                                Table[emptyLoc].DumpToScreen();
                                exit(-1);
                            }
                            break;
                        }
                    }
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
            // here we should check the flag, because if it isn't been rised, we have no space and the table will need to be resized and you have to unlock the write lock
            if(flag){
                continue;				// continue , you have make one swap, so you have to check whether you are in the
            }

            if (( i == start + BucketSize-1 ) && (!flag)) {
                outputFlags = HQRZ_NODE_NOT_ADDED;
                cout << "the bucket is full you have to resize the table: HashQueue 1\n";
                cout << "no. of stored nodes : " << getNodeCount() << " of total : " << TotalCapacity << endl;
                if ( getNodeCount() == 6003)
                    cout << "" << endl;
                Segments[seg].Unlock();		// unlock read lock in this segment
                return true;	// this value has no meaning as the node is neither added nor fetched
            }
        }
        // here we have the write lock again
        seg1 = getSegment(hashCode);
        if(seg1 != seg){
            Segments[seg1].WriteLock();		// lock the former segment
        }

        bool returnFlag;
        lockt = false;
        finalSeg = seg;

        if (!containsWithoutUnlocking(key, bp, lockt)) {	// if the element doesn't exist in the hash, then add it, otherwise, return with pointer to it

            if ((getNodeCount() >= levelStake[currentLevel])){
                outputFlags = HQRZ_NODE_NOT_ADDED;
                Segments[seg].Unlock();		// aquire read lock in this segment
                if(seg1 != seg){
                    Segments[seg1].Unlock();		// aquire read lock in this segment
                }
                return true;	// this value has no meaning as the node is neither added nor fetched
            }
            
            mutex.lock();
            NodeCounter++;
            mutex.unlock();
            AddNewItemInfo (hashCode,emptyLoc);	// update info
            info1 = Table[hashCode].getHopInfo();	// get info for this bucket

            T id = getNextId(hashCode);
            bp.setLocalId(id);
            bp.setHashString(hashString);
            bp.setTemporary();
            bp.ClearInverted();

            // store the pointer for the new added node
            Table[emptyLoc].clearAllFlags();

    /*	if (GCEnabled == H_GC_RF_ENABLED){	// if GC is enabled then; increase the reference for 'F', 'G' and 'H' pointers
                    BDDPointer bp1 = node.getF();
                    gcQueue->Ref(bp1);
                    bp1 = node.getG();
                    gcQueue->Ref(bp1);
                    bp1 = node.getH();
                    gcQueue->Ref(bp1);
            }*/

            Table[emptyLoc].setRequest(node);		// add your node
            Table[emptyLoc].setRLocalId(id);

            if(hashString == 1639823127){
                cout << "hashqueue: find or add :: node with hashstring : " << hashString << " and hash code : " << hashCode << " will be added at location : " << emptyLoc << endl;
                cout << " Pointer : " << endl;
                bp.DumpToScreen();
                Table[emptyLoc].DumpToScreen();
            }

            if( (hashIntegrityCheck == IC_VALIDATE_QUEUE)){	// if the 'validate queue' is enabled, then, validate this key
                integrityChecker ->validateQueueNode(key);
            }
            else if( hashIntegrityCheck == IC_VALIDATE_QUEUE_AFTER_APPLY){	// if the 'validate queue' is enabled, then, validate this request after performing apply (as this function is called at the end of apply method)
                integrityChecker ->validateQueueKeyAfterApply(node,bp);
            }
            else if( hashIntegrityCheck == IC_VALIDATE_WITH_DEPRECIATED_QUEUE){	// if the 'validate queue' is enabled, then, validate this key
                integrityChecker->validateDepreciatedQueueNode(key);
            }
            else if( hashIntegrityCheck == IC_VALIDATE_WITH_DEPRECIATED_QUEUE_AFTER_APPLY ){	// if the 'validate queue' is enabled, then, validate this request after performing apply (as this function is called at the end of apply method)
                integrityChecker ->validateDepreciatedQueueKeyAfterApply(node,bp);
            }

            if(!Table[emptyLoc].getRequest().isEqual(node)){
                cout << "hashqueue : find or add : node doesn't stored correctly" << endl;
                exit(-1);
            }


            returnFlag = false;
        }
        else {
            Table[emptyLoc].clearAllFlags();	// this location is empty, as you make a bubble within the hash table, you have to make it as empty , so that it can be utilized latter
            returnFlag = true;
        }


        Segments[seg].Unlock();		// aquire read lock in this segment
        if(seg1 != seg){
            Segments[seg1].Unlock();		// aquire read lock in this segment
        }
        return returnFlag;
    }
    bool ForcedFindOrAdd(Request <T,V,L> &node, BDDPointer <T,V> &bp, unsigned char &outputFlags, L &currentLevel){
        outputFlags = HQRZ_NODE_ADDED_CORRECTLY;
     /*   if( (node.getF().getHashString()  == 992888251) && (node.getG().getHashString()  == 0) &&(node.getH().getHashString()  == 1188803641))
            cout << "HashQueue : find or add 1: id : " << endl;*/

        RequestKey <T,V> key = node.getKey();		// get the key
        T hashString = getHashString(key);	// compute hash String
        T hashCode = getHashCode(hashString);	// compute hash code
        unsigned short seg = getSegment(hashCode);		// determine the segment
        RequestKey <T,V> keyTemp;		// get the key
        T hashCodeTemp;
        // if the node is already exist fetch it and return
        bool lockt = true;
        unsigned short finalSeg;
        // the last argument don't have any meaning....
        if (containsWithoutUnlocking(key, bp, lockt)){
            return true;
        }

        // in order to reduce waiting during scanning the table to obtain an empty location, I utilize a read lock to search for an empty location. then, I aquire a write loc in order to modify this location. But first, I will have to check if this location remains empty after I release the read lock and before aquire the write lock. if the location is the same , this would be fine. if the location is changed, i start from the last location in hand and search again.
        T locR = hashCode;	// location obtained from read lock
        T locW = 0;		// location obtained from write lock (initialized with any value)
        bool flag = false;			// used to determine if the write and read lock are equivalent

        // find nearest empty location and return a segment of this location (segment is write locked)
        if(!findNearestEmptyLocation(locR, locW, seg)){	// aquire write lock
            outputFlags = HQRZ_NODE_NOT_ADDED;
            cout << "the bucket is full you have to resize the table: Hash. In Find Nearest Neighbor write\n";
            cout << "program terminated" << endl;
            if ( getNodeCount() == 6003)
                cout << "" << endl;
            Segments[seg].Unlock();		// Unlock the segment from read lock
            return true;	// this value has no meaning as the node is neither added nor fetched
        }

        // here we have a write lock aquired
        flag = false;					// I will use flage here for another purpose
        unsigned short seg1;		// determine the segment
        T info1;
        T loc1;
        // hop the empty location all the way toward its bucket
        T emptyLoc = locW;
        T i;
        seg1 = seg;
        unsigned short seg2 = seg;
        while ( emptyLoc >= (BucketSize+ hashCode)){
            flag = false;	// initialization for the next round
            //cout << "you are far from the desired bucket: in HashQueue" << endl;
            T start = emptyLoc - (BucketSize-1);
            for(i =  start; i < emptyLoc ; i++) {
                seg1 = getSegment(i);

                if(seg1 != seg){
                    Segments[seg1].WriteLock();		// lock the former segment
                }
                info1 = Table[i].getHopInfo();	// get info for this bucket (if the computed table is enabled)

                while(getDatedNearestNeighbor(i,info1,loc1)){
                    seg2 = getSegment(loc1);
                    keyTemp = Table[loc1].getRequest().getKey();		// get the key of the targeted bucket
                    hashCodeTemp = getHashCode(getHashString(keyTemp));	// compute hash code

                    if (hashCodeTemp == i){ // if the hashcode for the target bucket equal to the hashcode of the request
                        if ( (loc1 < emptyLoc) ) {	// if the bucket isn't reserved, you can swap
                            flag = true;	//rise the flag
                            updateInfo (i,loc1,emptyLoc);	// update info
                            Table[emptyLoc].setFlags(Table[loc1].getFlags());
                            Request <T,V,L> req = Table[loc1].getRequest();
                            Table[emptyLoc].setRequest(req);	// swap the nodes
                            Table[emptyLoc].setRLocalId(Table[loc1].getRLocalId());
                            emptyLoc = loc1;			// update the new empty location
                            if ((!Table[emptyLoc].getRequest().isEqual(req)) || Table[emptyLoc].getFlags() != Table[loc1].getFlags()){
                                cout << "hashqeue: find or add: requests are not swapped correctly" << endl;
                                cout << "req at loc1" << endl;
                                req.DumpToScreen();
                                cout << "req at emptyLoc" << endl;
                                Table[emptyLoc].DumpToScreen();
                                exit(-1);
                            }
                            break;
                        }
                    }
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
            // here we should check the flag, because if it isn't been rised, we have no space and the table will need to be resized and you have to unlock the write lock
            if(flag){
                continue;				// continue , you have make one swap, so you have to check whether you are in the
            }

            if (( i == start + BucketSize-1 ) && (!flag)) {
              /*  outputFlags = HQRZ_NODE_NOT_ADDED;
                cout << "the bucket is full you have to resize the table: HashQueue 1\n";
                cout << "no. of stored nodes : " << getNodeCount() << " of total : " << TotalCapacity << endl;
                if ( getNodeCount() == 6003)
                    cout << "" << endl;
                Segments[seg].Unlock();		// unlock read lock in this segment
                return true;	// this value has no meaning as the node is neither added nor fetched*/
                cout << "hashqueue: foreced find or add : error " << endl;
                exit(-1);
            }
        }
        // here we have the write lock again
        seg1 = getSegment(hashCode);
        if(seg1 != seg){
            Segments[seg1].WriteLock();		// lock the former segment
        }

        bool returnFlag;
        lockt = false;
        finalSeg = seg;

        if (!containsWithoutUnlocking(key, bp, lockt)) {	// if the element doesn't exist in the hash, then add it, otherwise, return with pointer to it
            mutex.lock();
            NodeCounter++;
            mutex.unlock();
            AddNewItemInfo (hashCode,emptyLoc);	// update info

            info1 = Table[hashCode].getHopInfo();	// get info for this bucket

            T id = getNextId(hashCode);
            bp.setLocalId(id);
            bp.setHashString(hashString);
            bp.setTemporary();
            bp.ClearInverted();

            // store the pointer for the new added node
            Table[emptyLoc].clearAllFlags();

    /*	if (GCEnabled == H_GC_RF_ENABLED){	// if GC is enabled then; increase the reference for 'F', 'G' and 'H' pointers
                    BDDPointer bp1 = node.getF();
                    gcQueue->Ref(bp1);
                    bp1 = node.getG();
                    gcQueue->Ref(bp1);
                    bp1 = node.getH();
                    gcQueue->Ref(bp1);
            }*/

            Table[emptyLoc].setRequest(node);		// add your node
            Table[emptyLoc].setRLocalId(id);

            if(hashString == 1639823127){
                cout << "hashqueue: find or add :: node with hashstring : " << hashString << " and hash code : " << hashCode << " will be added at location : " << emptyLoc << endl;
                cout << " Pointer : " << endl;
                bp.DumpToScreen();
                Table[emptyLoc].DumpToScreen();
            }

            if( (hashIntegrityCheck == IC_VALIDATE_QUEUE)){	// if the 'validate queue' is enabled, then, validate this key
                integrityChecker ->validateQueueNode(key);
            }
            else if( hashIntegrityCheck == IC_VALIDATE_QUEUE_AFTER_APPLY){	// if the 'validate queue' is enabled, then, validate this request after performing apply (as this function is called at the end of apply method)
                integrityChecker ->validateQueueKeyAfterApply(node,bp);
            }
            else if( hashIntegrityCheck == IC_VALIDATE_WITH_DEPRECIATED_QUEUE){	// if the 'validate queue' is enabled, then, validate this key
                integrityChecker->validateDepreciatedQueueNode(key);
            }
            else if( hashIntegrityCheck == IC_VALIDATE_WITH_DEPRECIATED_QUEUE_AFTER_APPLY ){	// if the 'validate queue' is enabled, then, validate this request after performing apply (as this function is called at the end of apply method)
                integrityChecker ->validateDepreciatedQueueKeyAfterApply(node,bp);
            }

            if(!Table[emptyLoc].getRequest().isEqual(node)){
                cout << "hashqueue : find or add : node doesn't stored correctly" << endl;
                exit(-1);
            }


            returnFlag = false;
        }
        else {
            Table[emptyLoc].clearAllFlags();	// this location is empty, as you make a bubble within the hash table, you have to make it as empty , so that it can be utilized latter
            returnFlag = true;
        }


        Segments[seg].Unlock();		// aquire read lock in this segment
        if(seg1 != seg){
            Segments[seg1].Unlock();		// aquire read lock in this segment
        }
        return returnFlag;
    }

    // this function overwrite an already exist request (after some update to its temporary node), so it overwrite an already exist node, it would be similar to contains, but when you find data, overwrite it.
    bool putRequestWithoutProcessing (Request <T,V,L> &req, bool thenPointerFinishedFlag, bool elsePointerFinishedFlag){
        RequestKey <T,V> key = req.getKey();
        T hashCode = getHashCode(getHashString(key));	// compute hash code
        unsigned short previousSeg ;		// determine the segment
        unsigned short nextSeg ;			// store the segment to be locked
        T timeStamp;
        bool flag = true;
        bool flag2 = true;
        T hopinfo = Table[hashCode].getHopInfo();	// get its hop information
        timeStamp = hopinfo;
        T nextNeighbor;
        RequestKey <T,V> tempkey;
        do{
            while ( getDatedNearestNeighbor(hashCode, hopinfo, nextNeighbor) ){	// get the nearest neighbor
                if (flag){  // if this time you swap segments
                    previousSeg = getSegment(nextNeighbor);     // get the segment
                    Segments[previousSeg].ReadLock();           // aquire read lock
                    if (timeStamp != Table[hashCode].getHopInfo()){ // hif the hopinfo for bucket is changed, then you have to restart, note that you now get a lock on previous segments
                        flag = false;                               // the first time is finished
                        flag2 = false;                              // skip the error message at the end of the function
                        hopinfo = Table[hashCode].getHopInfo();
                        timeStamp = hopinfo;
                        break;
                    }
                    else{
                        flag = false;                               // the first time is finished
                        nextSeg = previousSeg;
                    }
                }
                else {
                    nextSeg = getSegment(nextNeighbor);		// determine the segment of the next bucket
                    if ((nextSeg != previousSeg)) {
                        Segments[previousSeg].Unlock();		// unlock the previous segment EXCEPT the first segment
                        Segments[nextSeg].ReadLock();		// lock the new segment
                        previousSeg = nextSeg;
                        if (timeStamp != Table[hashCode].getHopInfo()){
                            timeStamp = hopinfo = Table[hashCode].getHopInfo(); // update the time stamp
                            flag2 = false;
                            break;  // restart again
                        }
                     }
                    else{

                    }
                }

                tempkey = Table[nextNeighbor].getKey() ;
                if ( ( key.isEqual(tempkey) && (!Table[nextNeighbor].isEmpty()) && (!Table[nextNeighbor].isDated())  ) ){	// if you found the match store the pointer , overwrite request, unlock the write lock then return true
                    BDDNode <T,V,L> node = req.getR ();

                    if (GCEnabled == H_GC_RF_ENABLED){	// if GC is enabled then; check 'then' and 'else' pointers, if one of them is processed in reduce, the reference it for the first time only.

                        // check for 'then' pointer
                        if ( (!Table[nextNeighbor].isThenProcessedInReduce()) && thenPointerFinishedFlag) { // if the old 'then' pointer is not  processed and the new one is processed, then reference the new one and set the 'reduce processed flag'
                                Table[nextNeighbor].SetThenProcessedInReduce();	// set processed flag
                //		gcQueue->Ref(node.getThenP());					// reference it
                        }

                        // check for 'else' pointer
                        if ( (!Table[nextNeighbor].isElseProcessedInReduce()) && elsePointerFinishedFlag) { // if the old 'else' pointer is not  processed and the new one is processed, then reference it and set the 'reduce processed flag'
                                Table[nextNeighbor].SetElseProcessedInReduce();	// set processed flag
                //		gcQueue->Ref(node.getElseP());					// reference it
                        }

                    }
                    else{	// if there is no GC, then do the same EXCEPT for the Ref operation
                        // check for 'then' pointer
                        if ( (!Table[nextNeighbor].isThenProcessedInReduce()) && thenPointerFinishedFlag) { // if the old 'then' pointer is not  processed and the new one is processed, then reference the new one and set the 'reduce processed flag'
                                Table[nextNeighbor].SetThenProcessedInReduce();	// set processed flag
                        }

                        // check for 'else' pointer
                        if ( (!Table[nextNeighbor].isElseProcessedInReduce()) && elsePointerFinishedFlag) { // if the old 'else' pointer is not  processed and the new one is processed, then reference it and set the 'reduce processed flag'
                                Table[nextNeighbor].SetElseProcessedInReduce();	// set processed flag
                        }
                    }
                    Table[nextNeighbor].setRThen(node.getThen());	// store new request
                    Table[nextNeighbor].setRElse(node.getElse());	// store new request
                    Segments[nextSeg].Unlock();
                    return true;
                }
            }
            if(flag2){  // if the time stamp doesn't change
                // if you didn't find a match; then unlock the read lock then return false
                Segments[nextSeg].Unlock();
                mutex.lock();
                cout << "HashQueue : node doesn't exist : putRequestWithoutProcessing"  << endl;
                cout << "segement no. : " << nextSeg << endl;
                cout << "program terminated" << endl;
                Dump();
                exit(-1);
                mutex.unlock();
                return false;
            }
            else
                flag2 = true;
        }
        while(true);
    }


    // this function overwrite an already exist request (after some update to its temporary node), so it overwrite an already exist node, it would be similar to contains, but when you find data, overwrite it.
    bool putRequest (Request <T,V,L> &req, bool ThenProcessedFlag, bool ElseProcessedFlag){
        RequestKey <T,V> key = req.getKey();
        T hashCode = getHashCode(getHashString(key));	// compute hash code
        unsigned short previousSeg ;		// determine the segment
        unsigned short nextSeg ;			// store the segment to be locked
        T timeStamp;
        bool flag = true;
        bool flag2 = true;
        T hopinfo = Table[hashCode].getHopInfo();	// get its hop information
        timeStamp = hopinfo;
        T nextNeighbor;
        RequestKey <T,V> tempkey;
        do {
            while ( getDatedNearestNeighbor(hashCode, hopinfo, nextNeighbor) ){	// get the nearest neighbor
               if (flag){  // if this time you swap segments
                    previousSeg = getSegment(nextNeighbor);     // get the segment
                    Segments[previousSeg].ReadLock();           // aquire read lock
                    if (timeStamp != Table[hashCode].getHopInfo()){ // hif the hopinfo for bucket is changed, then you have to restart, note that you now get a lock on previous segments
                        flag = false;                               // the first time is finished
                        flag2 = false;                              // skip the error message at the end of the function
                        hopinfo = Table[hashCode].getHopInfo();
                        timeStamp = hopinfo;
                        break;
                    }
                    else{
                        flag = false;                               // the first time is finished
                        nextSeg = previousSeg;
                    }
                }
                else {
                    nextSeg = getSegment(nextNeighbor);		// determine the segment of the next bucket
                    if ((nextSeg != previousSeg)) {
                        Segments[previousSeg].Unlock();		// unlock the previous segment EXCEPT the first segment
                        Segments[nextSeg].ReadLock();		// lock the new segment
                         previousSeg = nextSeg;
                        if (timeStamp != Table[hashCode].getHopInfo()){
                            timeStamp = hopinfo = Table[hashCode].getHopInfo(); // update the time stamp
                            flag2 = false;
                            break;  // restart again
                        }
                     }
                    else{

                    }
                }

                tempkey = Table[nextNeighbor].getKey() ;
                if ( ( key.isEqual(tempkey)  && (!Table[nextNeighbor].isEmpty()) && (!Table[nextNeighbor].isDated()) ) ){	// if you found the match store the pointer , overwrite request, unlock the write lock then return true
                    BDDNode <T,V,L> node = req.getR ();
                    Table[nextNeighbor].setRThen(node.getThen());	// store new request
                    Table[nextNeighbor].setRElse(node.getElse());	// store new request
                    if(ThenProcessedFlag)
                            Table[nextNeighbor].SetThenProcessedInApply();
                    if(ElseProcessedFlag)
                            Table[nextNeighbor].SetElseProcessedInApply();

                    BDDPointer <T,V> bp1 = node.getThen();
                    BDDPointer <T,V> bp2 = node.getElse();

                    if( (!Table[nextNeighbor].getRThen().isEqual(bp1)) || (!Table[nextNeighbor].getRElse().isEqual(bp2))){
                        cout << "hashqueue : putRequest : error in storing then and else pointers" << endl;
                        exit(-1);
                    }
                    if( (hashIntegrityCheck == IC_VALIDATE_QUEUE_AFTER_APPLY)){	// if the 'validate queue' is enabled, then, validate this request after performing apply (as this function is called at the end of apply method)
                        BDDPointer <T,V> bp;
                        bp.setHashString(getHashString(key));
                        bp.setLocalId(Table[nextNeighbor].getRLocalId());
                        bp.setTemporary();
                        bp.ClearInverted();
                        bool ThenProcessedInReduce = Table[nextNeighbor].isThenProcessedInReduce();
                        bool ElseProcessedInReduce = Table[nextNeighbor].isElseProcessedInReduce();
                        integrityChecker ->validateQueueNodeAfterApply(req,bp, ThenProcessedFlag, ElseProcessedFlag, ThenProcessedInReduce, ElseProcessedInReduce );
                    }
                    else if( (hashIntegrityCheck == IC_VALIDATE_WITH_DEPRECIATED_QUEUE_AFTER_APPLY)){	// if the 'validate queue' is enabled, then, validate this request after performing apply (as this function is called at the end of apply method)
                        BDDPointer <T,V> bp;
                        bp.setHashString(getHashString(key));
                        bp.setLocalId(Table[nextNeighbor].getRLocalId());
                        bp.setTemporary();
                        bp.ClearInverted();
                        integrityChecker ->validateDepreciatedQueueNodeAfterApply(req,bp);
                    }
                    Segments[nextSeg].Unlock();
                    return true;
                }
            }
           if(flag2){  // if the time stamp doesn't change
                // if you didn't find a match; then unlock the read lock then return false
                Segments[nextSeg].Unlock();
                mutex.lock();
                cout << "HashQueue : node doesn't exist : putRequest"  << endl;
                cout << "segement no. : " << nextSeg << endl;
                cout << "program terminated" << endl;
                Dump();
                exit(-1);
                mutex.unlock();
                return false;
            }
            else
                flag2 = true;
        }
        while(true);
    }
    // fetch request based on certian pointer, teh request will not be removed from hash table , this method is used in dequeing request in apply method
    bool fetchRequest (BDDPointer <T,V> &bp,Request <T,V,L> &req, bool &ThenProcessedFlag, bool &ElseProcessedFlag){
        T hashCode = getHashCode(bp.getHashString());	// compute hash code
        T timeStamp;
        unsigned short previousSeg ;		// determine the segment
        unsigned short nextSeg ;			// store the segment to be locked
        bool flag = true;
        bool flag2 = true;
        T hopinfo = Table[hashCode].getHopInfo();	// get its hop information
        timeStamp = hopinfo;
        T nextNeighbor;
        ThenProcessedFlag = false;
        ElseProcessedFlag = false;
        do {
            while ( getDatedNearestNeighbor(hashCode, hopinfo, nextNeighbor) ){	// get the nearest neighbor
                if (flag){  // if this time you swap segments
                    previousSeg = getSegment(nextNeighbor);     // get the segment
                    Segments[previousSeg].ReadLock();           // aquire read lock
                    if (timeStamp != Table[hashCode].getHopInfo()){ // hif the hopinfo for bucket is changed, then you have to restart, note that you now get a lock on previous segments
                        flag = false;                               // the first time is finished
                        flag2 = false;                              // skip the error message at the end of the function
                        hopinfo = Table[hashCode].getHopInfo();
                        timeStamp = hopinfo;
                        break;
                    }
                    else{
                        flag = false;                               // the first time is finished
                        nextSeg = previousSeg;
                    }
                }
                else {
                    nextSeg = getSegment(nextNeighbor);		// determine the segment of the next bucket
                    if ((nextSeg != previousSeg)) {
                        Segments[previousSeg].Unlock();		// unlock the previous segment EXCEPT the first segment
                        Segments[nextSeg].ReadLock();		// lock the new segment
                        previousSeg = nextSeg;
                        if (timeStamp != Table[hashCode].getHopInfo()){
                            timeStamp = hopinfo = Table[hashCode].getHopInfo(); // update the time stamp
                            flag2 = false;
                            break;  // restart again
                        }
                     }
                    else{

                    }
                }
                if ( (bp.getLocalId() == Table[nextNeighbor].getRLocalId()) && (!Table[nextNeighbor].isDated()) && (!Table[nextNeighbor].isEmpty()) ){	// if you found the match , unlock the read lock then return true
                    ThenProcessedFlag = Table[nextNeighbor].isThenProcessedInApply();
                    ElseProcessedFlag = Table[nextNeighbor].isElseProcessedInApply();
                    req = Table[nextNeighbor].getRequest();
                    Segments[nextSeg].Unlock();
                    return true;
                }
            }
            // if you didn't find a match; then unlock the read lock then return false
            if(flag2){  // if the time stamp doesn't change
                mutex.lock();
                cout << "HashQueue : node doesn't exist : fetchRequest"  << endl;
                bp.DumpToScreen();
                cout << "program terminated" << endl;
                Dump();
                mutex.unlock();
                Segments[nextSeg].Unlock();
                exit(-1);
                return false;
            }
            else
                flag2 = true;
        }
        while(true);

    }
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // METHODS USED IN REDUCE
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // this function overwrite an already exist request (after some update to its temporary node), so it overwrite an already exist node, it would be similar to contains, but when you find data, overwrite it. and mark the request as dated. this function is only called at the reduce function
    bool putOldRequest (Request <T,V,L> &req, bool ExistNodeFlag, BDDPointer <T,V> & bp){
        RequestKey <T,V> key = req.getKey();
        T hashString = getHashString(key);
        T hashCode = getHashCode(hashString);	// compute hash code
        T hopinfo = Table[hashCode].getHopInfo();	// get its hop information
        T nextNeighbor;
        while ( getDatedNearestNeighbor(hashCode, hopinfo, nextNeighbor) ){	// get the nearest neighbor
            if ( Table[nextNeighbor].getKey().isEqual(key) && (!Table[nextNeighbor].isDated()) && (!Table[nextNeighbor].isEmpty()) ){	// if you found the match store the pointer , overwrite request, unlock the write lock then return true
                BDDNode <T,V,L> node = req.getR ();
                if( (req.getF().getHashString()  == 992888251) && (req.getG().getHashString()  == 0) &&(req.getH().getHashString()  == 1188803641))
                    cout << "HashQueue : putOldRequest: " << endl;
                // the node is forwarded, so, this means that both pointers are processed (the pointers may be forwarded, but this will not cause a problem. as if teh node are forwarded, they will not processed again in the reduce)
        /*	if (GCEnabled == H_GC_RF_ENABLED){	// if GC is enabled then; decrease the reference for 'F', 'G' and 'H' pointers
                        BDDPointer <T,V> bp;
                        bp = req.getF();
                        gcQueue->DeRef(bp);
                        bp = req.getG();
                        gcQueue->DeRef(bp);
                        bp = req.getH();
                        gcQueue->DeRef(bp);

                        // deref 'then' pointer
                        if ( Table[nextNeighbor].isThenPointerProcessedInReduce()) { // if the old 'then' pointer is processed , then DeRef it, as the node is forwarded
                                gcQueue->DeRef(Table[nextNeighbor].getRThen());					// reference it
                        }

                        // deref 'else' pointer
                        if ( Table[nextNeighbor].isElsePointerProcessedInReduce()) { // if the old 'else' pointer is processed , then DeRef it, as the node is forwarded
                                gcQueue->DeRef(Table[nextNeighbor].getRElse());					// reference it
                        }

                        // if the forwarded node is especially created for this request, then dereference it (to compansate the RC=1 when we create a new node)
                        if(!ExistNodeFlag){
                                node.isForwarded(bp);	// get forwarded node

                                gcQueue->DeRef(bp);
                        }
                }*/

                node.isForwarded(bp);
                Table[nextNeighbor].SetForwarded(bp);

                Table[nextNeighbor].SetDated();		// as this function is used only at reduce method, to update the requests, I set all requests as a dated one.
                bp.setHashString(hashString);
                bp.setLocalId(Table[nextNeighbor].getRLocalId());
                bp.setTemporary();
                bp.ClearInverted();

                BDDPointer <T,V> bp1;
                node.isForwarded(bp1);
                BDDPointer <T,V> bp2;
                Table[nextNeighbor].isForwarded(bp2);
                if(!bp1.isEqual(bp2)){
                    cout << "hashqueue: putOldRequest : error in storing forwarded node"<< endl;
                    exit(-1);
                }

                if( (hashIntegrityCheck == IC_VALIDATE_WITH_DEPRECIATED_QUEUE_AFTER_REDUCE)){	// if the 'validate queue' is enabled, then, validate this request after performing apply (as this function is called at the end of apply method)
                    BDDPointer <T,V> bp1;
                    Table[nextNeighbor].isForwarded(bp1);
                    integrityChecker ->validateDepreciatedQueueNodeAfterReduce(key, bp1 );
                }
        	else if( (hashIntegrityCheck == IC_VALIDATE_QUEUE_AFTER_REDUCE)){	// if the 'validate queue' is enabled, then, validate this request after performing apply (as this function is called at the end of apply method)
                    BDDPointer <T,V> bp1;
                    Table[nextNeighbor].isForwarded(bp1);
                    integrityChecker ->validateQueueNodeAfterReduce(key, bp1 );
                }
                return true;
            }
        }
        // if you didn't find a match; then unlock the read lock then return false
        mutex.lock();
        cout << "HashQueue : node doesn't exist : putOldRequest"  << endl;
        cout << "program terminated" << endl;
        Dump();
        exit(-1);
        mutex.unlock();

        return false;
    }
    // it fetch a temporary node from queue based on its pointer, (the fetched node will remain in the queue as well)
    bool fetchNode (BDDPointer <T,V> &bp, BDDNode <T,V,L> &node){
        T hashCode = getHashCode(bp.getHashString());	// compute hash code
        T hopinfo = Table[hashCode].getHopInfo();	// get its hop information
        T nextNeighbor;
        while ( getDatedNearestNeighbor(hashCode, hopinfo, nextNeighbor) ){	// get the nearest neighbor
            if ( (Table[nextNeighbor].getRLocalId() == bp.getLocalId() )&& (!Table[nextNeighbor].isEmpty()) ){	// if you found the match store the node, unlock the read lock then return true
                node = Table[nextNeighbor].getNode();	// store the id in the pointer
                return true;
            }
        }
        // if you didn't find a match; then unlock the read lock then return false

        mutex.lock();
        cout << "HashQueue : node doesn't exist : fetchNode"  << endl;
        cout << "hashCode " << hashCode << endl;
        bp.DumpToScreen();
        cout << "program terminated" << endl;
        Dump();
        exit(-1);
        mutex.unlock();
        return false;
    }
    // it fetch a temporary node from queue based on its pointer, (the fetched node will remain in the queue as well), even it is dated or not, this used when we fetch the temporary node after the reduce is finished
    bool fetchDatedNode (BDDPointer <T,V> &bp, BDDNode <T,V,L> &node){
        T hashCode = getHashCode(bp.getHashString());	// compute hash code
        T hopinfo = Table[hashCode].getHopInfo();	// get its hop information
        T nextNeighbor;
        while ( getDatedNearestNeighbor(hashCode, hopinfo, nextNeighbor) ){	// get the nearest neighbor
            if ( (Table[nextNeighbor].getRLocalId() == bp.getLocalId()) && (!Table[nextNeighbor].isEmpty())){	// if you found the match store the node, unlock the read lock then return true
                node = Table[nextNeighbor].getNode();	// store the id in the pointer
                return true;
            }
        }
        // if you didn't find a match; then unlock the read lock then return false
        mutex.lock();
        cout << "HashQueue : node doesn't exist : fetchDatedNode"  << endl;
        bp.Dump();
        cout << "program terminated" << endl;
        Dump();
        exit(-1);
        mutex.unlock();
        return false;
    }
    // fetch request based on certian pointer, teh request will not be removed from hash table , this method is used in dequeing request in apply method
    bool fetchRequestWithoutLocking (BDDPointer <T,V> &bp,Request <T,V,L> &req, bool &ThenProcessedFlag, bool &ElseProcessedFlag){
        T hashCode = getHashCode(bp.getHashString());	// compute hash code
        T hopinfo = Table[hashCode].getHopInfo();	// get its hop information
        T nextNeighbor;
        ThenProcessedFlag = false;
        ElseProcessedFlag = false;
        while ( getNearestNeighbor(hashCode, hopinfo, nextNeighbor) ){	// get the nearest neighbor
            if ( (Table[nextNeighbor].getRLocalId() == bp.getLocalId()) && (!Table[nextNeighbor].isEmpty()) ){	// if you found the match , unlock the read lock then return true
                ThenProcessedFlag = Table[nextNeighbor].isThenProcessedInApply();
                ElseProcessedFlag = Table[nextNeighbor].isElseProcessedInApply();
                req = Table[nextNeighbor].getRequest();
                return true;
            }
        }
        // if you didn't find a match; then unlock the read lock then return false
        mutex.lock();
        cout << "HashQueue : node doesn't exist : fetchRequestWithoutLocking"  << endl;
        bp.DumpToScreen();
        cout << "program terminated" << endl;
//	Dump();
        exit(-1);
        mutex.unlock();
        return false;
    }
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // Unique table access functions
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // calculate the top variable index of three nodes (depends on their id)
    L getTopVarIndex (BDDPointer <T,V> F, BDDPointer <T,V> G, BDDPointer <T,V> H) {
        L idf = UniqueTable->getLevelFromID(F);
        L idg = UniqueTable->getLevelFromID(G);
        L idh = UniqueTable->getLevelFromID(H);
        L max = (idf > idg) ? idf : idg;
        max =  (max > idh) ? max : idh;
        return max;
    }
    // An overloaded version
    L getTopVarIndex (RequestKey <T,V> &key) {
        BDDPointer <T,V> F = key.getF();
        BDDPointer <T,V> G = key.getG();
        BDDPointer <T,V> H = key.getH();
        return getTopVarIndex(F,G,H);
    }
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    // get hash internal parameters (for debugging and internal validation purposes)
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    void Dump() {
        char fName[40];
        fileCounter++;
        if (fileCounter>= (0)){
            sprintf(fName, "hashQueue Dump%d.txt",fileCounter);	// set file name
            ofstream outClientFile( fName, ios::out );
            Dump(outClientFile);
        }
    }
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
        for(i = 0; i <TotalCapacity; i++){
            ff << "element no. " << i << endl;
            Table[i].Dump(ff);
        }
    }
    void DumpToScreen() {
        cout << "======================================================================="<< endl;
        cout << "   				Table Information			" << endl;
        cout << "Max. Utilization : " << MaxUtilization << endl;
        cout << "input Capacity : " << inCapacity << endl;
        cout << "Concurrency Degree : " << ConcurrencyDegree << endl;
        cout << "Bucket Size : " << BucketSize << endl;
        cout << "Segment Size : " << SegmentSize << endl;
        cout << "Total Capacity : " << TotalCapacity << endl;
        int i;
        cout << "======================================================================="<< endl;
        cout << "   				DATA					" << endl;
        cout << "======================================================================="<< endl;
        // dump all table
        for(i = 0; i < TotalCapacity; i++){
            cout << "element no. " << i << endl;
            Table[i].DumpToScreen();
        }
    }
    // dump hash table (unique table) to a file, for integrity purposes ...
    void DumpQueue(int fileNo){
    	char fName[40];
        sprintf(fName, "Queue Dump %d.txt",fileNo);	// set file name
        ofstream outClientFile( fName, ios::out );
        T i;
        outClientFile << TotalCapacity << endl;
        outClientFile << BucketMask << endl;

        for (i = 0; i < TotalCapacity; i++){
            Table[i].DumpToFile(outClientFile);
        }
    }

    // dump hash table (unique table) to a file, for integrity purposes ...
    void DumpQueueAfterApply(int fileNo){
        char fName[40];
        sprintf(fName, "Queue Dump After Apply %d .txt",fileNo);	// set file name
        ofstream outClientFile( fName, ios::out );
        T i;
        outClientFile << TotalCapacity << endl;
        outClientFile << BucketMask << endl;

        for (i = 0; i < TotalCapacity; i++){
            Table[i].DumpToFile(outClientFile);
        }
    }

    // dump hash table (unique table) to a file, for integrity purposes ...
    void DumpQueueAfterReduce(int fileNo){
    	char fName[40];
        sprintf(fName, "Queue Dump After Reduce %d .txt",fileNo);	// set file name
        ofstream outClientFile( fName, ios::out );
        T i;
        outClientFile << TotalCapacity << endl;
        outClientFile << BucketMask << endl;

        for (i = 0; i < TotalCapacity; i++){
            Table[i].DumpToFile(outClientFile);
        }
    }
};
#endif 
