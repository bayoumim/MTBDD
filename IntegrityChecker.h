////////////////////////////////////////////////////////////////////////////////////////////////////
// this class responsible for checking the validaty of the nodes
////////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef _INTEGRITY_CHECKER_CLASS
#define _INTEGRITY_CHECKER_CLASS

#include "BDDNode.h"
#include "DepreciatedBDDPointer.h"
#include "DepreciatedBDDNode.h"
#include "BDDPointer.h"
#include "BDDUtil.h"
#include "NewToDepreciatedLookUpTable.h"
#include "DepreciatedTable.h"
#include "RequestKey.h"
#include "DepreciatedQueue.h"
#include "DepreciatedQueueAfterReduce.h"
#include "DepreciatedRequest.h"
#include "DepreciatedQueueAfterApply.h"
#include "NewToRawLookUpTable.h"
#include "RawTable.h"
#include "RawQueue.h"
#include "RawQueueAfterApply.h"
#include "RawQueueAfterReduce.h"
#include "RawTableAfterReduce.h"
//#include "GCedRawTable.h"


template<typename T,typename V,typename L,typename K,typename FLG> // T,V: Templates for LookUpTableDepreciatedNode AND bdd pointer
class IntegrityChecker {
protected:
    NewToDepreciatedLookUpTable <T,V> * newToDepreciatedLookUpTable;
    DepreciatedTable <T,V> * depreciatedTable;
    DepreciatedQueue <T,V> * depreciatedQueue;
    DepreciatedQueueAfterReduce <T,V> * depreciatedQueueAfterReduce;
    DepreciatedQueueAfterApply <T,V> * depreciatedQueueAfterApply;
    NewToDepreciatedLookUpTable <T,V> * lookUpTableDepreciatedQueueAfterApply;
    NewToRawLookUpTable <T,V> * newToRawLookUpTable;
    RawTable <T,V,L,K> * rawTable;
    RawQueue <T,V,L,K,FLG> * rawQueue;
    NewToRawLookUpTable <T,V> * lookUpTableRawQueueAfterApply;
    RawQueueAfterApply <T,V,L,K,FLG> * rawQueueAfterApply;
    RawQueueAfterReduce <T,V,L,K,FLG> * rawQueueAfterReduce;
    RawTableAfterReduce <T,V,L,K> * rawTableAfterReduce;
//	GCedRawTable * gcedRawTable;
public:
    // constructors
    IntegrityChecker(int hashIntegrityCheck){
        char *fName = new char[40];
        if(hashIntegrityCheck == IC_VALIDATE_WITH_DEPRECIATED_HASH){
            sprintf(fName, "Hash Table Dump.txt");	// set file name
            depreciatedTable = new DepreciatedTable <T,V>(fName,newToDepreciatedLookUpTable);
        }
        else if ( (hashIntegrityCheck > IC_VALIDATE_WITH_DEPRECIATED_HASH) && (hashIntegrityCheck <= IC_VALIDATE_WITH_DEPRECIATED_QUEUE_AFTER_REDUCE) ){   // if you are validating with the depreciated code
            sprintf(fName, "Hash Table Dump.txt");	// set file name
            depreciatedTable = new DepreciatedTable <T,V> (fName,newToDepreciatedLookUpTable);
            if (hashIntegrityCheck == IC_VALIDATE_WITH_DEPRECIATED_QUEUE) {
                depreciatedQueue = new DepreciatedQueue <T,V>(0);
            }
            else if (hashIntegrityCheck == IC_VALIDATE_WITH_DEPRECIATED_QUEUE_AFTER_REDUCE){
                depreciatedQueueAfterReduce = new DepreciatedQueueAfterReduce <T,V> (0, newToDepreciatedLookUpTable);
            }
            else if( hashIntegrityCheck == IC_VALIDATE_WITH_DEPRECIATED_QUEUE_AFTER_APPLY){	// if the 'validate queue' is enabled, then, reload the first "queue after apply"
                depreciatedQueueAfterApply = new DepreciatedQueueAfterApply <T,V> (0,lookUpTableDepreciatedQueueAfterApply, newToDepreciatedLookUpTable);
            }
        }
        else if (hashIntegrityCheck == IC_VALIDATE_HASH_DURING_GC){
             /*   // if validation is during GC, then build a unique new-to-raw-lookup table that will map to all rounds in Apply and reduce. However, this will be valid only when the Unique table doesn't resize itself (during dump phase, because this will cause 'raw' pointer to change). if so, you have to find another way other than using single new-to-raw lookup table (nothing is wrong when unique table resizing is done in the 'validation' phase)
                sprintf(fName, "GCed Hash Table Dump.txt");	// set file name
                int TotalCapacity, BucketMask;
                gcedRawTable = new GCedRawTable(0,TotalCapacity, BucketMask);
                this->newToRawLookUpTable  = new NewToRawLookUpTable(TotalCapacity, BucketMask);*/
        }
        else if (hashIntegrityCheck <= IC_VALIDATE_WITH_DEPRECIATED_QUEUE_AFTER_REDUCE){
            sprintf(fName, "Hash Table Dump.txt");	// set file name
            rawTable = new RawTable <T,V,L,K>(fName,newToRawLookUpTable);
            if( hashIntegrityCheck == IC_VALIDATE_QUEUE){	// if the 'validate queue' is enabled, then, reload the first queue
                rawQueue = new RawQueue <T,V,L,K,FLG>(0);
            }
            else if( hashIntegrityCheck == IC_VALIDATE_QUEUE_AFTER_APPLY){	// if the 'validate queue' is enabled, then, reload the first "queue after apply"
                rawQueueAfterApply = new RawQueueAfterApply <T,V,L,K,FLG> (0,lookUpTableRawQueueAfterApply, newToRawLookUpTable);
            }
            else if( hashIntegrityCheck == IC_VALIDATE_QUEUE_AFTER_REDUCE){	// if the 'validate queue' is enabled, then, reload the first "queue after reduce"
                rawQueueAfterReduce = new RawQueueAfterReduce <T,V,L,K,FLG> (0, newToRawLookUpTable);
            }
        }
        else if (hashIntegrityCheck == IC_VALIDATE_HASH_TABLE_AFTER_REDUCE){
            // build look up table
            sprintf(fName, "Hash Total Capacity.txt");	// set file name
            ifstream outClientFile(fName);
            char temp[32];
            outClientFile.getline(temp, 32);
            T TotalCapacity = strtoul(temp,0,10);	// get total capacity
            outClientFile.getline(temp, 32);
            T BucketMask  = strtoul(temp,0,10);	// get Bucket Mask
            outClientFile.close();
            newToRawLookUpTable = new NewToRawLookUpTable <T,V> (TotalCapacity, BucketMask);
            
            // build the current table after reduce
            rawTableAfterReduce = new RawTableAfterReduce <T,V,L,K> (0);
        }
    }
    // map between new to depreciated ids
    DepreciatedBDDPointer Depreciatedmap(BDDPointer<T,V> bp){
        bool inv = bp.isInverted();
        if(inv){
            bp.invert();
        }
        DepreciatedBDDPointer temp;

        if(bp.isTemporary()){	// if the pointer is temperory, then map it from queue
            lookUpTableDepreciatedQueueAfterApply->getRawId(bp, temp);
        }
        else{				// else , map it from hash table
            newToDepreciatedLookUpTable->getRawId(bp, temp);
        }
        if(inv){
            temp.invert();
        }
        return temp;
    }
    // map between new to raw ids
    BDDPointer <T,V>  map(BDDPointer <T,V> bp){
        bool inv = bp.isInverted();
        if(inv){
            bp.invert();
        }
        BDDPointer <T,V> temp;

        if(bp.isTemporary())	// if the pointer is temperory, then map it from queue
            lookUpTableRawQueueAfterApply->getRawId(bp, temp);
        else{				// else , map it from hash table
            newToRawLookUpTable->getRawId(bp, temp);
        }
        if(inv){
            temp.invert();
        }
        return temp;
    }
    // this function reload data from queue no "queueNO", and it deallocate any previous raw queue
    void ReloadNextRawQueue(int queueNo){
        if (queueNo !=0){
            delete rawQueue;	// delete prevous queue
            rawQueue = new RawQueue <T,V,L,K,FLG> (queueNo);	// reload the next queue
        }
    }
    // this function reload data from "queue after apply" no "queueNO", and it deallocate any previous raw queue
    void ReloadNextRawQueueAfterApply(int queueNo){
        if (queueNo !=0){
            delete rawQueueAfterApply;	// delete prevous queue
            rawQueueAfterApply = new RawQueueAfterApply <T,V,L,K,FLG> (queueNo,lookUpTableRawQueueAfterApply, newToRawLookUpTable);	// reload the next queue
        }
    }
    // this function reload data from "queue after apply" no "queueNO", and it deallocate any previous raw queue
    void ReloadNextDepreciatedQueueAfterReduce(int queueNo){
        if (queueNo !=0){
            delete depreciatedQueueAfterReduce;	// delete prevous queue
            depreciatedQueueAfterReduce = new DepreciatedQueueAfterReduce <T,V>(queueNo, newToDepreciatedLookUpTable);	// reload the next queue
        }
    }
    // this function reload data from queue no "queueNO", and it deallocate any previous raw queue
    void ReloadNextDepreciatedQueue(int queueNo){
        if (queueNo !=0){
            delete depreciatedQueue;	// delete prevous queue
            depreciatedQueue = new DepreciatedQueue <T,V>(queueNo);	// reload the next queue
        }
    }
    // this function reload data from "queue after apply" no "queueNO", and it deallocate any previous raw queue
    void ReloadNextDepreciatedQueueAfterApply(int queueNo){
        if (queueNo !=0){
            delete depreciatedQueueAfterApply;	// delete prevous queue
            depreciatedQueueAfterApply = new DepreciatedQueueAfterApply <T,V> (queueNo,lookUpTableDepreciatedQueueAfterApply, newToDepreciatedLookUpTable);	// reload the next queue
        }
    }
    // this function reload data from "queue after apply" no "queueNO", and it deallocate any previous raw queue
    void ReloadNextRawQueueAfterReduce(int queueNo){
        if (queueNo !=0){
            delete rawQueueAfterReduce;	// delete prevous queue
            rawQueueAfterReduce = new RawQueueAfterReduce <T,V,L,K,FLG> (queueNo, newToRawLookUpTable);	// reload the next queue
        }
    }
    // this function reload data from "queue after apply" no "queueNO", and it deallocate any previous raw queue
    void ReloadNextHashTableAfterReduce(int queueNo){
        if (queueNo !=0){
            delete rawTableAfterReduce;	// delete prevous queue
            rawTableAfterReduce = new RawTableAfterReduce <T,V,L,K> (queueNo);	// reload the next queue
        }
    }
    void finalizeIntegrityChecker(){
        delete newToRawLookUpTable;
    }
    // check wether the input node is exist in the raw table or not, and store the node id in the look up table
    void validateUniqueTableNodeAfterReduce(BDDNode <T,V,L> node, BDDPointer <T,V> newId){
        BDDNode <T,V,L> oldNode = node;

        BDDPointer <T,V> Then = node.getThen();	// get new then pointer
        BDDPointer <T,V> Else = node.getElse();	// get new else pointer
        BDDPointer <T,V> t = map(Then);		// map new then pointer to new one
        BDDPointer <T,V> E = map(Else);		// map new then pointer to raw one
        node.setThen(t);				// set the raw then pointer
        node.setElse(E);				// set the raw else pointer

        BDDPointer <T,V> rawId;

        if (rawTableAfterReduce->ValidateAndReturnRawID(node,rawId)){	// store value in the look up table
            newToRawLookUpTable->findOrAdd(newId,rawId);
        }
        else{
            cout << "can't find the node: IntegrityChecker , validateUniqueTableNode. The Old node is" << endl; // this is an impossible situation if every thing is all right
            node.DumpToScreen();
            cout << "dump raw node" << endl;
            oldNode.DumpToScreen();
            exit(-1);
        } // print an error message
    }
    // check wether the input node is exist in the raw table or not, and store the node id in the look up table
    void validateUniqueTableNode(BDDNode <T,V,L> node, BDDPointer <T,V> newId){
        BDDNode <T,V,L> oldNode = node;
        
        BDDPointer <T,V> Then = node.getThen();	// get new then pointer
        BDDPointer <T,V>  Else = node.getElse();	// get new else pointer
        BDDPointer <T,V>  t = map(Then);		// map new then pointer to new one
        BDDPointer <T,V>  E = map(Else);		// map new then pointer to raw one
        node.setThen(t);				// set the raw then pointer
        node.setElse(E);				// set the raw else pointer

        BDDPointer <T,V> rawId;

        if (rawTable->ValidateAndReturnRawID(node,rawId)){	// store value in the look up table
            newToRawLookUpTable->findOrAdd(newId,rawId);
        }
        else{
            cout << "can't find the node: IntegrityChecker , validateUniqueTableNode. The Old node is" << endl; // this is an impossible situation if every thing is all right
            node.DumpToScreen();
            cout << "dump raw node" << endl;
            oldNode.DumpToScreen();
            exit(-1);
        } // print an error message
    }
    // check wether the key is exist in the raw queue
    void validateQueueNode(RequestKey <T,V> key){
        RequestKey <T,V> oldKey = key;

        BDDPointer <T,V> F = key.getF();	// get new F pointer
        BDDPointer <T,V> G = key.getG();	// get new G pointer
        BDDPointer <T,V> H = key.getH();	// get new H pointer

        BDDPointer <T,V> F1 = map(F);		// map new F pointer to new one
        BDDPointer <T,V> G1 = map(G);		// map new G pointer to new one
        BDDPointer <T,V> H1 = map(H);		// map new H pointer to new one

        key.setF(F1);				// set the F then pointer
        key.setG(G1);				// set the G else pointer
        key.setH(H1);				// set the H else pointer

        if (!rawQueue->Validate(key)){	// store value in the look up table
            cout << "can't find the node: IntegrityChecker , validateQueueNode. The Old node is" << endl; // this is an impossible situation if every thing is all right
            oldKey.DumpToScreen();
            exit(-1);
        } // print an error message
    }
    // check wether the key is exist in the raw queue (this function is called in find or add in hashqueue, to map between raw and new keys)
    void validateQueueKeyAfterApply(Request <T,V,L> req, BDDPointer <T,V> newId){
        Request <T,V,L>  oldReq = req;

        RequestKey <T,V> key = req.getKey();

        BDDPointer <T,V> F = key.getF();	// get new F pointer
        BDDPointer <T,V> G = key.getG();	// get new G pointer
        BDDPointer <T,V> H = key.getH();	// get new H pointer

        BDDPointer <T,V> F1 = map(F);		// map new F pointer to new one
        BDDPointer <T,V> G1 = map(G);		// map new G pointer to new one
        BDDPointer <T,V> H1 = map(H);		// map new H pointer to new one

        key.setF(F1);				// set the F then pointer
        key.setG(G1);				// set the G else pointer
        key.setH(H1);				// set the H else pointer

        req.setKey(key);		// set key after mapping
        BDDPointer <T,V> rawId;
        int returnedValue = rawQueueAfterApply->ValidateAndReturnRawID(req,rawId);
        switch(returnedValue){
            case VQ_KEY_NOT_EXIST:
                cout << "can't find the key: IntegrityChecker , validateQueueKeyAfterApply. The Old node is" << endl; // this is an impossible situation if every thing is all right
                req.DumpToScreen();
                cout << "new request" << endl;
                oldReq.DumpToScreen();
                exit(-1);
                break;
            case VQ_THEN_NOT_EXIST_IN_QUEUE:
            case VQ_ELSE_NOT_EXIST_IN_QUEUE:
            case VQ_VALID_KEY:
                lookUpTableRawQueueAfterApply->findOrAdd(newId,rawId);
                break;
        }
    }
    // check wether the input node is exist in the raw table or not, and store the node id in the look up table
    void validateQueueNodeAfterApply(Request <T,V,L> req, BDDPointer <T,V> newId, bool ThenProcessedFlag, bool ElseProcessedFlag, bool ThenProcessedInReduce, bool ElseProcessedInReduce){
        Request <T,V,L> oldReq = req;
        RequestKey <T,V> key = req.getKey();

        BDDPointer <T,V> F = key.getF();	// get new F pointer
        BDDPointer <T,V> G = key.getG();	// get new G pointer
        BDDPointer <T,V> H = key.getH();	// get new H pointer

        BDDPointer <T,V> F1 = map(F);		// map new F pointer to new one
        BDDPointer <T,V> G1 = map(G);		// map new G pointer to new one
        BDDPointer <T,V> H1 = map(H);		// map new H pointer to new one

        key.setF(F1);				// set the F then pointer
        key.setG(G1);				// set the G else pointer
        key.setH(H1);				// set the H else pointer

        req.setKey(key);		// set key after mapping

        if(ThenProcessedFlag && (!ThenProcessedInReduce)){
            F = req.getR().getThen();		// get Then pointer of node R
            F1 = map(F);			// map "Then pointer of node R"
            req.setThen(F1);		// set mapped Then
        }

        if(ElseProcessedFlag && (!ElseProcessedInReduce) ){
            G = req.getR().getElse();		// get Then pointer of node R
            G1 = map(G);			// map "Then pointer of node R"
            req.setElse(G1);		// set mapped Else
        }

        BDDPointer <T,V> rawId;
        int returnedValue = rawQueueAfterApply->ValidateAndReturnRawID(req,rawId);
        switch(returnedValue){
            case VQ_KEY_NOT_EXIST:
                cout << "can't find the key: IntegrityChecker , validateQueueNodeAfterApply. The Old node is" << endl; // this is an impossible situation if every thing is all right
                req.DumpToScreen();
                cout << "new request" << endl;
                oldReq.DumpToScreen();
                exit(-1);
                break;
            case VQ_THEN_NOT_EXIST_IN_QUEUE:
                if(ThenProcessedFlag && (!ThenProcessedInReduce) ){
                    cout << "can't find the Then pointer: IntegrityChecker , validateQueueNodeAfterApply. The Old node is" << endl; // this is an impossible situation if every thing is all right
                    req.DumpToScreen();
                    cout << "new request" << endl;
                    oldReq.DumpToScreen();
                    exit(-1);
                }
                break;
            case VQ_ELSE_NOT_EXIST_IN_QUEUE:
                if(ElseProcessedFlag && (!ElseProcessedInReduce) ){
                    cout << "can't find the Else pointer: IntegrityChecker , validateQueueNodeAfterApply. The Old node is" << endl; // this is an impossible situation if every thing is all right
                    req.DumpToScreen();
                    cout << "new request" << endl;
                    oldReq.DumpToScreen();
                    exit(-1);
                }
                break;
            case VQ_VALID_KEY:
                lookUpTableRawQueueAfterApply->findOrAdd(newId,rawId);
                break;
        }
    }
    // check wether the input node is exist in the raw table or not, and store the node id in the look up table
    void validateQueueNodeAfterReduce(RequestKey <T,V> & key, BDDPointer <T,V> ForwardedNode){
        RequestKey <T,V> oldkey = key;

        BDDPointer <T,V> F = key.getF();	// get new F pointer
        BDDPointer <T,V> G = key.getG();	// get new G pointer
        BDDPointer <T,V> H = key.getH();	// get new H pointer

        BDDPointer <T,V> F1 = map(F);		// map new F pointer to new one
        BDDPointer <T,V> G1 = map(G);		// map new G pointer to new one
        BDDPointer <T,V> H1 = map(H);		// map new H pointer to new one
        BDDPointer <T,V> FN = map(ForwardedNode); // map new forwarded node to raw one

        key.setF(F1);				// set the F then pointer
        key.setG(G1);				// set the G else pointer
        key.setH(H1);				// set the H else pointer

        int returnedValue = rawQueueAfterReduce->ValidateAndReturnRawID(key,FN);
        switch(returnedValue){
            case VQR_KEY_NOT_EXIST:
                cout << "can't find the key: IntegrityChecker , validateQueueNodeAfterReduce. The Old node is" << endl; // this is an impossible situation if every thing is all right
                key.DumpToScreen();
                cout << "new key" << endl;
                oldkey.DumpToScreen();
                exit(-1);
                break;
            case VQR_FORWARDED_NODE_NOT_EXIST_IN_QUEUE:
                cout << "can't find the forwarded node pointer: IntegrityChecker , validateQueueNodeAfterReduce. The Old node is" << endl; // this is an impossible situation if every thing is all right
                key.DumpToScreen();
                cout << "new key" << endl;
                oldkey.DumpToScreen();
                exit(-1);
                break;
            case VQR_VALID_KEY:
                // DO NOTHING
                break;
        }
    }
    // check wether the input node is exist in the depreciated table or not, and store the node id in the look up table
    void validateUniqueTableNodeWithDepreciated(BDDNode<T,V,L> node, BDDPointer <T,V> newId ){
        DepreciatedNodeKey oldNode;
        BDDPointer <T,V>Then = node.getThen();             // get new then pointer
        BDDPointer <T,V> Else = node.getElse();	// get new else pointer
        DepreciatedBDDPointer T1 = Depreciatedmap(Then);		// map new then pointer to new one
        DepreciatedBDDPointer E1 = Depreciatedmap(Else);		// map new then pointer to raw one
        oldNode.setThen(T1);				// set the depreciated then pointer
        oldNode.setElse(E1);				// set the depreciated else pointer
        oldNode.setLevel(node.getLevel());              // set the depreciated level

        DepreciatedBDDPointer rawId;

        if (depreciatedTable->ValidateAndReturnRawID(oldNode,rawId)){	// store value in the look up table
            newToDepreciatedLookUpTable->findOrAdd(newId,rawId);
        }
        else{
            cout << "can't find the node: IntegrityChecker , validateUniqueTableNodeWithDepreciated. The Old node is" << endl; // this is an impossible situation if every thing is all right
            node.DumpToScreen();
            cout << "dump raw node" << endl;
            oldNode.DumpToScreen();
            exit(-1);
        } // print an error message
    }
    // check wether the key is exist in the raw queue
    void validateDepreciatedQueueNode(RequestKey <T,V> key){
        DepreciatedRequestKey oldKey;

        BDDPointer <T,V> F = key.getF();	// get new F pointer
        BDDPointer <T,V> G = key.getG();	// get new G pointer
        BDDPointer <T,V> H = key.getH();	// get new H pointer

        DepreciatedBDDPointer F1 = Depreciatedmap(F);		// map new F pointer to new one
        DepreciatedBDDPointer G1 = Depreciatedmap(G);		// map new G pointer to new one
        DepreciatedBDDPointer H1 = Depreciatedmap(H);		// map new H pointer to new one

        oldKey.setF(F1);				// set the F then pointer
        oldKey.setG(G1);				// set the G else pointer
        oldKey.setH(H1);				// set the H else pointer

        if (!depreciatedQueue->Validate(oldKey)){	// store value in the look up table
            cout << "can't find the node: IntegrityChecker , validateQueueNode. The Old node is" << endl; // this is an impossible situation if every thing is all right
            oldKey.DumpToScreen();
            exit(-1);
        } // print an error message
    }

    // check wether the input node is exist in the raw table or not, and store the node id in the look up table
    void validateDepreciatedQueueNodeAfterReduce(RequestKey <T,V> & key, BDDPointer <T,V> ForwardedNode){
        DepreciatedRequestKey oldkey ;

        BDDPointer <T,V> F = key.getF();	// get new F pointer
        BDDPointer <T,V> G = key.getG();	// get new G pointer
        BDDPointer <T,V> H = key.getH();	// get new H pointer

        DepreciatedBDDPointer F1 = Depreciatedmap(F);		// map new F pointer to new one
        DepreciatedBDDPointer G1 = Depreciatedmap(G);		// map new G pointer to new one
        DepreciatedBDDPointer H1 = Depreciatedmap(H);		// map new H pointer to new one
        DepreciatedBDDPointer FN = Depreciatedmap(ForwardedNode); // map new forwarded node to raw one

        oldkey.setF(F1);				// set the F then pointer
        oldkey.setG(G1);				// set the G else pointer
        oldkey.setH(H1);				// set the H else pointer

        int returnedValue = depreciatedQueueAfterReduce->ValidateAndReturnRawID(oldkey,FN);
        switch(returnedValue){
            case VQR_KEY_NOT_EXIST:
                cout << "can't find the key: IntegrityChecker , vvalidateDepreciatedQueueNodeAfterReduce. The Old node is" << endl; // this is an impossible situation if every thing is all right
                key.DumpToScreen();
                exit(-1);
                break;
            case VQR_FORWARDED_NODE_NOT_EXIST_IN_QUEUE:
                cout << "can't find the forwarded node pointer: IntegrityChecker , validateDepreciatedQueueNodeAfterReduce. The Old node is" << endl; // this is an impossible situation if every thing is all right
                cout << "new key" << endl;
                key.DumpToScreen();
                cout << "depreciated key" << endl;
                oldkey.DumpToScreen();
                cout << "new forwarded pointer" << endl;
                ForwardedNode.DumpToScreen();
                cout << "depreciated forewarded pointer" << endl;
                FN.DumpToScreen();
                exit(-1);
                break;
            case VQR_VALID_KEY:
                // DO NOTHING
                break;
        }
    }
    // check wether the input node is exist in the raw table or not, and store the node id in the look up table
    void validateDepreciatedQueueNodeAfterApply(Request <T,V,L> req, BDDPointer <T,V> newId){
        DepreciatedRequest oldReq;
        DepreciatedRequestKey key;
        
        BDDPointer <T,V> F = req.getF();	// get new F pointer
        BDDPointer <T,V> G = req.getG();	// get new G pointer
        BDDPointer <T,V> H = req.getH();	// get new H pointer

        DepreciatedBDDPointer F1 = Depreciatedmap(F);		// map new F pointer to new one
        DepreciatedBDDPointer G1 = Depreciatedmap(G);		// map new G pointer to new one
        DepreciatedBDDPointer H1 = Depreciatedmap(H);		// map new H pointer to new one

        key.setF(F1);				// set the F then pointer
        key.setG(G1);				// set the G else pointer
        key.setH(H1);				// set the H else pointer

        oldReq.setKey(key);		// set key after mapping

        F = req.getR().getThen();		// get Then pointer of node R
        F1 = Depreciatedmap(F);			// map "Then pointer of node R"

        G = req.getR().getElse();		// get Then pointer of node R
        G1 = Depreciatedmap(G);			// map "Then pointer of node R"

        oldReq.setThen(F1);		// set mapped Then
        oldReq.setElse(G1);		// set mapped Else

        DepreciatedBDDPointer rawId;
        int returnedValue = depreciatedQueueAfterApply->ValidateAndReturnRawID(oldReq,rawId);
        switch(returnedValue){
            case VQ_KEY_NOT_EXIST:
                cout << "can't find the key: IntegrityChecker , validateDepreciatedQueueNodeAfterApply. The Old node is" << endl; // this is an impossible situation if every thing is all right
                req.DumpToScreen();
                exit(-1);
                break;
            case VQ_THEN_NOT_EXIST_IN_QUEUE:
                cout << "can't find the Then pointer: IntegrityChecker , validateDepreciatedQueueNodeAfterApply. The Old node is" << endl; // this is an impossible situation if every thing is all right
                req.DumpToScreen();
                exit(-1);
                break;
            case VQ_ELSE_NOT_EXIST_IN_QUEUE:
                cout << "can't find the Else pointer: IntegrityChecker , validateDepreciatedQueueNodeAfterApply. The Old node is" << endl; // this is an impossible situation if every thing is all right
                req.DumpToScreen();
                exit(-1);
                break;
            case VQ_VALID_KEY:
                lookUpTableDepreciatedQueueAfterApply->findOrAdd(newId,rawId);
                break;
        }
    }
    // check wether the key is exist in the raw queue (this function is called in find or add in hashqueue, to map between raw and new keys)
    void validateDepreciatedQueueKeyAfterApply(Request<T,V,L>  req, BDDPointer <T,V> newId){
        DepreciatedRequest oldReq;
        DepreciatedRequestKey key;

        BDDPointer <T,V> F = req.getF();	// get new F pointer
        BDDPointer <T,V> G = req.getG();	// get new G pointer
        BDDPointer <T,V> H = req.getH();	// get new H pointer

        DepreciatedBDDPointer F1 = Depreciatedmap(F);		// map new F pointer to new one
        DepreciatedBDDPointer G1 = Depreciatedmap(G);		// map new G pointer to new one
        DepreciatedBDDPointer H1 = Depreciatedmap(H);		// map new H pointer to new one

        key.setF(F1);				// set the F then pointer
        key.setG(G1);				// set the G else pointer
        key.setH(H1);				// set the H else pointer

        oldReq.setKey(key);		// set key after mapping

        DepreciatedBDDPointer rawId;
        int returnedValue = depreciatedQueueAfterApply->ValidateAndReturnRawID(oldReq,rawId);
        switch(returnedValue){
            case VQ_KEY_NOT_EXIST:
                cout << "can't find the key: IntegrityChecker , validateQueueKeyAfterApply. The Old node is" << endl; // this is an impossible situation if every thing is all right
                cout << "new request" << endl;
                req.DumpToScreen();
                cout << endl << endl << "depreciated request" << endl;
                oldReq.DumpToScreen();
                exit(-1);
                break;
            case VQ_THEN_NOT_EXIST_IN_QUEUE:
            case VQ_ELSE_NOT_EXIST_IN_QUEUE:
            case VQ_VALID_KEY:
                lookUpTableDepreciatedQueueAfterApply->findOrAdd(newId,rawId);
                break;
        }
    }

/*    //  store the node id in the look up table; otherwise return
    void validateUniqueTableNodeIfPossible(BDDNode node){
            NodeKey key = node.getKey();
            DepreciatedBDDPointer Then = key.getThenP();	// get new then pointer
            NodeKey oldKey = key;

            if(node.getIdP().getPointer() == 0xd000001){
                    cout << "IntegrityChecker ; validateUniqueTableNodeIfPossible " << endl;
            }

            DepreciatedBDDPointer Else = key.getElseP();	// get new else pointer
            DepreciatedBDDPointer T = map(Then);		// map new then pointer to new one
            DepreciatedBDDPointer E = map(Else);		// map new then pointer to raw one
            key.setThen(T);				// set the raw then pointer
            key.setElse(E);				// set the raw else pointer

            unsigned int newId = 0;
            DepreciatedBDDPointer rawId;

            if (gcedRawTable->ValidateAndReturnRawID(key,rawId)){	// store value in the look up table
                    newId = node.getId();
                    newToRawLookUpTable->findOrAdd(newId,rawId);
            }
    }
    bool CheckIfDeadNode(DepreciatedBDDPointer bp){
            // if the node doesn't exit, it is OK. But if the node is exits; so this is a problem, as this means that you will going to remove a node by accident
            bp = map(bp);	// get raw pointer

            return gcedRawTable->contains(bp);
    }





    // this function reload data from "GC-ed Hash Table, and it deallocate any previous hash Table
    void ReloadNextGCedHashTable(int roundNo){
            if (roundNo !=0){
                    delete gcedRawTable;	// delete prevous queue
                    int dumy1;
                    int dumy2;
                    gcedRawTable = new GCedRawTable(roundNo,dumy1,dumy2);	// reload the next queue
            }
    }*/
};

#endif 
