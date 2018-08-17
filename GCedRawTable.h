#ifndef _GC_RAW_TABLE_CLASS
#define _GC_RAW_TABLE_CLASS


#include "BDDUtil.hh"
#include "GCedRawTableNode.h"
#include "NewToRawLookUpTable.h"

//template 
class GCedRawTable{
protected:	
	// private members
	int TotalCapacity; 	// determine the total capacity of the table
	GCedRawTableNode * Table; 		// pointer to the table
	int BucketSize;	// determine the size of the neighborhood
	int BucketMask;		// used to mask key
//	int xxx;
//	NewToRawLookUpTable * newToRawLookUpTable;

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
	// reload hash table from file no. "fileNo"
	void reloadData(int fileNo , int &TotalCapacity, int & BucketMask){
	//	this->newToRawLookUpTable = newToRawLookUpTable;
		char fName[40];
		sprintf(fName, "GCed Hash Table Dump %d .txt",fileNo);	// set file name		
		ifstream outClientFile(fName);
		char temp[32];
		this->BucketSize = 32;
		outClientFile.getline(temp, 32);
		TotalCapacity = this->TotalCapacity = strtoul(temp,0,10);	// get total capacity
		Table = new GCedRawTableNode[TotalCapacity];	
		outClientFile.getline(temp, 32);
		BucketMask = this->BucketMask  = strtoul(temp,0,10);	// get Bucket Mask
		int i;
		unsigned char flags;

		BDDPointer bp;

		// initiate look up table
//		this->newToRawLookUpTable = newToRawLookUpTable = new NewToRawLookUpTable(TotalCapacity, BucketMask);

		// fill up table
		for(i =0; i< TotalCapacity; i++){
		//	if (i ==32767)
		//		Table[i].DumpToScreen();
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

			outClientFile.getline(temp, 32);	// get Flags
			flags = strtoul(temp,0,10);
			Table[i].setFlags(flags);			// get flags		

		}
	}
public:
	// constructors
	GCedRawTable(){
	}

	GCedRawTable(int fileNo, int &TotalCapacity, int & BucketMask){	
		// reload the first file
	//	xxx = fileNo;
		reloadData(fileNo, TotalCapacity, BucketMask);	
	}
	// compute the hash function for the key (triple key)
	int getHashCode(NodeKey &key){
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
	bool ValidateAndReturnRawID (NodeKey key, BDDPointer & id) {

		int hashCode;
		hashCode = getHashCode(key);	// get hash code for this key

		GCedRawTableNode tempBucket = Table[hashCode];	// get the bucket
		int hopinfo = tempBucket.getHopInfo();	// get its hop information
		int nextNeighbor;
		while ( getNearestNeighbor(hashCode, hopinfo, nextNeighbor) ){	// get the nearest neighbor
		
			if ( (key.isEqual(Table[nextNeighbor].getKey())) && (!Table[nextNeighbor].isEmpty()) ){	// if you found the match,  store the pointer unlock the read lock then return true
				id = Table[nextNeighbor].getIdP();	// store the id in the pointer
				return true;
			}			
		}		
		
	/*	cout << "can't find the node: GCedRawTable, ValidateAndReturnRawID" << endl; // this is an impossible situation if every thing is all right
		cout << "Total Capacity: " << TotalCapacity << endl;
		key.DumpToScreen();
		exit(-1);*/
		return false;
	}
	// check if the pointer is exist or not, this function is used with CheckIfDeadNode in IntegrityChecker class. if the node is exist then you will deref a wrong node , which is an error
	bool contains (BDDPointer bp) {
		int hashCode;
		hashCode = bp.getBucket();	// get hash code for this key

		GCedRawTableNode tempBucket = Table[hashCode];	// get the bucket
		int hopinfo = tempBucket.getHopInfo();	// get its hop information
		int nextNeighbor;
		while ( getNearestNeighbor(hashCode, hopinfo, nextNeighbor) ){	// get the nearest neighbor
		
			if ( ( bp.isEqual(Table[nextNeighbor].getIdP() )) && (!Table[nextNeighbor].isEmpty()) ){	// if you found the match,  store the pointer unlock the read lock then return true
				return true;
			}			
		}		
		
	/*	cout << "can't find the node: GCedRawTable, ValidateAndReturnRawID" << endl; // this is an impossible situation if every thing is all right
		cout << "Total Capacity: " << TotalCapacity << endl;
		key.DumpToScreen();
		exit(-1);*/
		return false;
	}
	~GCedRawTable(){
		if(Table !=NULL){
			delete [] Table;
	//		delete newToRawLookUpTable;
		}
	}
	
};

#endif 
