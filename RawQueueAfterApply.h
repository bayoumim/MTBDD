#ifndef _RAW_QUEUE_AFTER_APPLY_CLASS
#define _RAW_QUEUE_AFTER_APPLY_CLASS
#include "BDDUtil.h"
#include "NewToRawLookUpTable.h"
#include "Request.h"
#include "RequestBucket.h"

// defined the returned values from the validate queue "after Apply" function
enum
{
   VQ_KEY_NOT_EXIST,         		/* 0 */
   VQ_THEN_NOT_EXIST_IN_QUEUE,		/* 1 */
   VQ_ELSE_NOT_EXIST_IN_QUEUE,		/* 2 */
   VQ_VALID_KEY				/* 3 */
};

//template 
template<typename T,typename V,typename L,typename K,typename FLG>	// T and V are types of the fields in the BDDPointer
class RawQueueAfterApply{
protected:	
    // private members
    T TotalCapacity; 	// determine the total capacity of the table
    RequestBucket <T,V,K,FLG> * Table; 		// pointer to the table
    unsigned char BucketSize;	// determine the size of the neighborhood
    T BucketMask;		// used to mask key
    NewToRawLookUpTable <T,V> * lookUpTableRawQueueAfterApply;
    NewToRawLookUpTable <T,V> * newToRawLookUpTable;

    // private functions
    // compute the hash function for the key (triple key)
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
    void reloadData(int fileNo, NewToRawLookUpTable <T,V> * & lookUpTableRawQueueAfterApply){
        char fName[40];
        sprintf(fName, "Queue Dump After Apply %d .txt",fileNo);	// set file name
        ifstream outClientFile(fName);
        char temp[32];
        this->BucketSize = 32;
        outClientFile.getline(temp, 32);
        this->TotalCapacity = strtoul(temp,0,10);	// get total capacity
        Table = new RequestBucket <T,V,K,FLG> [TotalCapacity];
        outClientFile.getline(temp, 32);
        this->BucketMask  = strtoul(temp,0,10);	// get Bucket Mask
        T i;

        // initiate look up table
        this->lookUpTableRawQueueAfterApply = lookUpTableRawQueueAfterApply = new NewToRawLookUpTable <T,V> (TotalCapacity, BucketMask);

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
    RawQueueAfterApply(){}

    RawQueueAfterApply(int QueueNo, NewToRawLookUpTable <T,V> * & lookUpTableRawQueueAfterApply, NewToRawLookUpTable <T,V> * newToRawLookUpTable){
        // reload the first file
        reloadData(QueueNo, lookUpTableRawQueueAfterApply);
        this->newToRawLookUpTable = newToRawLookUpTable;
    }
    // this function validate whether a node with key "key" is exist in the raw table, and return a "raw id" for this key. if the key doesn't exist it returns 'false'
    int ValidateAndReturnRawID (Request <T,V,L> req, BDDPointer <T,V> & id) {
        RequestKey <T,V> key = req.getKey();
        T hashCode;
        hashCode = getHashCode(getHashString(key));	// get hash code for this key
        T hopinfo = Table[hashCode].getHopInfo();	// get its hop information
        T nextNeighbor;
        BDDPointer <T,V> bp;
        while ( getNearestNeighbor(hashCode, hopinfo, nextNeighbor) ){	// get the nearest neighbor
            RequestKey <T,V> key1 = Table[nextNeighbor].getKey();
            if ( (key.isEqual(key1)) ){	// if you found the match,  store the pointer unlock the read lock then return true
                id.setLocalId(Table[nextNeighbor].getRLocalId());	// store the id in the pointer
                id.setHashString(getHashString(key1));
                id.setTemporary();
                id.ClearInverted();

                bp = req.getR().getThen();
                if( !Table[nextNeighbor].getRThen().isEqual(bp) ) // if there isn't match in then pointer, return
                    return VQ_THEN_NOT_EXIST_IN_QUEUE;

                bp = req.getR().getElse();
                if( !Table[nextNeighbor].getRElse().isEqual(bp) ) // if there isn't match in else pointer, return
                    return VQ_ELSE_NOT_EXIST_IN_QUEUE;
                return VQ_VALID_KEY;
            }
        }
        return VQ_KEY_NOT_EXIST;
    }
    ~RawQueueAfterApply(){
        if(Table !=NULL)
            delete [] Table;
        if(lookUpTableRawQueueAfterApply !=NULL)
            delete lookUpTableRawQueueAfterApply;
    }	
};

#endif 
