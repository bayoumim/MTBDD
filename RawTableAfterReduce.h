#ifndef _RAW_TABLE_AFTER_REDUCE_CLASS
#define _RAW_TABLE_AFTER_REDUCE_CLASS
#include "BDDUtil.h"
#include "Bucket.h"

//template 
template<typename T,typename V,typename L,typename K>	// T and V are types of the fields in the BDDPointer
class RawTableAfterReduce{
protected:	
    // private members
    T TotalCapacity; 	// determine the total capacity of the table
    Bucket <T,V,L> * Table; 		// pointer to the table
    unsigned char BucketSize;	// determine the size of the neighborhood
    T BucketMask;		// used to mask key
	// private functions
	// hash single key
    K hashSingleKey(K key){
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

public:
    // constructors
    RawTableAfterReduce(){}

    RawTableAfterReduce(int fileNo){
        char fName[40];
        sprintf(fName, "Hash Table Dump %d .txt",fileNo);	// set file name
        ifstream outClientFile(fName);
        char temp[32];
        this->BucketSize = 32;
        outClientFile.getline(temp, 32);
        this->TotalCapacity = strtoul(temp,0,10);	// get total capacity
        Table = new Bucket <T,V,L> [TotalCapacity];
        outClientFile.getline(temp, 32);
        this->BucketMask  = strtoul(temp,0,10);	// get Bucket Mask
        T i;

        // fill up table
        for(i =0; i< TotalCapacity; i++){
            outClientFile.getline(temp, 32);	// get hop info
            T t = strtoul(temp,0,10);
            Table[i].setI(t);

            outClientFile.getline(temp, 32);	// get then
            t = strtoul(temp,0,10);
            Table[i].setT(t);

            outClientFile.getline(temp, 32);	// get else
            t = strtoul(temp,0,10);
            Table[i].setE(t);

            outClientFile.getline(temp, 32);	// get group data
            t = strtoul(temp,0,10);
            Table[i].setGD(t);
        }
    }
    // this function validate whether a node with key "key" is exist in the raw table, and return a "raw id" for this key. if the key doesn't exist it returns 'false'
    bool ValidateAndReturnRawID (BDDNode <T,V,L> key, BDDPointer <T,V> & id) {
        T hashCode;
        T hashString;
        hashString = getHashString(key);
        hashCode = getHashCode(hashString);	// get hash code for this key
        Bucket <T,V,L> tempBucket = Table[hashCode];	// get the bucket
        T hopinfo = tempBucket.getHopInfo();	// get its hop information
        T nextNeighbor;
        while ( getNearestNeighbor(hashCode, hopinfo, nextNeighbor) ){	// get the nearest neighbor
            if (Table[nextNeighbor].getNode().isEqual(key)){	// if you found the match,  store the pointer unlock the read lock then return true
                id.setHashString(hashString);
                id.setLocalId(Table[nextNeighbor].getLocalId());	// store the id in the pointer
                id.clearTemporary();
                id.ClearInverted();
                return true;
            }
        }
        cout << "can't find the node: RawTableAfterReduce, ValidateAndReturnRawID " << endl; // this is an impossible situation if every thing is all right
        cout << "Total Capacity: " << TotalCapacity << endl;
        key.DumpToScreen();
        exit(-1);
        return false;
    }
};

#endif 
