#ifndef _RAW_TO_NEW_LOOK_UP_TABLE_CLASS
#define _RAW_TO_NEW_LOOK_UP_TABLE_CLASS
#include "BDDUtil.h"
#include "LookUpTableNode.h"

//template 
template<typename T,typename V>	// T and V are types of the fields in the BDDPointer
class NewToRawLookUpTable{
protected:	
    // private members
    T TotalCapacity; 	// determine the total capacity of the table
    LookUpTableNode <T,V>* Table; 		// pointer to the table
    unsigned char BucketSize ;	// determine the size of the neighborhood
    T BucketMask;		// used to mask key

    // private functions
    // hash single key
    T hashSingleKey(T key)
    {
        key = ~key + (key << 15);
        key = key ^ (key >> 12);
        key = key + (key << 2);
        key = key ^ (key >> 4);
        key = (key + (key << 3)) + (key << 11);
        key = key ^ (key >> 16);
        return key;
    }
    // get the nearest neigbor for certain bucket based on its hop info (hop info is modified each time this function called )
    bool getNearestNeighbor(T hashcode, T &hopinfo, T &location){
        if(hopinfo == 0){	// if hop information is zero return false
            return false;
        }
        // if it does contain data, find the nearest neighbor
        T x = BucketSize /2;
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

    bool findNearestEmptyLocation(T start, T & location ){
        if (start >= TotalCapacity) {	//if you reached the last place in the hash, return false
            return false;
        }
        
        while( !(Table[start].isEmpty()) ){
            start++;
            if (start == TotalCapacity) {	//if you reached the last place in the hash, return a number bigger than the capacity of the table
                // FIND PALCE TO RESIZE THE TABLE
                return false;
            }
        }
        location = start;	// store the new value
        return true;
    }
    // check if there a place to add a new bucket
    bool isThereAnEmptyPlace(T hopinfo){
        T h = hopinfo;
        return h ^ 0xFFFFFFFF; // IF ALL slots contains ones; then return zero
    }
    // upadate info field (this is required when swapping is exploited)
    void updateInfo(T bucket, T oldlocation, T newlocation){
        // we don't have to lock this function, as the programmer will lock it from outside
        if( (oldlocation < bucket) || (newlocation < bucket) ){
            cout<< "error in update info function";
            return;
        }
        unsigned char oldshift = BucketSize - (oldlocation - bucket) - 1;
        unsigned char newshift = BucketSize - (newlocation - bucket) - 1;
        T temp = Table[bucket].getHopInfo();
        temp = (temp ^ (1<<oldshift) ); // unset the old location
        temp = (temp ^ (1<<newshift) );	// set the new location
        Table[bucket].setHopInfo(temp);	// store the new value of Hop Information
        return;
    }
    // add a new item to the bucket list
    void AddNewItemInfo(T bucket, T itemLocation){
        // debugging ADDNewItemInfo
        //cout << "bucket no. : " << bucket << endl;
        //cout << "insert element at. : " << itemLocation << endl;
        if (itemLocation < bucket)
            cout << "Invalid input : AddNewItemInfo Failed" << endl;
        T shift = BucketSize - (itemLocation - bucket ) - 1;
        T temp = Table[bucket].getHopInfo();
        shift = (1<<shift);
        //cout << "shifts" << hex << shift << setbase (10) << endl;
        temp = (temp ^ shift ); // set the old location
        Table[bucket].setHopInfo(temp);
        return;
    }
    // get no. of bits to represent a number
    int getNoOfBits (T Number) {
        int bitsCount = 0;
        do {
            bitsCount ++;
            Number >>= 1;
        }
        while ( Number!= 0);
        return bitsCount;
    }
    // convert 'n' to '2^n'
    int getPowerOf2(T number) {
        return 1<<(number);
    }
    // approximate teh number to higher value of 2
    int ApproximateToBiggerPowerOf2(T number){
        return getPowerOf2(getNoOfBits(number));
    }

    // do the same function of the above function + return an pointer to the node , but don't unlock the bucket of the hash code . // if any resizing occurs the output of contains will be false
    // key is input
    // bp : is output
    /// lock: input
    // finalSeg : output
    // nextSeg : output (used only when the lock is false)
    // nextNeighbor: location of the last location in the bucket
    bool containsWithoutUnlocking(BDDPointer <T,V> key, BDDPointer <T,V> &bp, T &nextNeighbor) {
        T hashCode = getHashCode(key);	// compute hash code
        nextNeighbor = hashCode;		// initialize the nextNeighbor
        LookUpTableNode <T,V> tempBucket = Table[hashCode];	// get the bucket
        T hopinfo = tempBucket.getHopInfo();	// get its hop information
        while ( getNearestNeighbor(hashCode, hopinfo, nextNeighbor) ){	// get the nearest neighbor
            if ( (Table[nextNeighbor].getNewId().isEqual(key))  && (!Table[nextNeighbor].isEmpty())  ){	// if you found the match store the pointer unlock the read lock then return true
                bp = Table[nextNeighbor].getRawId();	// store the id in the pointer
                return true;
            }
        }
        // if you didn't find a match; then unlock the read lock then return false
        return false;
    }


public:
    // constructors
    NewToRawLookUpTable(){}

    NewToRawLookUpTable(T TotalCapacity, T BucketMask){
        this->BucketSize = 32;
        this->TotalCapacity = TotalCapacity;	// get the total capacity
        this->BucketMask = BucketMask;
        Table = new LookUpTableNode <T,V> [TotalCapacity];
        BDDPointer <T,V> bp;
        // add "one" node pointer
        bp.setHashString(TERMINAL_LEVEL);
        bp.setLocalId(ONE_ID);
        bp.clearTemporary();
        bp.ClearInverted();
        findOrAdd(bp,bp);
    }
    ~NewToRawLookUpTable(){
        delete [] Table;
    }
    // compute the hash function for the key (triple key)
    int getHashCode(BDDPointer <T,V>  newId){
        T l = hashSingleKey(newId.composePointer());
        l = l & (this->BucketMask); 	// round the key to be in the range
        return l;
    }
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // find Or Add
    // store the node if it isn't already exists
    // Inputs: the node
    // pointer to the node
    // outputs: bool; true if the node is already exists, false : if the node is a new node

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool findOrAdd(BDDPointer <T,V> key , BDDPointer <T,V> & bp){
        T hashCode = getHashCode(key);	// compute hash code
        T LastLoc;

        if ( containsWithoutUnlocking(key, bp, LastLoc) ){ // if any resizing occurs the output of contains will be false, so there isn't any handling of resizing in this section
            return true;	// the node is already there....
        }

        // in order to reduce waiting during scanning the table to obtain an empty location, I utilize a read lock to search for an empty location. then, I aquire a write loc in order to modify this location. But first, I will have to check if this location remains empty after I release the read lock and before aquire the write lock. if the location is the same , this would be fine. if the location is changed, i start from the last location in hand and search again.
        T locR = LastLoc;	// location obtained from read lock
        if( !findNearestEmptyLocation(LastLoc, locR) ){	// aquire read lock
            // this section is called when the thread didn't find a new location
            cout << "the bucket is full you have to resize the table: Hash. In Find Nearest Neighbor read\n";
            return false;				// this value has no meaning, as the node doesn't exist or added
        }
        // the lock is aquired (write lock). and the segment is "Seg"
        T info1;
        T loc1;
        // hop the empty location all the way toward its bucket
        T emptyLoc = locR;
        T i;
        bool flag;
        while ( emptyLoc >= (BucketSize+ hashCode) ){
            flag = false;	// initialization for the next round
            //cout << "you are far from the desired bucket, in Hash" << endl;
            T start = emptyLoc - (BucketSize-1);
            for(i =  start; i < emptyLoc ; i++) {
                info1 = Table[i].getHopInfo();	// get info for this bucket
                while(getNearestNeighbor(i,info1,loc1)){
                    if (loc1 < emptyLoc) {	// if the bucket isn't reserved, you can swap
                        flag = true;	//rise the flag
                        Table[emptyLoc].setFlags(Table[loc1].getFlags());

                        BDDPointer <T,V>req = Table[loc1].getRawId();
                        Table[emptyLoc].setRawId(req);	// swap the raw id

                        BDDPointer <T,V> req1 = Table[loc1].getNewId();
                        Table[emptyLoc].setNewId(req1);	// swap the new id

                        updateInfo (i,loc1,emptyLoc);	// update info
                        emptyLoc = loc1;			// update the new empty location
                        break;
                    }
                }
                if(flag){
                    break;				// exit "for loop" as we reach the nearest bucket in this round
                }
            }

            if(flag){
                continue;				// continue , you have make one swap, so you have to check whether you are in the neighborhood or not
            }

            // here we should check the flag, because if it isn't been rised, we have no space and the table will need to be resized and you have to unlock the write lock
            if (( i == start + BucketSize-1 ) && (!flag)) {
                cout << "the bucket is full you have to resize the table: Hash 1\n";
                cout << "program terminated" << endl;
                exit(-1);
            }
        }

        bool returnFlag;
        if ( (emptyLoc - hashCode) >= BucketSize){
                cout << "error in adding node : emptyLoc : " << emptyLoc << "hashCode : " << hashCode << endl;
        }
        Table[emptyLoc].setRawId(bp);		// add raw id
        Table[emptyLoc].setNewId(key);		// add new id
        AddNewItemInfo (hashCode,emptyLoc);	// update info
        returnFlag = false;
        return returnFlag;
    }
// THIS FUNCTION RETURN NODE CORRESPONDING TO CERTAIN POINTER, it doesn't remove the node (it just read it) (it will be more similar to contain the second version) // FINISHED
    bool getRawId (BDDPointer <T,V> key, BDDPointer <T,V> & bp) {
        T hashCode = getHashCode(key);	// compute hash code;
        LookUpTableNode <T,V> tempBucket = Table[hashCode];	// get the bucket
        T hopinfo = tempBucket.getHopInfo();	// get its hop information
        T nextNeighbor;
        while ( getNearestNeighbor(hashCode, hopinfo, nextNeighbor) ){	// get the nearest neighbor
            if ((Table[nextNeighbor].getNewId().isEqual(key)) && (!Table[nextNeighbor].isEmpty()) ){	// if you found the match,  store the pointer unlock the read lock then return true
                bp = Table[nextNeighbor].getRawId();	// get the id in the pointer
                return true;
            }
        }
        cout << "can't find the node: NewToRawLookUpTable, getRawId" << endl; // this is an impossible situation if every thing is all right
        cout << "Total Capacity: " << TotalCapacity << endl;
        exit(-1);
        return false;
    }
    bool contains (BDDPointer <T,V> key) {
        T hashCode = getHashCode(key);	// compute hash code;
        LookUpTableNode <T,V> tempBucket = Table[hashCode];	// get the bucket
        T hopinfo = tempBucket.getHopInfo();	// get its hop information
        T nextNeighbor;
        while ( getNearestNeighbor(hashCode, hopinfo, nextNeighbor) ){	// get the nearest neighbor
            if ((key == Table[nextNeighbor].getNewId()) && (!Table[nextNeighbor].isEmpty()) ){	// if you found the match,  store the pointer unlock the read lock then return true
                cout << "Error: find the node: NewToRawLookUpTable, contains" << endl; // this is an impossible situation if every thing is all right
                cout << "key : " << key << endl;
                cout << "raw id" << endl;
                Table[nextNeighbor].getRawId().DumpToScreen();
                cout << "Total Capacity: " << TotalCapacity << endl;
                exit(-1);
                return true;
            }
        }
        return false;
    }	
};

#endif 
