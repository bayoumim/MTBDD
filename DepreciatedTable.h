#ifndef _DEPRECIATED_TABLE_CLASS
#define _DEPRECIATED_TABLE_CLASS
#include "BDDUtil.h"
#include "DepreciatedTableNode.h"
#include "NewToDepreciatedLookUpTable.h"

//template 
template<typename T,typename V> // T,V: Templates for DepreciatedTableNode
class DepreciatedTable{
protected:	
	// private members
	int TotalCapacity; 	// determine the total capacity of the table
	DepreciatedTableNode * Table; 		// pointer to the table
	int BucketSize;	// determine the size of the neighborhood
	int BucketMask;		// used to mask key

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

public:
	// constructors
	DepreciatedTable(){
	}

	DepreciatedTable (char *fName, NewToDepreciatedLookUpTable <T,V> * & lookUpTable){
		ifstream outClientFile(fName);
		char temp[32];
		this->BucketSize = 32;
		outClientFile.getline(temp, 32);
		this->TotalCapacity = strtoul(temp,0,10);	// get total capacity
		Table = new DepreciatedTableNode[TotalCapacity];
		outClientFile.getline(temp, 32);
		this->BucketMask  = strtoul(temp,0,10);	// get Bucket Mask
		int i;
		DepreciatedBDDPointer bp;

		// initiate look up table
		lookUpTable = new NewToDepreciatedLookUpTable<T,V> (TotalCapacity, BucketMask);

		// fill up table
		for(i =0; i< TotalCapacity; i++){
			outClientFile.getline(temp, 32);	// get hop info
			unsigned int t = strtoul(temp,0,10);
			Table[i].setHopInfo(t);

			outClientFile.getline(temp, 32);	// get id pointer
			bp.setPointer(strtoul(temp,0,10));
			outClientFile.getline(temp, 32);	// get id bucket
			bp.setBucket(strtoul(temp,0,10));
			Table[i].setIdP(bp);

			outClientFile.getline(temp, 32);	// get level
			Table[i].setLevel(strtoul(temp,0,10));

			outClientFile.getline(temp, 32);	// get then pointer
			bp.setPointer(strtoul(temp,0,10));
			outClientFile.getline(temp, 32);	// get then bucket
			bp.setBucket(strtoul(temp,0,10));
			Table[i].setThen(bp);			// get Then
			
			outClientFile.getline(temp, 32);	// get else pointer
			bp.setPointer(strtoul(temp,0,10));
			outClientFile.getline(temp, 32);	// get else bucket
			bp.setBucket(strtoul(temp,0,10));
			Table[i].setElse(bp);			// get else		
		}
	}
	// compute the hash function for the key (triple key)
	unsigned int getHashCode(DepreciatedNodeKey &key){
		int t = key.getThen();
		int e = key.getElse();
		int l = key.getLevel();
		t = hashSingleKey(t);
		e = hashSingleKey(t^e);
		l = hashSingleKey(l^e);
		l = l & (this->BucketMask); 	// round the key to be in the range
		return l; 
	}
	// this function validate whether a node with key "key" is exist in the raw table, and return a "raw id" for this key. if the key doesn't exist it returns 'false' 
	bool ValidateAndReturnRawID (DepreciatedNodeKey key, DepreciatedBDDPointer & id) {
		int hashCode;
		hashCode = getHashCode(key);	// get hash code for this key
		DepreciatedTableNode tempBucket = Table[hashCode];	// get the bucket
		int hopinfo = tempBucket.getHopInfo();	// get its hop information
		int nextNeighbor;
		while ( getNearestNeighbor(hashCode, hopinfo, nextNeighbor) ){	// get the nearest neighbor
		
			if ((key.isEqual(Table[nextNeighbor].getKey()))  ){	// if you found the match,  store the pointer unlock the read lock then return true
				id = Table[nextNeighbor].getIdP();	// store the id in the pointer
				return true;
			}			
		}		
		cout << "can't find the node: DepreciatedTable, ValidateAndReturnRawID" << endl; // this is an impossible situation if every thing is all right
		cout << "Total Capacity: " << TotalCapacity << endl;
		key.DumpToScreen();
		exit(-1);
		return false;
	}	
};

#endif 
