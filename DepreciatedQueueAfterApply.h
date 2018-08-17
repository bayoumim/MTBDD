#ifndef _DEPRECIATED_QUEUE_AFTER_APPLY_CLASS
#define _DEPRECIATED_QUEUE_AFTER_APPLY_CLASS
#include "BDDUtil.h"
#include "DepreciatedTableNode.h"
#include "NewToDepreciatedLookUpTable.h"
#include "DepreciatedQueueNodeAfterApply.h"
#include "DepreciatedRequest.h"

// defined the returned values from the validate queue "after Apply" function
enum
{
   DVQ_KEY_NOT_EXIST,         		/* 0 */
   DVQ_THEN_NOT_EXIST_IN_QUEUE,		/* 1 */
   DVQ_ELSE_NOT_EXIST_IN_QUEUE,		/* 2 */
   DVQ_VALID_KEY			/* 3 */
};

//template
template<typename T,typename V> // T,V: Templates for NewToDepreciatedLookUpTable
class DepreciatedQueueAfterApply{
protected:	
	// private members
	int TotalCapacity; 	// determine the total capacity of the table
	DepreciatedQueueNodeAfterApply  * Table; 		// pointer to the table
	int BucketSize;	// determine the size of the neighborhood
	int BucketMask;		// used to mask key
	NewToDepreciatedLookUpTable <T,V> * lookUpTableRawQueueAfterApply;
	NewToDepreciatedLookUpTable <T,V> * newToDepreciatedLookUpTable;

	// private functions
	// compute the hash function for the key (triple key)
	unsigned int getHashCode(DepreciatedRequestKey & key){
		int t = key.getThen();
		int e = key.getElse();
		int l = key.getLevel();
		t = hashSingleKey(t);
		e = hashSingleKey(t^e);
		l = hashSingleKey(l^e);
		l = l & (this->BucketMask); 
		return l; 
	}
	// hash single key
	int hashSingleKey(int key)
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
	bool getNearestNeighbor(int hashcode, int &hopinfo, int &location){
		if(hopinfo == 0){	// if hop information is zero return false
			return false;
		} 
		// if it does contain data, find the nearest neighbor
		unsigned int x = BucketSize /2;
		unsigned int highHalf,lowHalf;
		unsigned int hifo =hopinfo;
		unsigned int delta = hashcode + BucketSize -1;
		do{
			highHalf = (hifo >> x ) & ((unsigned int)( ( 1 << (x+1) ) - 1));
			lowHalf = hifo & ((unsigned int)( ( 1 << (x+1) ) - 1));
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
	void reloadData(int fileNo, NewToDepreciatedLookUpTable <T,V> * & lookUpTableRawQueueAfterApply){
		char fName[40];
		sprintf(fName, "Queue Dump After Apply %d .txt",fileNo);	// set file name		
		ifstream outClientFile(fName);
		char temp[32];
		this->BucketSize = 32;
		outClientFile.getline(temp, 32);
		this->TotalCapacity = strtoul(temp,0,10);	// get total capacity
		Table = new DepreciatedQueueNodeAfterApply[TotalCapacity];
		outClientFile.getline(temp, 32);
		this->BucketMask  = strtoul(temp,0,10);	// get Bucket Mask
		int i;
		DepreciatedBDDPointer bp;

		// initiate look up table
		this->lookUpTableRawQueueAfterApply = lookUpTableRawQueueAfterApply = new NewToDepreciatedLookUpTable <T,V>(TotalCapacity, BucketMask);

		// fill up table
		for(i =0; i< TotalCapacity; i++){
			if( i == 1050)
				cout << "" << endl;

			outClientFile.getline(temp, 32);	// get hop info
			unsigned int t = strtoul(temp,0,10);
			Table[i].setHopInfo(t);

			outClientFile.getline(temp, 32);	// get id pointer
			bp.setPointer(strtoul(temp,0,10));
			outClientFile.getline(temp, 32);	// get id bucket
			bp.setBucket(strtoul(temp,0,10));
			Table[i].setIdP(bp);

			outClientFile.getline(temp, 32);	// get f pointer
			bp.setPointer(strtoul(temp,0,10));
			outClientFile.getline(temp, 32);	// get f bucket
			bp.setBucket(strtoul(temp,0,10));
			Table[i].setF(bp);			// set f
			
			outClientFile.getline(temp, 32);	// get g pointer
			bp.setPointer(strtoul(temp,0,10));
			outClientFile.getline(temp, 32);	// get g bucket
			bp.setBucket(strtoul(temp,0,10));
			Table[i].setG(bp);			// set g		
			
			outClientFile.getline(temp, 32);	// get h pointer
			bp.setPointer(strtoul(temp,0,10));
			outClientFile.getline(temp, 32);	// get h bucket
			bp.setBucket(strtoul(temp,0,10));
			Table[i].setH(bp);			// set h		

			outClientFile.getline(temp, 32);	// get R Then pointer
			bp.setPointer(strtoul(temp,0,10));
			outClientFile.getline(temp, 32);	// get R Then bucket
			bp.setBucket(strtoul(temp,0,10));
			Table[i].setRThen(bp);			// set R Then
		
			outClientFile.getline(temp, 32);	// get R Else pointer
			bp.setPointer(strtoul(temp,0,10));
			outClientFile.getline(temp, 32);	// get R Else bucket
			bp.setBucket(strtoul(temp,0,10));
			Table[i].setRElse(bp);			// set R Else


		}
	}
public:
	// constructors
	DepreciatedQueueAfterApply(){
	}

	DepreciatedQueueAfterApply(int QueueNo, NewToDepreciatedLookUpTable <T,V> * & lookUpTableRawQueueAfterApply, NewToDepreciatedLookUpTable <T,V> * newToDepreciatedLookUpTable){
		// reload the first file
		reloadData(QueueNo, lookUpTableRawQueueAfterApply);
		this->newToDepreciatedLookUpTable = newToDepreciatedLookUpTable;
	}
	// this function validate whether a node with key "key" is exist in the raw table, and return a "raw id" for this key. if the key doesn't exist it returns 'false' 
	int ValidateAndReturnRawID (DepreciatedRequest req, DepreciatedBDDPointer & id) {
		DepreciatedRequestKey key = req.getKey();
		int hashCode;
		hashCode = getHashCode(key);	// get hash code for this key

		DepreciatedQueueNodeAfterApply tempBucket = Table[hashCode];	// get the bucket
		int hopinfo = tempBucket.getHopInfo();	// get its hop information
		int nextNeighbor;
		DepreciatedBDDPointer bp;
		while ( getNearestNeighbor(hashCode, hopinfo, nextNeighbor) ){	// get the nearest neighbor
			DepreciatedRequestKey key1 = Table[nextNeighbor].getKey();
			if ( (key.isEqual(key1)) ){	// if you found the match,  store the pointer unlock the read lock then return true
				id = Table[nextNeighbor].getIdP();	// store the id in the pointer

				bp = req.getRThen();
				if( !Table[nextNeighbor].getRThen().isEqual(bp) ) // if there isn't match in then pointer, return
					return DVQ_THEN_NOT_EXIST_IN_QUEUE;

				bp = req.getRElse();
				if( !Table[nextNeighbor].getRElse().isEqual(bp) ) // if there isn't match in else pointer, return
					return DVQ_ELSE_NOT_EXIST_IN_QUEUE;
								

				return DVQ_VALID_KEY;
			}			
		}		
		
	/*	cout << "can't find the node: RawTable, getInfo" << endl; // this is an impossible situation if every thing is all right
		cout << "Total Capacity: " << TotalCapacity << endl;
		key.DumpToScreen();
		exit(-1);*/
		return DVQ_KEY_NOT_EXIST;
	}
	~DepreciatedQueueAfterApply(){
		if(Table !=NULL)
			delete [] Table;
		if(lookUpTableRawQueueAfterApply !=NULL)
			delete lookUpTableRawQueueAfterApply;
	}
	
};

#endif 
