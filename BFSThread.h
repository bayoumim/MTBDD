#ifndef _BFS_THREAD_CLASS
#define _BFS_THREAD_CLASS

#include "Queue.h"
#include "Hash.h"
#include "mutex.h"

#include "thread.h"
#include "Barrier.h"
#include "TripleChecker.h"

template<typename T,typename V,typename L,typename K,typename FLG>
class BFSThread : public Thread {
protected:
    Hash <T,V,L,K,FLG>* HashTable;	// pointer to hash table
    unsigned short id;
    unsigned short ThreadsNo;		// total number of threads
    Queue <T,V,L,K,FLG> * queue;		// pointer to all queues
    L VariableCount;	// total number of variables
    BDDPointer <T,V> *Zero;
    BDDPointer <T,V> *One;
    Barrier *barrier;
    Barrier *GlobalBarrier;	// barrier used to synchronize all threads + manaager
    bool enableComputedTable; // flag for enabling or disabling the coomputed table
    TripleChecker <T,V,L,K,FLG> *tc;
    bool *TerminateThreads;			// this flag is used to terminate threads, this is done at finilize methods
    Mutex *mutex;
    volatile L *startingLevel;
    volatile bool *hybridFlag;
    unsigned char hashIntegrityCheck;
    unsigned char GCEnabled;
    Barrier *GCBarrier;
/////////////////////////////////////////////////////////////////////////////////////////////////////
//				PRIVATE FUNCTIONS
/////////////////////////////////////////////////////////////////////////////////////////////////////
    // it check if the requests is terminal case for the terminal case
    bool isTerminalCaseForInvertedEdge  (BDDPointer <T,V> &F, BDDPointer <T,V> &G, BDDPointer <T,V> &H, BDDPointer <T,V> & R) {
        return tc->isTerminalCaseForInvertedEdge(F, G, H, R);
    }
    // get the x component for node, corresponding to some x variable
    void GetChildComponents(BDDPointer <T,V> &F, L level, BDDPointer <T,V> &Fx, BDDPointer <T,V> &Fn){
        BDDNode  <T,V,L>nodeF;
        if (!F.isTemporary()){
            // get level of the node F
            L lev = this->HashTable->getLevelFromID(F);
            if (level > lev){	// if the input level > node level ; the return pointer to node
                Fx = F;
                Fn = F;
                return;
            }
            else if (F.isInverted()) {	// if the node is inverted
                BDDPointer <T,V> g = F;
                g.invert();	// invert F
                HashTable->getInfo (g, nodeF);		// get inverted node
                Fx = nodeF.getThen();
                Fx.invert();
                Fn = nodeF.getElse();
                Fn.invert();
                return  ;	// return then pointer of the node F
            }
            else {		// if it is regular node
                HashTable->getInfo (F, nodeF);
                Fx = nodeF.getThen();
                Fn = nodeF.getElse();
                return  ;	// return then pointer of the node F
            }
        }
        cout << "BFSThread : GetChildComponents : something wrong" << endl;
        exit(-1);
    }
    // put the triple in the standard form, and return true if the ouptput is inverted
    bool putRequestInStandardTriple(BDDPointer <T,V> &  F, BDDPointer <T,V> & G, BDDPointer <T,V> & H) {
        return tc->putRequestInStandardTriple(F,G,H);
    }
    // get hybrid flag
    bool getHybridFlag(){
        bool temp;
        mutex->lock();
        temp =	*hybridFlag;
        mutex->unlock();
        return temp;
    }
    void setHybridFlag(bool temp){
        mutex->lock();
        *hybridFlag = temp;
        mutex->unlock();
    }
    L getStartingLevel(){
        L temp;
        mutex->lock();
        temp =	*startingLevel;
        cout << "Apply: BFSThread. starting from level : " << temp << endl;
        mutex->unlock();
        return temp;
    }
    void setStartingLevel(int temp){
        mutex->lock();
        if (*startingLevel >= temp)
            *startingLevel = temp;
        mutex->unlock();
    }

public:
    BFSThread () : Thread(id){

    }
    BFSThread(unsigned short id,
                Hash <T,V,L,K,FLG> * &HashTable,
                Queue <T,V,L,K,FLG>* &queue,
                L VariableCount,
                BDDPointer <T,V> & Zero,
                BDDPointer <T,V> &One,
                Barrier &barrier,
                bool enableComputedTable,
                TripleChecker <T,V,L,K,FLG> *tc,
                bool TerminateThreads,
                Barrier *GlobalBarrier,
                Mutex &mutex,
                volatile L &startingLevel,
                volatile bool &hybridFlag,
                unsigned char hashIntegrityCheck,
                unsigned char GCEnabled,
                Barrier *GCBarrier) : Thread(id) {
        
        this->GCBarrier = GCBarrier;
        this->GCEnabled = GCEnabled;
        this->HashTable = HashTable;
        this->id = id;
        this->queue = queue;
        this->VariableCount = VariableCount;
        this->Zero = &Zero;
        this->One = &One;
        this->barrier = &barrier;
        this->enableComputedTable = enableComputedTable;
        this->tc = tc;
        this->TerminateThreads = &TerminateThreads;
        this->GlobalBarrier = GlobalBarrier;
        this->mutex = &mutex;
        this->startingLevel = &startingLevel;
        this->hybridFlag = &hybridFlag;
        this->hashIntegrityCheck = hashIntegrityCheck;
        start();	// run threads
    }

    void run(){
        // wait for all threads to be created
        do {
            do{
                GlobalBarrier-> wait();	// wait until the request is enqueued
                L temp = VariableCount;
                if (HashTable->getNodeCount() == 31724){
                    cout << "manager : run : no of nodes " << HashTable->getNodeCount() << endl;
                }
                BfApply (temp);
                GlobalBarrier -> wait();	// barrier (wait until the queue is reloaded by manager)
                GlobalBarrier -> wait();	// barrier (now the queue is reloaded, reduce can proceed)
                temp = VariableCount;
                BfReduce (temp);	// apply reduce
                // reset the queue again, our policy is to keep all temporary nodes from all previous ITE OPERATIONS,
                // NOTE: ALTHOUGH THIS IS THE EASIEST SOLUTION, IT IS NOT EFFICIENT WHEN THE NUMBER OF NODES BECAME VERY LARGE
                // reset queues for next ite operations
                // wait until all threads are finished (barrier)
                GlobalBarrier -> wait();	// barrier ( now the manager can regulate pointers, check for the next starting level in apply method, and check if there is another round in the hybrid approach)
        /*	if (GCEnabled == H_GC_RF_ENABLED){	// if GC is enabled then; allow GC threads to do its duty
                        GCBarrier -> wait();	// GC barrier (now GC threads can run)
                        GCBarrier -> wait();	// GC barrier (now referenced pointers are finished; you can Dereference the 'dereference pointers')
                        GCBarrier -> wait();	// GC barrier (now GC threads finish)
                }
                else if ( (GCEnabled == H_GC_MARK_ENABLED) ) {	// if we have ordinary  marking
                        GCBarrier -> wait();	// GC barrier (now GC threads can run)
                        GCBarrier -> wait();	// GC barrier (operation schedular is now loaded)
                        GCBarrier -> wait();	// GC barrier (now GC threads finish)
                }*/
                if(!getHybridFlag()){
                    break;
                }
            }
            while(true);	// as long as there is request are not processed yet, keep executing Apply and reduce methods (hybrid approach loop)
            // incremental MARK GC will be exploited after the hybrid approach is done. As during hybrid approach all node created will be neccassary. So, garbage collector will be an overhead if it is exists in the 'hybrid loop' (this is true for ordinary mark and incremental mark gc only).
            if ( (GCEnabled == H_GC_INCREMENTAL_MARK_ENABLED) ) {	// if we have ordinary or incremental marking
                GCBarrier -> wait();	// GC barrier (now GC threads can run)
                GCBarrier -> wait();	// GC barrier (operation schedular is now loaded)
                GCBarrier -> wait();	// GC barrier (now GC threads finish)
            }
        }
//	while(!(*TerminateThreads));
        while(true);
    }

    // perform apply
    void BfApply (L startLevel) {
        L MaxIndex = startLevel;	// get the top variable
        BDDNode <T,V,L> R,R1;
        Request <T,V,L> req ;	// create a request
        BDDPointer <T,V> Fx,Gx,Hx,Fn,Gn,Hn;
        L i;
        BDDPointer <T,V> result,F, G, H;	// pointer to the result
        bool flag = false; // this flag is rised when all threads finished one level
        bool invertOutput;
        unsigned char thenFlag, elseFlag;
        bool scheduleThisLoopFlag = false;
        // allow all queues to overwrite forwarded requests
        for (i = MaxIndex - 1; (T)i >= TERMINAL_LEVEL; i--){
            scheduleThisLoopFlag = false;
            bool ThenProcessedFlag, ElseProcessedFlag;
            do {
                if (queue->dequeRequest(req,i,ThenProcessedFlag,ElseProcessedFlag)){ // there is items in the queue
                    if( (req.getF().getHashString() == 211836844) && (req.getG().getHashString() == 0) && (req.getH().getHashString() == 1681356105) ){
                        cout << "bfs threads : apply : " << endl;
                        req.DumpToScreen();
                    }
                    // get pointer from the request
                    R = req. getR();
                    if (ThenProcessedFlag && ElseProcessedFlag){	// if this request is processed in a previous round
                        continue;
                    }
                    F = req. getF();
                    G = req. getG();
                    H = req. getH();
                    // get nodes corresponding to pointers f, g,h (from hash table)
                    thenFlag = HQRZ_NODE_ADDED_CORRECTLY;
                    GetChildComponents(F, i+1,Fx,Fn);
                    GetChildComponents(G, i+1,Gx,Gn);
                    GetChildComponents(H, i+1,Hx,Hn);
                    if (!ThenProcessedFlag){	// if else pointer is not processed; then, process it.
                        if(Gx.getHashString() == 0 && Gx.getLocalId() == 1){
                            cout << "bfs threads: ya lahwy : then" << endl;
                        }
                        if(Gx.getHashString() == 0 && Gx.getLocalId() == 1){
                            cout << "bfs threads: ya lahwy : then" << endl;
                        }
                        // generate request for the then pointer
                        if (isTerminalCaseForInvertedEdge ( Fx, Gx, Hx, result)){// if it is a terminal case
                            R.setThen(result);		// update node
                        }
                        else {
                            invertOutput = putRequestInStandardTriple(Fx, Gx, Hx);	// put pointer in the standard form
                            req.setF(Fx);
                            req.setG(Gx);
                            req.setH(Hx);
                            queue->findOrEnqRequest(req,result,thenFlag,i);	// enque the request
                            if (invertOutput)
                                result.invert();    // invert the results
                            R.setThen(result);	// update node
                        }
                    }

                    elseFlag = HQRZ_NODE_ADDED_CORRECTLY;
                    bool elseFlagReduceFinished = false;		// the flag indicat that the pointer is finished.
                    if (!ElseProcessedFlag){	// if then pointer is not processed, then process it
                        if(Gn.getHashString() == 0 && Gn.getLocalId() == 1){
                            cout << "bfs threads: ya lahwy : else" << endl;
                        }
                        // generate requests for the else pointer
                        if (isTerminalCaseForInvertedEdge ( Fn, Gn, Hn, result)){// if it is a terminal case
                            R.setElse(result);		// update node
                            elseFlagReduceFinished = true;
                        }
                        else {
                            invertOutput = putRequestInStandardTriple(Fn, Gn, Hn);	// put pointer in the standard form
                            req.setF(Fn);
                            req.setG(Gn);
                            req.setH(Hn);

                            queue->findOrEnqRequest(req,result,elseFlag,i);	// store it in queue again
                            if (invertOutput)
                                result.invert();
                            R.setElse(result);	// update node
                        }
                    }

                    queue->putRequest( F, G, H, R, !(thenFlag == HQRZ_NODE_NOT_ADDED), !(elseFlag == HQRZ_NODE_NOT_ADDED));	// store it in queue again

                    if ( (thenFlag == HQRZ_NODE_ADDED_LIMIT_REACHED) || (elseFlag == HQRZ_NODE_ADDED_LIMIT_REACHED) || (thenFlag == HQRZ_NODE_NOT_ADDED) || (elseFlag == HQRZ_NODE_NOT_ADDED) ) {	// if the queue reached the stake of this level after inserting "then" or "else", then quit this level to the next one, (ofcourse, wait for other threads).
                        if (!getHybridFlag()){
                            setHybridFlag(true);	// rise hybrid flag
                            mutex->lock();
                            cout << "hybrid approach will be exploited at level : " << i << endl;
                            mutex->unlock();
                        }
                    }
                }
                else{ // if there is no other request in the queue
                    barrier->wait();
                    flag = true;
                }
            }
            while (!flag); // this flag is rised only when all thread completes one level
            flag = false; // reset flag agian
            if (i == TERMINAL_LEVEL)    // as the for loop counter is unsigned; so, it can't be goes below zero. and hence, in order to end it we have to break.
                break;
        }
    }
    // perform reduce
    void BfReduce (L startLevel) {
        int i;
        L MaxIndex = startLevel;	// get the top variable
        BDDNode <T,V,L> tempnode,TT,E;	// used to store a temp node from queue
        BDDNode <T,V,L> finalNode;	// store a final node to be added to hash table
        BDDPointer <T,V> tempT, tempE;
        BDDPointer <T,V> nodePointer,T1,E1;
        // resetting all queues
        Request <T,V,L> req;
        bool flag = false;
        bool invertedOutput,addAsDatedNode;
        // starting the main loop
        for (i = 0; i <= (MaxIndex - 1); i++) {
            bool ThenProcessedFlag, ElseProcessedFlag;
            do {
                if (queue->dequeRequestWithoutLocking(req, i,ThenProcessedFlag,ElseProcessedFlag)){ // if there is a node in queue
                    if ( ((!ThenProcessedFlag) && (!ElseProcessedFlag)) || req.getR().isForwarded(tempT) ){	// if this request is NOT processed in Apply (both then and else pointer) OR it is forwarded, in skip This Loop
                        continue;
                    }

             /*       if ( (req.getF().getHashString() == 932596056) &&  (req.getG().getHashString() == 4967765 ) && (req.getH().getHashString() == 0 ) )
                        cout << "BFSThread : Reduce : " << endl;*/

                    tempnode = req.getR();

                    T1 = tempnode.getThen();	// get pointer for t
                    bool thenPointerFinishedFlag = false;	// flag indicate that then pointer is finished
                    if (ThenProcessedFlag) { // if Then pointer is processed; then, reduce it...
                        // perform forwarding if there is any for then node
                        if (T1.isEqual(*One) || T1.isEqual(*Zero)) {	// if then point to zero or one nodes (so there will be no need to reduce them), also there is no level zero in in any queue
                            tempT = T1;	// just forward node to itself
                            thenPointerFinishedFlag = true;	// then pointer is finished
                        }
                        else {
                            if (T1.isTemporary()){	// if then is a temporary node
                                invertedOutput = T1.isInverted();
                                if (invertedOutput)	// if the edge is inverted
                                    T1.invert();

                                queue->fetchNode(T1, TT);	// get T node
                                if (TT.isForwarded(tempT)) {
                                    if (TT.isInverted()) { // if the forward node is inverted, then invert the edge pointing to it.
                                        tempT.invert();
                                    }
                                    thenPointerFinishedFlag = true;	// then pointer is finished
                                }
                                else {
                                    tempT = T1;	// if the node is not forwarded, save the pointer
                                }
                                if (invertedOutput)	// if the edge is inverted
                                    tempT.invert();
                            }
                            else{
                                tempT = T1;
                                thenPointerFinishedFlag = true;	// then pointer is finished
                            }
                        }
                    }
                    else	// if the pointer is processed in a previous round, then get it
                        tempT = T1;

                    E1 = tempnode.getElse();	// get pointer for e
                    bool elsePointerFinishedFlag = false;	// flag indicate that then pointer is finished
                    if (ElseProcessedFlag) { // if Else pointer is processed; then, reduce it...
                        // perform forwarding if there is any for else node
                        if (E1.isEqual(*Zero) || E1.isEqual(*One)) {	// if else point to zero or one nodes (so there will be no need to reduce them)
                            tempE = E1;	// just forward node to itself
                            elsePointerFinishedFlag = true;	// then pointer is finished
                        }
                        else {
                            if (E1.isTemporary()){	// if then is a temporary node
                                invertedOutput = E1.isInverted();
                                if (invertedOutput)	// if the edge is inverted
                                    E1.invert();

                                queue->fetchNode(E1, E);	// get E node
                                if (E.isForwarded(tempE)) {
                                    if (E.isInverted()) { // if the forward node is inverted, then invert the edge pointing to it.
                                        tempE.invert();
                                    }
                                    elsePointerFinishedFlag = true;	// then pointer is finished
                                }
                                else{
                                    tempE = E1;	// if the node is not forwarded, save the pointer
                                }
                                if (invertedOutput)	// if the edge is inverted
                                    tempE.invert();
                            }
                            else{
                                tempE = E1;
                                elsePointerFinishedFlag = true;	// then pointer is finished
                            }
                        }
                    }
                    else	// if the pointer is processed in a previous round, then get it
                        tempE = E1;

                    addAsDatedNode = true;
                    bool ExistNodeFlag = true;	// a flag used to indicate wether the created node is new or not, it is set to 'true' by default as we assume that the node is already exists (this may happens in forwardering etc...)
                    if ( tempT.isTemporary() || tempE.isTemporary()|| !ThenProcessedFlag || !ElseProcessedFlag) { // if any edges are temporary or any of them are not processed yet, add it to queue for further processing
                        tempnode.setThen(tempT); // store the new value
                        tempnode.setElse(tempE); // store the new value
                        addAsDatedNode = false;
                    }
                    else if (tempT.isEqual (tempE) ) {	// if it is  a redundant node
                        tempnode.forwardTo (tempT);	// forward the node
                    }
                    else if ( tempT.isInverted() ) {	// if the edges need to be restricted. If the then edge is inverted
                        tempT.invert();	// invert then edge
                        tempE.invert();	// invert else edge
                        // create node and find or add it to the hash table
                        finalNode.setThen(tempT);
                        finalNode.setElse(tempE);
                        finalNode.setLevel(i + 1);

                        ExistNodeFlag = HashTable->findOrAdd (finalNode, nodePointer);

                        nodePointer.invert();	// invert the pointer to this node
                        tempnode.forwardTo (nodePointer);
                    }
                    else {	// if this node is an acceptable node ("then" edge is regular)
                        finalNode.setThen(tempT);
                        finalNode.setElse(tempE);
                        finalNode.setLevel(i + 1);
                        finalNode.clearForwarded();
                        finalNode.clearInverted();

                        ExistNodeFlag = HashTable->findOrAdd (finalNode, nodePointer);
                        tempnode.forwardTo (nodePointer);
                    }

                    req.setR(tempnode);


                    if (addAsDatedNode){	// if the 'then' and 'else pointer is totally processed, then finish this request
                        queue->putOldRequest (req,ExistNodeFlag, tempT);
                        queue->freeOneNode(tempT, i);
                    }
                    else {
                        queue->putRequestWithoutProcessing (req, thenPointerFinishedFlag, elsePointerFinishedFlag);
                    }
                }
                else {//IF THE queue EMPTY; ADD SYNCHRONIZATION PART HERE
                    barrier->wait();
                    flag = true;
                }
            }
            while (!flag);
            flag = false;
        }
    }
};

#endif
