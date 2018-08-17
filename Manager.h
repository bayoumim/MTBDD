////////////////////////////////////////////////////////////////////////////////////////////////////////
// this class is responsible for all BDD operations, apply, reduce and managing the memory and threads 
////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _MANAGER_CLASS
#define _MANAGER_CLASS

#include "Hash.h"
#include "Queue.h"
#include "BFSThread.h"
#include "EdgeQueue.h"
#include "circuit.hh"
#include "OperationSchedular.h"
//#include "GCThread.h"
#include "InputVectors.h"

template<typename T,typename V,typename L,typename K,typename FLG>
class Manager {
private:
    bool enableComputedTable;
    Hash <T,V,L,K,FLG> * UniqueTable;	// pointer to unique table
    Queue <T,V,L,K,FLG> * queue;
    unsigned short ThreadsNo;
    L VariableCount;	// total number of variable
    BDDPointer <T,V> Zero;
    BDDPointer <T,V> One;
    BDDPointer <T,V> *Variable;	// pointers to variables, the index start from 0
    Barrier *barrier;	// barrier used to synchronize bfs threads between levels
    Barrier *GlobalBarrier;	// barrier used to synchronize all threads + manaager
    Barrier *GCBarrier;	// barrier used to synchronize all threads + manaager + GC (used only when GC is enabled)
    circuit *ckt;	// pointer to the circuit
    BDDPointer <T,V> *output;	// pointer to output list
    VariableCreator <T,V,L,K,FLG> *vc;
    TripleChecker <T,V,L,K,FLG> *tc; 	// triple checker
    BFSThread <T,V,L,K,FLG> ** threads;	// pointer to list of threads
//	GCThread **gcThreads;	// pointer to list of GC threads
    volatile bool TerminateThreads;			// this flag is used to terminate threads, this is done at finilize methods
    Dumper <T,V,L,K,FLG> * dumper;
    ofstream *outClientFile;
    Mutex mutex;
    volatile L startingLevel;	// shared variable, store the starting level of the requests enqued,
    volatile bool hybridFlag; 	// this flag is used to indicate whether the hybrid approach during some rounds will be utilized or not ( and this would depends only on number of requests needs to be processed).
    unsigned char hashIntegrityCheck;
    IntegrityChecker <T,V,L,K,FLG> * integrityChecker;
    unsigned char GCEnabled;
//	GCQueue * gcQueue;
    unsigned short GCThreadsNumber;
    OperationSchedular <T,V,L,K,FLG> * opSchd;
//	NewToRawLookUpTable * CUDDtoMTLookUpTable ; // used when validating the package with CUDD
    unsigned char ResizeType;
////////////////////////////////////////////////////////////////////////////////////////////////////////
// 				PRIVATE FUNCTIManagerConditionONS
////////////////////////////////////////////////////////////////////////////////////////////////////////
    // it check if the requests is terminal case for the terminal case
    bool isTerminalCaseForInvertedEdge  (BDDPointer <T,V> &F, BDDPointer <T,V> &G, BDDPointer <T,V> &H, BDDPointer <T,V> & R) {
        return tc->isTerminalCaseForInvertedEdge(F, G, H, R);
    }
    BDDPointer <T,V>  CreateOne () {
        return vc->getOne();
    }
    BDDPointer <T,V>  CreateZero () {
        return vc->getZero();
    }
    // get the forwarded pointer, this function return a pointer in the hash table corresponding to a forwarded pointer in the queue
    void getElement(BDDPointer <T,V> &bp){
        if (bp.isTemporary()) {	// if it is a temporary pointer, then get it from queue, otherwise, do nothing
            queue-> getElement (bp);	// get the forwarded element
        }
    }
    // check if BDD Node is inverted
    bool isInverted (BDDPointer  <T,V> &f) {
        return f.isInverted();
    }
    BDDPointer <T,V> getRegularEdge (BDDPointer <T,V> &f) {
        return f.ClearInverted();
    }
    // put the triple in the standard form, and return true if the ouptput is inverted
    bool putRequestInStandardTriple(BDDPointer <T,V> &  F, BDDPointer <T,V> & G, BDDPointer <T,V> & H) {
        return tc->putRequestInStandardTriple(F,G,H);
    }
    // intinitiate the class (this function is seperated because c++ doesn't support constructor chaining)
    void Initiate(T inCapacity,
                    unsigned char HashMaxUtilization,
                    L VariableCount,
                    double QueueMaxUtilization,
                    unsigned short ThreadsNumber,
                    unsigned short HashConcurrrecyDegree,
                    unsigned short QueueConcurrencyDegree,
                    unsigned char HashBucketSize,
                    unsigned char QueueBucketSize,
                    bool enableComputedTable,
                    T QueueSize,
                    unsigned char hashIntegrityCheck,
                    unsigned char GCEnabled ,
                    unsigned short GCThreadsNumber,
                    unsigned char VariableReordering,
                    unsigned char ResizeType){
        
        this->ResizeType = ResizeType;
        this->GCEnabled = GCEnabled;
        this->enableComputedTable = enableComputedTable;
        this->VariableCount = VariableCount;
        this->ThreadsNo = ThreadsNumber;
        this->GCThreadsNumber = GCThreadsNumber;
        if (ThreadsNumber <= 0){
            cout << "Error : invalid no. of threads" << endl;
            exit(-1);
        }
        if (    (hashIntegrityCheck == IC_VALIDATE_WITH_DEPRECIATED_QUEUE_AFTER_APPLY ) ||
                (hashIntegrityCheck == IC_VALIDATE_WITH_DEPRECIATED_QUEUE_AFTER_REDUCE ) ||
                (hashIntegrityCheck == IC_VALIDATE_WITH_DEPRECIATED_QUEUE ) ||
                (hashIntegrityCheck == IC_VALIDATE_WITH_DEPRECIATED_HASH ) ||
                (hashIntegrityCheck == IC_VALIDATE_HASH ) ||
                (hashIntegrityCheck == IC_VALIDATE_QUEUE) ||
                (hashIntegrityCheck == IC_VALIDATE_QUEUE_AFTER_APPLY) ||
                (hashIntegrityCheck == IC_VALIDATE_QUEUE_AFTER_REDUCE) ||
                (hashIntegrityCheck == IC_VALIDATE_HASH_TABLE_AFTER_REDUCE) ||
                (hashIntegrityCheck == IC_VALIDATE_HASH_DURING_GC) ) {	// if "validate hash" or "validate queue" option is enabled
            integrityChecker = new IntegrityChecker <T,V,L,K,FLG>(hashIntegrityCheck);
        }
        // allocate queue for GC
//	GCQueue * gcQueue;
/*	if (GCEnabled == H_GC_RF_ENABLED){	// RC GC Case
                if (GCThreadsNumber <= 0){
                        cout << "Error : invalid no. of GC threads" << endl;
                        exit(-1);
                }
                this->gcQueue = gcQueue = new GCQueue(idGen);
                GCBarrier = new Barrier (ThreadsNumber+GCThreadsNumber+1);	// barrier for all bfs threads + manager + GC threads
        }
        else if ( (GCEnabled == H_GC_MARK_ENABLED) || (GCEnabled == H_GC_INCREMENTAL_MARK_ENABLED) ) {	// Mark GC & Incremental Mark Case
                if (GCThreadsNumber <= 0){
                        cout << "Error : invalid no. of GC threads" << endl;
                        exit(-1);
                }
                GCBarrier = new Barrier (ThreadsNumber+GCThreadsNumber+1);	// barrier for all bfs threads + manager + GC threads
        }*/

        UniqueTable = new Hash <T,V,L,K,FLG>(inCapacity,
                                            HashConcurrrecyDegree,
                                            HashMaxUtilization,
                                            HashBucketSize,
                                            hashIntegrityCheck,
                                            integrityChecker,
                                            GCEnabled /*, gcQueue*/,
                                            ResizeType,
                                            One);	// allocate hash table
        //UniqueTable->Dump();	// for debugging purposes

        queue = new Queue<T,V,L,K,FLG>(QueueSize,
                                        QueueConcurrencyDegree,
                                        QueueMaxUtilization,
                                        QueueBucketSize,
                                        enableComputedTable,
                                        VariableCount,
                                        hashIntegrityCheck,
                                        integrityChecker,
                                        GCEnabled /*, gcQueue*/,
                                        UniqueTable); // allocate and initialize queue
        unsigned short i;
        // THIS SECTION WILL BE MODIFIED WHEN THE OVERHEAD FREE GC IS IMPLEMENTED (AS IT DOESN'T USE GC THREADS AT ALL)
/*	if (GCEnabled != H_NO_GC){
                // create GC threads
                gcThreads = (GCThread **) malloc (sizeof(GCThread*) * GCThreadsNumber) ;	// allocate pointers to threads
                // create and run threads
                for (i = 0; i < GCThreadsNumber ; i++) {
                        gcThreads[i] =  new GCThread (i, UniqueTable, gcQueue, GCBarrier,GCEnabled, queue);
                }

        }*/

        this->vc = new VariableCreator <T,V,L,K,FLG> ( VariableCount,UniqueTable);
	if(VariableReordering == VR_WEIGHTED_HEURISTIC ){	// if the heuristic weight is enabled
                L * variableLookUpTable;
                variableLookUpTable = heuristicWeightVariableReordering();	// get the variable look up table
                vc->setVarLookUpTable(variableLookUpTable);			// set the new lookup table
        }
        DumpVariableOrder();

        // create and store the Zero and One Nodes
        One = CreateOne ();
        Zero = CreateZero ();
        // initialize the triple checker
        this->tc = new TripleChecker <T,V,L,K,FLG> (Zero, One, UniqueTable);
        // initiate barriers
        barrier = new Barrier (ThreadsNumber);
        GlobalBarrier = new Barrier (ThreadsNumber+1);	// barrier for all bfs threads + manager

        this-> Variable= vc->getVariablePointer();	// get a pointer to variables
        // initialize conditions
        TerminateThreads = false;

        //
        dumper =  new Dumper <T,V,L,K,FLG> (ThreadsNumber,
                                            VariableCount,
                                            Zero,
                                            One,
                                            Variable,
                                            UniqueTable,
                                            queue,
                                            output,
                                            vc );
        // initialize threads
        threads = (BFSThread<T,V,L,K,FLG> **) malloc (sizeof(BFSThread<T,V,L,K,FLG>*) * ThreadsNo) ;	// allocate pointers to threads
        // create and run threads
        for (i = 0; i < ThreadsNo ; i++) {
            threads[i] =  new BFSThread <T,V,L,K,FLG> (i,
                                                        UniqueTable,
                                                        queue,
                                                        VariableCount,
                                                        Zero,
                                                        One,
                                                        *barrier,
                                                        enableComputedTable,
                                                        tc,
                                                        TerminateThreads,
                                                        GlobalBarrier,
                                                        mutex,
                                                        startingLevel,
                                                        hybridFlag,
                                                        hashIntegrityCheck,
                                                        GCEnabled,
                                                        GCBarrier);
        }
        this->hashIntegrityCheck = hashIntegrityCheck;
        // create conditon for threads and manager
//		Dump();
    }
    void Initiate (circuit *ckt,
                    T inCapacity,
                    unsigned char HashMaxUtilization,
                    unsigned char QueueMaxUtilization,
                    unsigned short ThreadsNumber,
                    unsigned short HashConcurrrecyDegree,
                    unsigned short QueueConcurrencyDegree,
                    unsigned char HashBucketSize,
                    unsigned char QueueBucketSize,
                    bool enableComputedTable,
                    T QueueSize,
                    unsigned char hashIntegrityCheck,
                    unsigned char GCEnabled,
                    unsigned short GCThreadsNumber,
                    unsigned char VariableReordering,
                    unsigned char ResizeType){
        
        this->ckt = ckt;	// store circuit
        L VariableCount = ckt->getNumInput();	// get number of input
        Initiate( inCapacity,
                    HashMaxUtilization,
                    VariableCount,
                    QueueMaxUtilization,
                    ThreadsNumber,
                    HashConcurrrecyDegree,
                    QueueConcurrencyDegree,
                    HashBucketSize,
                    QueueBucketSize,
                    enableComputedTable,
                    QueueSize,
                    hashIntegrityCheck,
                    GCEnabled,
                    GCThreadsNumber ,
                    VariableReordering,
                    ResizeType);	// call the regular constructor

    }
    // get hybrid flag
    bool getHybridFlag(){
        bool temp;
        mutex.lock();
        temp =	hybridFlag;
        mutex.unlock();
        return temp;
    }
    // simulate the circuit for single input
    char * SimulateCircuitForInput(char * inputvec){
        int NumOfOutput = ckt-> getNumoutput();
        int i;
        char * Output = new char[NumOfOutput+1];
        Output[NumOfOutput] = 0;
        for(i=0; i<NumOfOutput; i++){
//			if (i == 13)
//			cout << "manager : SimulateCircuitForInput" <<endl;
//		cout << "manager : SimulateCircuitForInput : simulating output no. : " << i << endl;
            Output[i] = SimulateBDD(ckt->getGateOutputNum(i),inputvec);
        }
        return Output;
    }
    // simulate one BDD (the index start from 1)
    char SimulateBDD(int GateNum, char * inputvec){
        BDDPointer <T,V> bp = opSchd->getGateNum(GateNum);
        return UniqueTable->SimulateBDD(bp,inputvec);
    }
public:
    // constructor
    Manager (){
    }
    // overloaded constructor, this constructor is used when we want to construct one operation only
    Manager(T inCapacity,
                unsigned char HashMaxUtilization,
                L VariableCount,
                unsigned char QueueMaxUtilization,
                unsigned short ThreadsNumber,
                unsigned short HashConcurrrecyDegree,
                unsigned short QueueConcurrencyDegree,
                unsigned char HashBucketSize,
                unsigned char QueueBucketSize,
                bool enableComputedTable,
                T QueueSize,
                unsigned char hashIntegrityCheck,
                unsigned char GCEnabled,
                unsigned short GCThreadsNumber,
                unsigned char VariableReordering,
                unsigned char ResizeType){

            Initiate (inCapacity,
                        HashMaxUtilization,
                        VariableCount,
                        QueueMaxUtilization,
                        ThreadsNumber,
                        HashConcurrrecyDegree,
                        QueueConcurrencyDegree,
                        HashBucketSize,
                        QueueBucketSize,
                        enableComputedTable,
                        QueueSize,
                        hashIntegrityCheck,
                        GCEnabled,
                        GCThreadsNumber,
                        VariableReordering,
                        ResizeType);
    }
    // overloaded constructor, by passing the whole circuit object
    Manager(circuit *ckt,
                T inCapacity,
                unsigned char HashMaxUtilization,
                unsigned char QueueMaxUtilization,
                unsigned short ThreadsNumber,
                unsigned short HashConcurrrecyDegree,
                unsigned short QueueConcurrencyDegree,
                unsigned char HashBucketSize,
                unsigned char QueueBucketSize,
                bool enableComputedTable,
                T QueueSize,
                unsigned char hashIntegrityCheck,
                unsigned char GCEnabled,
                unsigned short GCThreadsNumber,
                unsigned char VariableReordering,
                unsigned char ResizeType){

            this->ckt = ckt;	// store circuit
            L VariableCount = ckt->getNumInput();	// get number of input
            Initiate( inCapacity,
                        HashMaxUtilization,
                        VariableCount,
                        QueueMaxUtilization,
                        ThreadsNumber,
                        HashConcurrrecyDegree,
                        QueueConcurrencyDegree,
                        HashBucketSize,
                        QueueBucketSize,
                        enableComputedTable,
                        QueueSize,
                        hashIntegrityCheck,
                        GCEnabled,
                        GCThreadsNumber,
                        VariableReordering,
                        ResizeType);	// call the regular constructor
    }
    // Manager, by passing the circuit name
    Manager(char *cktName,
                T inCapacity,
                unsigned char HashMaxUtilization,
                unsigned char QueueMaxUtilization,
                unsigned short ThreadsNumber,
                unsigned short HashConcurrrecyDegree,
                unsigned short QueueConcurrencyDegree,
                unsigned char HashBucketSize,
                unsigned char QueueBucketSize,
                bool enableComputedTable,
                T QueueSize,
                unsigned char hashIntegrityCheck,
                unsigned char GCEnabled,
                unsigned short GCThreadsNumber,
                unsigned char VariableReordering,
                unsigned char ResizeType){

            ckt = new circuit(cktName);	// create a circuit object
            Initiate(ckt,
                        inCapacity,
                        HashMaxUtilization,
                        QueueMaxUtilization,
                        ThreadsNumber,
                        HashConcurrrecyDegree,
                        QueueConcurrencyDegree,
                        HashBucketSize,
                        QueueBucketSize,
                        enableComputedTable,
                        QueueSize,
                        hashIntegrityCheck,
                        GCEnabled,
                        GCThreadsNumber,
                        VariableReordering,
                        ResizeType);			// call the constructor

    }
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    //				Logical Operations
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////
    // CREATE SINGLE OUPUT BDD
    ////////////////////////////
    // Perform BF ITE algorithm
    BDDPointer <T,V>  BfIte (BDDPointer <T,V>  F, BDDPointer <T,V>  G, BDDPointer <T,V>  H) {
        BDDPointer <T,V> bp;	// POINTER TO THE RESULT OF THE OPERATION , THIS COULD BE CHANGE TO AN ARRAY; IF MANY OPERATIONS ARE ISSUED IN THE SAME TIME

        //int i;
        if (isTerminalCaseForInvertedEdge ( F, G, H, bp) ) {
            return bp;
        }
        else {
            // THE MANAGEER CREATE A REUEST OR SOME REQUESTS , THEN ENQUE THEM TO THE QUEUE, PRESERVE A POINTER FOR EACH REQUEST, CALL APPLY, REDUCE FUNCTION, FINALLY RETRIVE A FORWARDED REFERENCE TO EACH POINTER
            // create request
            int MaxIndex = this->queue->getTopVarIndex(F,G,H);	// get the top variable
            BDDNode <T,V,L> R;
            R.setLevel (MaxIndex);
            R.clearForwarded();
            R.clearForwarded();

            bool inv = putRequestInStandardTriple(F,G,H);	// put the triple in the standard form, and store whether the pointer to operation is inverted or not
            Request <T,V,L> req ( F, G, H, R);	// create a request
            int temp = 0;

            queue->findOrEnqRequest( req, bp, temp, temp);		// enque it

            // wait until all threads are finished (barrier)
            GlobalBarrier -> wait();	// the first wait (now apply in bfs thread can run)
            GlobalBarrier -> wait();	// the second wait (now apply  is finished , and manager can start reloading the queue)

            //queue->ReLoad ();


            // get the forwarded pointer to the node in the hash table
            getElement(bp);
            if (inv){	// if the pointer is inverted
                bp.invert();
            }
            return bp;
        }
    }

    // perform AND operation
    BDDPointer <T,V> And(BDDPointer <T,V> &f, BDDPointer <T,V>  &g){
        BDDPointer <T,V> result = BfIte (f,g,Zero);	// ITE(F,G,0) = F AND G
        return result;
    }
    // perform OR operation
    BDDPointer <T,V> Or(BDDPointer <T,V> &f, BDDPointer <T,V>  &g){
        BDDPointer <T,V> result = BfIte (f,One,g);	// ITE(F,1,G) = F OR G
        return result;
    }
    // perform NOT operation
    BDDPointer <T,V> Not(BDDPointer <T,V> &f){
        BDDPointer <T,V> result = f;
        result.invert();	// ITE(F,0,1) = NOT(F)
        return result;
    }

    BDDPointer <T,V> Xor(BDDPointer <T,V> &f, BDDPointer <T,V>  &g){
        BDDPointer <T,V> result = Not (g);	// NOT(G)
        result = BfIte (f,result,g);	// ITE(F,NOT(G),G) = F XOR G
        return result;
    }
    // perform NOR operation
    BDDPointer <T,V> Nor(BDDPointer <T,V> &f, BDDPointer <T,V>  &g){
        BDDPointer <T,V> result = Not (g);	// NOT(G)
        result = BfIte (f,Zero,result);	// ITE(F,0,NOT(G)) = F NOR G
        return result;
    }
    // perform NAND operation
    BDDPointer <T,V> Nand(BDDPointer <T,V> &f, BDDPointer <T,V>  &g){
        BDDPointer <T,V> result = Not (g);	// NOT(G)
        result = BfIte (f,result,One);	// ITE(F,NOT(G),1) = F NAND G
        return result;
    }
    // perform XNOR operation
    BDDPointer <T,V> Xnor(BDDPointer <T,V> &f, BDDPointer <T,V>  &g){
        BDDPointer <T,V> result = Not (g);	// NOT(G)
        result = BfIte (f,g,result);	// ITE(F,NOT(G),G) = F XOR G
        return result;
    }
    ///////////////////////////////////////
    // CREATE THE WHOLE CIRCUIT OUPUTS BDD
    ///////////////////////////////////////

    void getOutputCktBfIte () {
        int i,InputCounter, outputCounter;
        BDDPointer <T,V> bp;	// POINTER TO THE RESULT OF THE OPERATION , THIS COULD BE CHANGE TO AN ARRAY; IF MANY OPERATIONS ARE ISSUED IN THE SAME TIME
        InputCounter = 0;
        outputCounter = 0;
        L numLevels = ckt->getNoOfLevel();	// get number of levels
        int MaxNoOfGateInALevel = ckt->getMaxNoOfGatesInLevel();
        int MaxNoOfInputsInAGate = ckt->getMaxInputInGate();
        opSchd = new OperationSchedular <T,V,L,K,FLG>(  MaxNoOfGateInALevel,
                                                        MaxNoOfInputsInAGate,
                                                        ckt,
                                                        queue,
                                                        vc,
                                                        tc,
                                                        dumper,
                                                        UniqueTable,
                                                        startingLevel,
                                                        GCEnabled/*,gcQueue*/);	// create operation schedular
        ckt->ResetGateCounter();	// reset the gate index counter
        int fileNo = 0;	// counter to file that store the dump of queues
        int fileNo1 = 0;
        hybridFlag = false;
        for (i=0;i < numLevels; i++) {	// for each level in the circuit
            opSchd->initiateLevel(i);	// initiate level
            do{
                startingLevel = 0;

                // if the VALIDATE queue is enabled
                if( hashIntegrityCheck == IC_VALIDATE_WITH_DEPRECIATED_QUEUE){
                    integrityChecker->ReloadNextDepreciatedQueue(fileNo1);
                }
                else if( hashIntegrityCheck == IC_VALIDATE_WITH_DEPRECIATED_QUEUE_AFTER_REDUCE){
                    integrityChecker->ReloadNextDepreciatedQueueAfterReduce(fileNo1);
                }
                else if (hashIntegrityCheck == IC_VALIDATE_WITH_DEPRECIATED_QUEUE_AFTER_APPLY ){
                    integrityChecker->ReloadNextDepreciatedQueueAfterApply(fileNo1);
                }
                else if( hashIntegrityCheck == IC_VALIDATE_QUEUE){
                    integrityChecker->ReloadNextRawQueue(fileNo1);
                }
                else if( hashIntegrityCheck == IC_VALIDATE_QUEUE_AFTER_APPLY){
                    integrityChecker->ReloadNextRawQueueAfterApply(fileNo1);
                }
                else if( hashIntegrityCheck == IC_VALIDATE_QUEUE_AFTER_REDUCE){
                    integrityChecker->ReloadNextRawQueueAfterReduce(fileNo1);
                }
                else if( hashIntegrityCheck == IC_VALIDATE_HASH_TABLE_AFTER_REDUCE){
                    integrityChecker->ReloadNextHashTableAfterReduce(fileNo1);
                }
          /*      else if( hashIntegrityCheck == IC_VALIDATE_HASH_DURING_GC){
                        cout << "relaod hash no. " << fileNo1 << endl;
                        integrityChecker->ReloadNextGCedHashTable(fileNo1);
                }*/

                opSchd->NextRound();

                do{	// Hybrid approach loop.
                    queue->ReLoad ();
                    queue->resetFreeNodeCounter();

                    hybridFlag = false;
                    // RUN THREAD
                    UniqueTable->checkSegmentsConsistancy();
                    GlobalBarrier -> wait();	// the first wait (now apply in bfs thread can run)
                    GlobalBarrier -> wait();	// the second wait (now apply  is finished , and manager can start reloading the queue)

                    queue->ReLoad ();

                    // if the dump queue is enabled
                    if( hashIntegrityCheck == IC_DUMP_QUEUE){
                        queue->DumpQueue(fileNo);
                    }
                    else if( hashIntegrityCheck == IC_DUMP_QUEUE_AFTER_APPLY) {
                        queue->DumpQueueAfterApply(fileNo);
                    }

                    GlobalBarrier -> wait();	// the third wait (now reduce in bfs thread can run)
                    GlobalBarrier -> wait();	// the second wait (now reduce  is finished , and manager can start reloading the queue, and regulate the pointers)

                    if( hashIntegrityCheck == IC_DUMP_QUEUE_AFTER_REDUCE) {
                        queue->DumpQueueAfterReduce(fileNo);
                    }
                    else if ( hashIntegrityCheck == IC_DUMP_HASH_TABLE_AFTER_REDUCE){
                        UniqueTable->DumpHashTable(fileNo);
                    }

                    opSchd->RegulatePointers();		// 	regulate temporary pointers to ordinary pointers
    /*		if (GCEnabled == H_GC_RF_ENABLED){	// if GC (RF COUNTING) is enabled then; allow GC threads to do its duty
                    //	cout << " Manager : GC (RC) is running now" << endl;
                            GCBarrier -> wait();	// GC barrier (now GC threads can run)
                            GCBarrier -> wait();	// GC barrier (now referenced pointers are finished; you can Dereference the 'dereference pointers')
                            GCBarrier -> wait();	// GC barrier (now GC threads finish)
                    }
                    else if (GCEnabled == H_GC_MARK_ENABLED) {
                    //	cout << " Manager : GC (mark) is running now" << endl;
                            GCBarrier -> wait();	// GC barrier (now GC threads can run)

                            // if GC (MARK) is enabled then; then pass 'operation schedular' to every GC thread, so that GC threads can have an access to 'gate list' and 'operation' structures (that will be used to mark all used nodes)
                            if(fileNo == 0)	{	// for the first time only, reload the 'operation schedular'
                                    int j;
                                    for (j = 0; j < GCThreadsNumber ; j++) {
                                            gcThreads[j]->addSchedular(opSchd);
                                    }
                            }
                            UniqueTable->NextMark();	// set the next mark for GC
                            queue->ReLoad ();	// reload queue as 'GCThread' fetches all elements in the queue (to mark it)
                            GCBarrier -> wait();	// GC barrier (operation schedular is now loaded)
                            GCBarrier -> wait();	// GC barrier (now GC threads finish)
                    }

                    // as this check the integrity of the hash table after GC; so it have to be dumped after GC
                    if ((hashIntegrityCheck == IC_DUMP_HASH_AFTER_GC))
                            DumpGCedHashTable(fileNo);*/

                    if(!getHybridFlag()){
                       break;
                    }
                    cout << "No. of nodes in Hash Table for level " << i << " is :" << UniqueTable->getNodeCount() << endl;
                    cout << "No. of nodes in queue for level " << i << " is :" << queue->getNodeCount() << endl;
                    cout << "No. of freed nodes in queue for level " << i << " is :" << queue->getFreedNodeCount() << endl;
                }
                while(true );	// as long as there is request are not processed yet, keep executing Apply and reduce methods

                // incremental MARK GC will be exploited after the hybrid approach is done. As during hybrid approach all node created will be neccassary. So, garbage collector will be an overhead if it is exists in the 'hybrid loop' (this is true for ordinary mark and incremental mark gc only).
        /*	if (GCEnabled == H_GC_INCREMENTAL_MARK_ENABLED) {
                        cout << " Manager : GC (Incremental mark) is running now" << endl;
                        GCBarrier -> wait();	// GC barrier (now GC threads can run)

                        // if GC (MARK) is enabled then; then pass 'operation schedular' to every GC thread, so that GC threads can have an access to 'gate list' and 'operation' structures (that will be used to mark all used nodes)
                        opSchd->InitiateMarkedGate();
                        if(i == 0)	{	// for the first time only, reload the 'operation schedular'
                                int j;
                                for (j = 0; j < GCThreadsNumber ; j++) {
                                        gcThreads[j]->addSchedular(opSchd);
                                }
                        }
                        UniqueTable->NextMark();	// set the next mark for GC
//			queue->ReLoad ();	// reload queue as 'GCThread' fetches all elements in the queue (to mark it)
                        GCBarrier -> wait();	// GC barrier (operation schedular is now loaded)
                        GCBarrier -> wait();	// GC barrier (now GC threads finish)
                }*/

                fileNo++;
                fileNo1++;
          //      queue->validate();
                queue->Reset ();
            }
            while(!opSchd->isFinished() );	// as long as the gates are finished , keep enque request for this level

    /*	if ((hashIntegrityCheck == IC_DUMP_HASH_AFTER_GC))	// dump gate list up to level i
                    opSchd->DumpGateList(i);	// dump gate list
            else if( hashIntegrityCheck == IC_VALIDATE_HASH_DURING_GC){	// validate gate list up to level i
            //	cout << "relaod hash no. " << fileNo1 << endl;
                    ValidateGateListInGCedHashTableUpToLevel(i);
            }*/
            // THIS SECTION WILL BE MOVED (ISA) AFTER THE END OF FOR LOOP (AFTER DOING DEBUGGING)
            cout << "No. of nodes in Hash Table for level " << i << " is :" << UniqueTable->getNodeCount() << endl;
      //      cout << "No. of nodes in queue for level " << i << " is :" << queue->getNodeCount() << endl;

                   // if "dump hash" or "dump queue" option is enabled; then, dump Hash Table
    //        opSchd->DumpTheLevelToDot();
        }
        if ( (hashIntegrityCheck == IC_DUMP_HASH_TABLE) ||
                (hashIntegrityCheck == IC_DUMP_QUEUE) ||
                (hashIntegrityCheck == IC_DUMP_QUEUE_AFTER_APPLY) ||
                (hashIntegrityCheck == IC_DUMP_QUEUE_AFTER_REDUCE))
            DumpHashTable();
        else if ( hashIntegrityCheck == IC_DUMP_HASH_TABLE_AFTER_REDUCE){
            UniqueTable->DumpTotalCapacity();
        }
        else if ( hashIntegrityCheck == IC_VALIDATE_HASH_TABLE_AFTER_REDUCE){
            integrityChecker -> finalizeIntegrityChecker();
        }
        else if( hashIntegrityCheck == IC_VALIDATE_CIRCUIT_OUTPUTS_WITH_SIMULATION){
        //	SimulateCircuit();
        }
        else if( hashIntegrityCheck == IC_VALIDATE_CIRCUIT_WITH_CUDD){
        //	validatCircuitWithCUDD();
        }
 //	else if ((hashIntegrityCheck == IC_DUMP_HASH_AFTER_GC))
//		opSchd->DumpGateList(i);	// dump gate list
   }

    // return zero and one nodes
    BDDPointer <T,V> getZero(){
        return Zero;
    }
    BDDPointer <T,V> getOne(){
        return One;
    }
    // get i-th variable, WHEN WE DO VARIABLE REORDER, THIS FUNCTION WILL BE MODIFIED
    BDDPointer <T,V> getVariable (int i) {
        // return Variable[i];
        return vc->getVariable(i);
    }
    // get number of output
    int getNumberOfOutput(){
        return ckt-> getNumoutput();
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    // dump data to different formats
    ///////////////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////
    // dump internal data of the package
    ///////////////////////////////////////////////

    // dump all information for data in manager
    void Dump (){
        dumper->Dump ();
    }

    ///////////////////////////////////////////////
    // dump to a DOT file
    ///////////////////////////////////////////////

    // output the graph to a dot file, it accept a pointer to the graph, and the file name, return true if the file is written, false otherwise
    void OutputDot (BDDPointer <T,V> bp,char *fName ){
        dumper->OutputDot (bp ,fName );
    }
    void OutputAllOutputToDot (char *fName ) {
        dumper->OutputAllOutputToDot (fName, ckt );
    }
    // dump hash table (unique table) to a file, for integrity purposes ...
    void DumpHashTable(){
        dumper->DumpHashTable();
    }
    void DumpGCedHashTable(int fileNo){
        UniqueTable->DumpGCedHashTable(fileNo);
    }
    // get the appropriete variable order (according to heuristic weight)
    L* heuristicWeightVariableReordering(){
        L *variableLookUpTable = new L[VariableCount];
        int i;
        ckt->InitiateWeightedHeuristic();
        for(i = 0 ; i < VariableCount; i++){
            variableLookUpTable[VariableCount -1 - i] = ckt->getNextVariable();
        }
        return variableLookUpTable;
    }
    // dump variable order to a file (in order that, CUDD can read it and build its BDD according to it- verification purpose)
    void DumpVariableOrder(){
        char fName[40];
        char *cktName = ckt->getCircuitName();	// get the name of the circuit (c432, c880 ... etc)
        strcpy(fName, cktName);
        strcat(fName, ".varOrder");
        ofstream outClientFile( fName, ios::out );
        int i;
        for(i = 0 ; i < VariableCount; i++){
            outClientFile << vc->mapVar(i) << endl;
        }
    }

/*	// simulate the output of the BDD
    void SimulateCircuit(){
            char *cktName;
            cktName = ckt->getCircuitName();	// get the name of the circuit (c432, c880 ... etc)
            int i;
            InputVectors *vct;		// pointer to object that deals with input vectors

            printf("---------------------------------------------------------------------------\n");
            printf("Simulation of Circuit %s\n", cktName);
            printf("---------------------------------------------------------------------------\n");

            // read in the input vectors and build data structure
            vct = new InputVectors(cktName);

            int vecnum = vct->getNumberOfVectors();	// get number of input vector
            vector <char*> inputVec = vct->getInputVectors();	// get  input vectors
            char *outputVec;
            for(i = 0; i < vecnum; i++){	// for each input vector
                    printf("vec #%d\n",i);
                    printf("INPUT: %s\n",inputVec[i]);
                    outputVec = SimulateCircuitForInput(inputVec[i]);	// simulate circuit with the current input vector
                    printf("OUTPUT: %s\n",outputVec);
            }
    }
    // Validates circuit with CUDD package (CUDD data are dumped in .blif files names 'gate[#].blif)
    void validatCircuitWithCUDD(){
            int gateNum = opSchd->gateCount();	// get number of gates
            int i;
            for (i =1; i <= gateNum; i++){	// for every gate
            //	if(i ==199)
            //		cout << "manager : validatCircuitWithCUDD : " << endl;
                    validateGate(i);
            }
            cout << "manager : validatCircuitWithCUDD : ALL gates are isomorphic with CUDD result" << endl;
    }
    void ValidateGateListInGCedHashTableUpToLevel(int fileNo){
            // open file
            char fName[40];
            sprintf(fName, "GateList %d .txt",fileNo);	// set file name
            ifstream outClientFile(fName);
            char temp[32];
            int i;
            BDDPointer <T,V> bpRaw;
            BDDPointer <T,V> bpNew;
            outClientFile.getline(temp, 32);	// get pointer for gate no "i+1"
            int gateCount = strtoul(temp,0,10);
    //	int * RawGateList = new [gateCount];
            // reload gate list
            for(i =0; i< gateCount; i++){
                    outClientFile.getline(temp, 32);	// get pointer for gate no "i+1"
                    bpRaw.setPointer(strtoul(temp,0,10));	// raw pointer
//			RawGateList[i] = t;
                    bpNew = opSchd->getGateNum(i+1);		// new pointer
                    if(!integrityChecker->map(bpNew).isEqual(bpRaw)){
                            cout << "Hash : ValidateGateListInGCedHashTableUpToLevel : gate no. " << i << " not mateched" << endl;
                            exit(-1);
                    }
                    cout << "Hash : ValidateGateListInGCedHashTableUpToLevel : gate no. " << i << " mateched" << endl;
            }
    }*/
};

#endif
