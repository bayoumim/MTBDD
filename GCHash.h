#ifndef _GC_HASH_CLASS
#define _GC_HASH_CLASS
#include "GCHashBucket.h"
#include "BDDUtil.hh"
#include "RWLock.hh"

//template 
class GCHash{
protected:	
	// private members
	int TotalCapacity; 	// determine the total capacity of the table
	GCHashBucket * Table; 		// pointer to the table
	int BucketSize;	// determine the size of the neighborhood
	int BucketMask;		// used to mask key
	RWLock mutex; 

	// private functions
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
	// compute the hash function for the key (triple key)
	int getHashCode(int key){
		int l = hashSingleKey(key);;
		l = l & (this->BucketMask); 	// round the key to be in the range
		return l; 
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
	// get no. of bits to represent a number
	int getNoOfBits (int Number) {
		int bitsCount = 0;
		do {
			bitsCount ++;
			Number >>= 1;
		}
		while ( Number!= 0);
		return bitsCount;
	}
	// convert 'n' to '2^n'
	int getPowerOf2(int number) {
		return 1<<(number);
	}

	// approximate teh number to higher value of 2
	int ApproximateToBiggerPowerOf2(int number){
		return getPowerOf2(getNoOfBits(number));
	}

	bool containsWithoutUnlocking( BDDPointer &bp,  int &nextNeighbor, int count) {
		int key = bp.getPointer();
		int hashCode = getHashCode(key);	// compute hash code
		nextNeighbor = hashCode;		// initialize the nextNeighbor		
		GCHashBucket tempBucket = Table[hashCode];	// get the bucket
		int hopinfo = tempBucket.getHopInfo();	// get its hop information		
		while ( getNearestNeighbor(hashCode, hopinfo, nextNeighbor) ){	// get the nearest neighbor 
			if ( (Table[nextNeighbor].getBP().isEqual(bp))  && (!Table[nextNeighbor].isEmpty())  ){	// if you found the match store the pointer unlock the read lock then return true
				Table[nextNeighbor].setCount( Table[nextNeighbor].getCount() + count);	// add "count" to the orignial number
				return true;
			}			
		}		
		// if you didn't find a match; then unlock the read lock then return false
		return false;
	}

	bool findNearestEmptyLocation(int start, int & location ){	
		if (start >= TotalCapacity) {	//if you reached the last place in the hash, return false
			// ADD CODE FOR RESIZING TABEL
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
	// upadate info field (this is required when swapping is exploited)
	void updateInfo(int bucket, int oldlocation, int newlocation){
		// we don't have to lock this function, as the programmer will lock it from outside
		if( (oldlocation < bucket) || (newlocation < bucket) ){
			cout<< "error in update info function";
			return;
		}
		int oldshift = BucketSize - (oldlocation - bucket) - 1;
		int newshift = BucketSize - (newlocation - bucket) - 1;
		int temp = Table[bucket].getHopInfo();
		temp = (temp ^ (1<<oldshift) ); // unset the old location
		temp = (temp ^ (1<<newshift) );	// set the new location
		Table[bucket].setHopInfo(temp);	// store the new value of Hop Information
		return;
	}
	// add a new item to the bucket list
	void AddNewItemInfo(int bucket, int itemLocation){	
		// debugging ADDNewItemInfo
		//cout << "bucket no. : " << bucket << endl;
		//cout << "insert element at. : " << itemLocation << endl;
		if (itemLocation < bucket)
			cout << "Invalid input : AddNewItemInfo Failed" << endl; 
		int shift = BucketSize - (itemLocation - bucket ) - 1;
		int temp = Table[bucket].getHopInfo();
		shift = (1<<shift);
		//cout << "shifts" << hex << shift << setbase (10) << endl;
		temp = (temp ^ shift ); // set the old location		
		Table[bucket].setHopInfo(temp);
		return;
	}
	// remove item from bucket list
	void removeItemInfo(int bucket, int itemLocation){	
		if (itemLocation < bucket)
			cout << "Invalid input : removeItemInfo Failed" << endl; 
		int shift = BucketSize - (itemLocation - bucket ) - 1;
		unsigned int temp = Table[bucket].getHopInfo();
		shift = (1<<shift);
		shift = shift ^ 0xFFFFFFFF;	// invert all bits
		temp = (temp & shift ); // reset the old location		
		Table[bucket].setHopInfo(temp);
		return;
	}

public:
	// constructors
	GCHash(){
	}

	GCHash(int inCapacity, int MaxUtilization){		
		this->BucketSize = 32;
	//	int MaxUtilization = 75;		
		this->TotalCapacity = (ApproximateToBiggerPowerOf2( ceil(inCapacity/MaxUtilization * 100))) << 2;	// get the total capacity (of the queue), then multiply it by 4
		Table = new GCHashBucket[TotalCapacity];	
		this->BucketMask = TotalCapacity - 1;
	}

	void findOrAdd(BDDPointer bp,bool & newBucketFlag, int & hashCode, int count){
		unsigned int key = bp.getPointer();
		hashCode = getHashCode(key);	// compute hash code

		int LastLoc;
		mutex.WriteLock();
		int info1 = Table[hashCode].getHopInfo();	// get info for this bucket

		if(info1 == 0){	// if there no other pointers in the bucket
			newBucketFlag = true;
		}
		else{
			newBucketFlag = false;		
		}

		if ( containsWithoutUnlocking(bp, LastLoc, count) ){ // if any resizing occurs the output of contains will be false, so there isn't any handling of resizing in this section
			mutex.Unlock();
			return ;	// the node is already there....
		}

		int locR = LastLoc;	// location obtained from read lock

		if( !findNearestEmptyLocation(LastLoc, locR) ){	// aquire read lock
			// this section is called when the thread didn't find a new location
			cout << "GCHash : the bucket is full you have to resize the table: Hash. In Find Nearest Neighbor read\n";
			exit(-1);
			return ;				// this value has no meaning, as the node doesn't exist or added
		}


	
		// the lock is aquired (write lock). and the segment is "Seg"
		int loc1;
		// hop the empty location all the way toward its bucket
		int emptyLoc = locR;
		int i;
		bool flag;
		while ( (emptyLoc - hashCode) >= BucketSize){
			flag = false;	// initialization for the next round
			//cout << "you are far from the desired bucket, in Hash" << endl;
			int start = emptyLoc - (BucketSize-1);
			for(i =  start; i < emptyLoc ; i++) {
				info1 = Table[i].getHopInfo();	// get info for this bucket
				while(getNearestNeighbor(i,info1,loc1)){      					                
					if (loc1 < emptyLoc) {	// if the bucket isn't reserved, you can swap
						flag = true;	//rise the flag						
						Table[emptyLoc].setNode(Table[loc1].getNode());
						Table[emptyLoc].setFlags(Table[loc1].getFlags());
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
				cout << "the bucket is full you have to resize the table: GCHash 1\n";
				cout << "program terminated" << endl;
				mutex.Unlock();
				exit(-1);
			}
		}			

		if ( (emptyLoc - hashCode) >= BucketSize){
			cout << "error in adding node : emptyLoc : " << emptyLoc << "hashCode : " << hashCode << endl; 
		}
	/*	if(key == 16777216){
			cout << "key : " << key << " is added at location : " <<  emptyLoc << endl; 
		}*/
		GCHashNode tempNode;
		tempNode.setCount(count);	// add raw id
		tempNode.setBP(bp);		// add raw id
		Table[emptyLoc].setNode(tempNode);

		AddNewItemInfo (hashCode,emptyLoc);	// update info		
		mutex.Unlock();
		return ;

	}
	// this function fetch any node at location 'bucket' 
	// it returns true when it find one node or more in this 'bucket', and false if there isn't any node stored in this 'bucket'
	bool fetchNodeAtBucket (int hashCode, GCHashNode &gcNode) {
		mutex.ReadLock();	// get lock
		GCHashBucket tempBucket = Table[hashCode];	// get the bucket
		int hopinfo = tempBucket.getHopInfo();	// get its hop information
		if (hopinfo == 0){	// if there isn't any other stored node , then unlock and return false
			mutex.Unlock();	// unlock
			return false;
		}
		int nextNeighbor;
		while ( getNearestNeighbor(hashCode, hopinfo, nextNeighbor) ){	// get the nearest neighbor
			if(!Table[nextNeighbor].isEmpty()){
				gcNode = Table[nextNeighbor].getNode();	// store the node
				removeItemInfo(hashCode,nextNeighbor);	// remove this element from the hop list
				Table[nextNeighbor].clearAllFlags();	// clear empty flag
				mutex.Unlock();				// unlock
				return true;
			}
			else{	// this is for debugging purposes only, it is an impossible case if the program runn correctly
				cout << "error in fetchNodeAtBucket: GCHash 1\n";
				cout << "program terminated" << endl;
				mutex.Unlock();
				exit(-1);
			}
		}		
		return false;
	}
};

#endif 
