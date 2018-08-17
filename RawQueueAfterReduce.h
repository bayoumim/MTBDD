#ifndef _RAW_QUEUE_AFTER_REDUCE_CLASS
#define _RAW_QUEUE_AFTER_REDUCE_CLASS
#include "BDDUtil.h"
#include "NewToRawLookUpTable.h"
#include "Request.h"
#include "RequestBucket.h"

// defined the returned values from the validate queue "after Apply" function
enum
{
   VQR_KEY_NOT_EXIST,         				/* 0 */
   VQR_FORWARDED_NODE_NOT_EXIST_IN_QUEUE,		/* 1 */
   VQR_VALID_KEY					/* 2 */
};

//template 
template<typename T,typename V,typename L,typename K,typename FLG>	// T and V are types of the fields in the BDDPointer
class RawQueueAfterReduce {
protected:	
    // private members
    T TotalCapacity; 	// determine the total capacity of the table
    RequestBucket <T,V,K,FLG> * Table; 		// pointer to the table
    unsigned char BucketSize;	// determine the size of the neighborhood
    T BucketMask;		// used to mask key
    NewToRawLookUpTable <T,V> * newToRawLookUpTable;

    // private functions
    // compute the hash function for the key (triple key)
    K hashSingleKey(K key){
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
    // reload queue from file no. "fileNo"
    void reloadData(int fileNo){
        char fName[40];
        sprintf(fName, "Queue Dump After Reduce %d .txt",fileNo);	// set file name
        ifstream outClientFile(fName);
        char temp[32];
        this->BucketSize = 32;
        outClientFile.getline(temp, 32);
        this->TotalCapacity = strtoul(temp,0,10);	// get total capacity
        Table = new RequestBucket <T,V,K,FLG> [TotalCapacity];
        outClientFile.getline(temp, 32);
        this->BucketMask  = strtoul(temp,0,10);	// get Bucket Mask
        T i;
        // fill up table
        for(i =0; i< TotalCapacity; i++){
            outClientFile.getline(temp, 32);	// get F
            T t = strtoul(temp,0,10);
            Table[i].setFrecord(t);

            outClientFile.getline(temp, 32);	// get G
            t = strtoul(temp,0,10);
            Table[i].setGrecord(t);

            outClientFile.getline(temp, 32);	// get H
            t = strtoul(temp,0,10);
            Table[i].setHrecord(t);

            outClientFile.getline(temp, 32);	// get HopInfo
            t = strtoul(temp,0,10);
            Table[i].setHopInforecord(t);

            outClientFile.getline(temp, 32);	// get Then
            t = strtoul(temp,0,10);
            Table[i].setThenrecord(t);

            outClientFile.getline(temp, 32);	// get Else
            t = strtoul(temp,0,10);
            Table[i].setElserecord(t);

            outClientFile.getline(temp, 32);	// get GroupData1
            t = strtoul(temp,0,10);
            Table[i].setGroupData1record(t);

            outClientFile.getline(temp, 32);	// get GroupData2
            t = strtoul(temp,0,10);
            Table[i].setGroupData2record(t);
        }
    }
public:
    // constructors
    RawQueueAfterReduce(){}

    RawQueueAfterReduce(int QueueNo, NewToRawLookUpTable <T,V> * newToRawLookUpTable){
        // reload the first file
        reloadData(QueueNo);
        this->newToRawLookUpTable = newToRawLookUpTable;
    }
    // this function validate whether a node with key "key" is exist in the raw table, and return a "raw id" for this key. if the key doesn't exist it returns 'false'
    int ValidateAndReturnRawID (RequestKey <T,V> key, BDDPointer <T,V> ForwardedNode) {
        T hashCode;
        hashCode = getHashCode( getHashString(key) );	// get hash code for this key

        RequestBucket <T,V,K,FLG> tempBucket = Table[hashCode];	// get the bucket
        T hopinfo = tempBucket.getHopInfo();	// get its hop information
        T nextNeighbor;
        while ( getNearestNeighbor(hashCode, hopinfo, nextNeighbor) ){	// get the nearest neighbor
            RequestKey <T,V> key1 = Table[nextNeighbor].getKey();
            BDDPointer <T,V> bp;
            Table[nextNeighbor].getNode().isForwarded(bp);
            if ( (key.isEqual(key1)) ){	// if you found the match,  store the pointer then return true
                if( !bp.isEqual(ForwardedNode) ) // if there isn't match in then pointer, return
                    return VQR_FORWARDED_NODE_NOT_EXIST_IN_QUEUE;
                else
                    return VQR_VALID_KEY;
            }
        }
        return VQR_KEY_NOT_EXIST;
    }
    ~RawQueueAfterReduce(){
        if(Table !=NULL)
            delete [] Table;
    }	
};
#endif 
