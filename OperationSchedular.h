// this class schedule the operation of a gate
#ifndef _OPERATION_SCHEDULAR_CLASS
#define _OPERATION_SCHEDULAR_CLASS
#include "BDDPointer.h"
#include "circuit.hh"
#include "Queue.h"
#include "VariableCreator.h"
#include "TripleChecker.h"
#include "Dumper.h"
//#include "GCQueue.h"

enum
{
   OP_XOR,          /* 0 */
   OP_AND,          /* 1 */
   OP_OR            /* 2 */
};

template<typename T,typename V,typename L,typename K,typename FLG>
class OperationSchedular {
private:
    int *GateFlag;	// flags 	(operation count | finished | invert | operation (xor, and, or) )
    int *GateCount;	// pointer to gate count
    BDDPointer <T,V> ** Operations;	// 2D array for operations
    int level;
    bool ReadCircuitFlag;	// indicate whether the level is finished or not (used in NextRound)
    circuit *ckt;

    Queue <T,V,L,K,FLG> * queue;
    int numOfGatesInLevel;		// store the number of gates in a level
    int finishedGates;		// store the number of gates in a level
    int MaxNumOfGates;		// store the maximum no. of gates in any level
    int MaxNumOfInputs;		// store the maximum no. of inputs in any gates
    int numOut;	// get number of outputs
    int TotalNoOfGate;	// get total number of gates
    int InputCounter, outputCounter;
    BDDPointer <T,V>*Outputs;	// reserve a space for outputs	(index start from 0)
    BDDPointer < T,V>*GateList;	// reserve a space for all gates (index start from 0)
    int GateCounter;
    VariableCreator <T,V,L,K,FLG> *vc;
    BDDPointer <T,V> One;
    BDDPointer <T,V> Zero;
    TripleChecker <T,V,L,K,FLG> * tc;
    int StartCounter;	// this pointer is used when we regulate the pointers of the operations, it store the starting gate to be processed
    Dumper <T,V,L,K,FLG> * dumper;
    int * NumOfRemainingPointers;	// number of remaining pointer (used in regulating function)
    Hash <T,V,L,K,FLG>* UniqueTable;
    volatile L *startingLevel; 	// store the level that apply and reduce methods should begin with
    int GCEnabled;
//	GCQueue * gcQueue;
    volatile int markedGates;
    volatile int markedOperations;
    Mutex mutex;

    // perform AND operation
    BDDPointer <T,V> And(BDDPointer <T,V> &f, BDDPointer <T,V>  &g){
        BDDPointer <T,V> result = insertIteRequest (f,g,Zero);	// ITE(F,G,0) = F AND G
        return result;
    }
    // perform OR operation
    BDDPointer <T,V> Or(BDDPointer <T,V> &f, BDDPointer <T,V>  &g){
        BDDPointer <T,V> result = insertIteRequest (f,One,g);	// ITE(F,1,G) = F OR G
        return result;
    }
    BDDPointer <T,V> Xor(BDDPointer <T,V> &f, BDDPointer <T,V>  &g){
        BDDPointer <T,V> result = Not (g);	// NOT(G)
        result = insertIteRequest (f,result,g);	// ITE(F,NOT(G),G) = F XOR G
        return result;
    }
    // perform XNOR operation
    BDDPointer <T,V> Xnor(BDDPointer <T,V> &f, BDDPointer <T,V>  &g){
        BDDPointer <T,V> result = Not (g);	// NOT(G)
        result = insertIteRequest (f,g,result);	// ITE(F,NOT(G),G) = F XOR G
        return result;
    }
    // perform NOT operation
    BDDPointer <T,V> Not(BDDPointer <T,V> &f){
        BDDPointer <T,V> result =f;
        result.invert();	// ITE(F,0,1) = NOT(F)
        return result;
    }
    // perform NOR operation
    BDDPointer <T,V> Nor(BDDPointer <T,V> &f, BDDPointer <T,V>  &g){
        BDDPointer <T,V> result = Not (g);	// NOT(G)
        result = insertIteRequest (f,Zero,result);	// ITE(F,0,NOT(G)) = F NOR G
        return result;
    }
    // perform NAND operation
    BDDPointer <T,V> Nand(BDDPointer <T,V> &f, BDDPointer <T,V>  &g){
        BDDPointer <T,V> result = Not (g);	// NOT(G)
        result = insertIteRequest (f,result,One);	// ITE(F,NOT(G),1) = F NAND G
        return result;
    }
    // it check if the requests is terminal case for the terminal case
    bool isTerminalCaseForInvertedEdge  (BDDPointer <T,V> &F, BDDPointer <T,V> &G, BDDPointer <T,V> &H, BDDPointer <T,V> & R) {
        return tc->isTerminalCaseForInvertedEdge(F, G, H, R);
    }
    // put the triple in the standard form, and return true if the ouptput is inverted
    bool putRequestInStandardTriple(BDDPointer <T,V> &  F, BDDPointer <T,V> & G, BDDPointer <T,V> & H) {
        return tc->putRequestInStandardTriple(F,G,H);
    }

    // insert a request in queue, and return a pointer to the operation (in the queue). So, the returned pointer is temporary (not permenant), unless it is a terminal case
    BDDPointer <T,V> insertIteRequest(BDDPointer <T,V>  F, BDDPointer <T,V>  G, BDDPointer <T,V>  H){
        BDDPointer <T,V> bp;	// POINTER TO THE RESULT OF THE OPERATION
        bool inv = false;
        if (!isTerminalCaseForInvertedEdge ( F, G, H, bp) ) {	// if not the terminal case
            // THE MANAGEER CREATE A REUEST, THEN ENQUE IT TO THE QUEUE, PRESERVE A POINTER FOR EACH REQUEST
            // create request
            L MaxIndex = this->queue->getTopVarIndex(F,G,H);	// get the top variable
            if (MaxIndex > (*startingLevel) )
                *startingLevel = MaxIndex;
            BDDNode <T,V,L> R;
            R.setLevel (MaxIndex);
            R.clearForwarded();
            R.clearInverted();
            inv = putRequestInStandardTriple(F,G,H);	// put the triple in the standard form, and store whether the pointer to operation is inverted or not
            unsigned char temp = 0;
            L temp1 = MaxIndex;
            Request <T,V,L> req ( F, G, H, R);	// create a request
            queue->ForcedFindOrEnqRequest(req,bp,temp, temp1);		// enque it
        }
        if (inv)
            bp.invert();
        return bp;
    }
    // set the finish flag for certain gate
    void SetFinishFlag(int Gate) {
        GateFlag[Gate] = GateFlag[Gate] | 0x8; // set inversion bit
    }
    // clear the finish flag for certain gate
    void ClearFinishFlag(int Gate) {
        GateFlag[Gate] = GateFlag[Gate] & 0xFFFFFFF7; // clear inversion bit
    }
    // check if the gate is finished
    bool isFinished(int Gate) {
        if ((GateFlag[Gate] & 0x8) == 0) {	// if it is finished
            return false;
        }
        else {				// if the gate isn't finished
            return true;
        }
    }
    // check if the gate is inverted
    bool isInverted(int Gate) {
        if ((GateFlag[Gate] & 0x4) ==0) {	// if it is inverted
            return false;
        }
        else {				// if the gate isn't inverted
            return true;
        }
    }
    // set operation for certain gate
    void SetOperation (int Gate, int OperationType) {
        GateFlag[Gate] = GateFlag[Gate] & 0xFFFFFFFC; // clear old operation
        GateFlag[Gate] = GateFlag[Gate] | OperationType; // set new operation

    }
    // set operation for certain gate
    int GetOperation (int Gate) {
        return GateFlag[Gate] & 0x3; // get  operation
    }
    // set invertion flag for certain gate
    void setInvert (int Gate) {
        GateFlag[Gate] = GateFlag[Gate] | 0x4; // set inversion bit
    }
    // clear invertion flag for certain gate
    void ClearInvert (int Gate) {
        GateFlag[Gate] = GateFlag[Gate] & 0xFFFFFFFB; // clear inversion bit
    }
    // set number of operation for certain gate
    void SetOperationCount (int Gate, int OperationCount) {
        int temp =  GateFlag[Gate] & 0xF; // clear old operation
        OperationCount = OperationCount << 4;
        temp = temp | (OperationCount); // set new operation
        GateFlag[Gate] = temp ;

    }
    // get number of operation for certain gate
    int GetOperationCount (int Gate) {
        return GateFlag[Gate] >> 4;
    }
public:
    // constructors
    // this constructor is called
    OperationSchedular(int NumOfGate,
            int NumOfInputs,
            circuit *ckt,
            Queue <T,V,L,K,FLG> * queue,
            VariableCreator <T,V,L,K,FLG> *vc,
            TripleChecker <T,V,L,K,FLG> * tc,
            Dumper <T,V,L,K,FLG>* dumper,
            Hash <T,V,L,K,FLG> * UniqueTable,
            volatile L &startingLevel,
            int GCEnabled /*,GCQueue * gcQueue*/) {
        
        // initialize the space for pointers
//	this->gcQueue = gcQueue;
        this->GCEnabled = GCEnabled;

        Operations = (BDDPointer <T,V> **) malloc (sizeof(BDDPointer <T,V> *) * NumOfGate) ;
        GateFlag = (int *) malloc (sizeof(int) * NumOfGate) ;
        int i;
        NumOfRemainingPointers = new int[NumOfGate];
        for (i = 0 ; i < NumOfGate; i ++) {
            Operations [i] =  new  BDDPointer <T,V>[( (NumOfInputs>>1) + (NumOfInputs & 1) )];
            GateFlag[i] = 0;
        }
        InputCounter = 0;
        outputCounter = 0;
        GateCounter = 0;
        ReadCircuitFlag = true;
        this->ckt = ckt;
        this-> queue = queue;
        this->vc = vc;
        this->tc = tc;
        Zero = vc->getZero ();
        One = vc->getOne ();
        numOut = ckt-> getNumoutput();
        TotalNoOfGate = ckt->getGateCount();	// get total number of gates
        Outputs = new  BDDPointer <T,V> [numOut];
        GateList = new  BDDPointer <T,V> [TotalNoOfGate];
        this->dumper = dumper;
        this->MaxNumOfGates = NumOfGate;
        this->MaxNumOfInputs = NumOfInputs;
        this->UniqueTable = UniqueTable;
        this->startingLevel = &startingLevel;
        *(this->startingLevel) = 0;
        markedGates = 0;
        markedOperations = 0;
    }
    // this function is called at the begining of processing the level (it is contains initialization only)
    void initiateLevel(int level){
        this->level = level;
        this->ReadCircuitFlag = true;
        this->numOfGatesInLevel = ckt->getNumOfGateInLevel(level);	// reset counter of the gate in the level
        this->finishedGates = 0;
        StartCounter = GateCounter + 1;
    }
    // indicate if the level is finished or not
    bool isFinished () {
        if (numOfGatesInLevel == finishedGates)
            return true;
        else
            return false;
    }
    // this function enque all avialable request for certain level,
    void NextRound() {
        *startingLevel = 0;
        int j,GateType,i;
        BDDPointer <T,V> f,g;
        int LoopLimit;
        int NumOfInputs;

        if (ReadCircuitFlag) {	// this is the first time you enter the next round function
            for (j = 0 ; j <  numOfGatesInLevel; j++) { // LOOP FOR EVERY GATE IN A LEVEL, including the input level
                GateCounter = ckt->getNextGate();
                GateType = ckt->getGateType (GateCounter);	// get gate type

                if ( GateCounter >= (2061 -1 ) ) {
                    int x;
                    x = 50;
                }

                switch (GateType)	// set the initial value for the function depends on the gate
                {
                    case G_INPUT:	// if the gate is input, just ignore it,and consider it is a a finished gate, and store the corresponding BDD POINTER in the gate list
                        finishedGates++;	// this is a finished gate
                        cout << "gate no. : "<< GateCounter << " is finished" << endl;
                        SetFinishFlag(j);
                        GateList[GateCounter-1] = vc->getVariable(InputCounter);	// get the variable
                        InputCounter++;		// increment the input counter
                        NumOfRemainingPointers[j] = 0;
                        break;
                    case G_OUTPUT:	// if the gate is output, store the input of this gate, store BDD pointer in the output list,and consider it is a a finished gate
                        finishedGates++;	// this is a finished gate
                        cout << "gate no. : "<< GateCounter << " is finished" << endl;
                        SetFinishFlag(j);
                        GateList[GateCounter-1] = GateList[ckt->getGateInList(GateCounter)[0]-1];	// get the index of the input, and store the corresponding bdd pointer in the gate list
                        Outputs[outputCounter] = GateList[GateCounter-1];
                        NumOfRemainingPointers[j] = 0;
                        outputCounter++;
                        break;

                    default:
                        NumOfInputs = ckt->getGateInNum(GateCounter);
                        if ( NumOfInputs <= 2 ) {	// if number of inputs = 2 or = 1 (BUFFERS AND INVERTERS)
                            switch (GateType)	// COMPLETE THE REMAINING OF THE LIST, XOR, XNOR, ..., ETC.
                            {
                                case G_XOR:	// if the gate is NOR,
                                    f = GateList[ckt->getGateInList(GateCounter)[0]-1];	// get the first input
                                    g = GateList[ckt->getGateInList(GateCounter)[1]-1];	// get the second input
                                    GateList[GateCounter-1] = Xor(f,g);			// perform operation
                                    finishedGates++;					// this gate is finished
                                    break;
                                case G_XNOR:	// if the gate is XNOR,
                                    f = GateList[ckt->getGateInList(GateCounter)[0]-1];	// get the first input
                                    g = GateList[ckt->getGateInList(GateCounter)[1]-1];	// get the second input
                                    GateList[GateCounter-1] = Xnor(f,g);			// perform operation
                                    finishedGates++;					// this gate is finished
                                    break;
                                case G_AND:	// if the gate is AND,
                                    f = GateList[ckt->getGateInList(GateCounter)[0]-1];	// get the first input
                                    g = GateList[ckt->getGateInList(GateCounter)[1]-1];	// get the second input
                                    GateList[GateCounter-1] = And(f,g);			// perform operation
                                    finishedGates++;					// this gate is finished
                                    break;
                                case G_NAND:	// if the gate is NAND,
                                    f = GateList[ckt->getGateInList(GateCounter)[0]-1];	// get the first input
                                    g = GateList[ckt->getGateInList(GateCounter)[1]-1];	// get the second input
                                    GateList[GateCounter-1] = Nand(f,g);			// perform operation
                                    finishedGates++;					// this gate is finished
                                    break;
                                case G_OR:	// if the gate is OR,
                                    f = GateList[ckt->getGateInList(GateCounter)[0]-1];	// get the first input
                                    g = GateList[ckt->getGateInList(GateCounter)[1]-1];	// get the second input
                                    GateList[GateCounter-1] = Or(f,g);			// perform operation
                                    finishedGates++;					// this gate is finished
                                    break;
                                case G_NOR:	// if the gate is NOR,
                                    f = GateList[ckt->getGateInList(GateCounter)[0]-1];	// get the first input
                                    g = GateList[ckt->getGateInList(GateCounter)[1]-1];	// get the second input
                                    GateList[GateCounter-1] = Nor(f,g);			// perform operation
                                    finishedGates++;					// this gate is finished
                                    break;
                                case G_NOT:	// if the gate is NOT,
                                    f = GateList[ckt->getGateInList(GateCounter)[0]-1];	// get the first input
                                    GateList[GateCounter-1] = Not(f);			// perform operation
                                    finishedGates++;					// this gate is finished
                                    break;
                                case G_BUF:	// if the gate is BUF,
                                    f = GateList[ckt->getGateInList(GateCounter)[0]-1];	// get the first input
                                    GateList[GateCounter-1] = f;			// perform operation
                                    finishedGates++;					// this gate is finished
                                    break;
                                default:
                                    break;
                            }
                            cout << "gate no. : "<< GateCounter << " is finished" << endl;

                            SetFinishFlag(j);
                            NumOfRemainingPointers[j] = 0;
                        }
                        else {	// if the number of inputs are greater than 2 and this is the first time the manager call the function, this happens latter for c432 and others. In the first time this function is called, we will need to read the CIRCUIT first, then we will get the corresponding pointer from hashtable
                            LoopLimit = NumOfInputs >> 1;	// floor of number of gate divide by 2
                            for (i = 0; i < LoopLimit ; i++ ) {
                                f = GateList[ckt->getGateInList(GateCounter)[i << 1]-1];	// get the first input (2 x i)
                                g = GateList[ckt->getGateInList(GateCounter)[(i << 1 ) + 1]-1];	// get the second input (2 x i + 1)

                                switch (GateType)	// COMPLETE THE REMAINING OF THE LIST, XOR, XNOR, ..., ETC.
                                {
                                    case G_XOR:	// if the gate is NOR,
                                        Operations[j][i] = Xor(f,g);			// perform operation
                                        SetOperation(j,OP_XOR);
                                        ClearInvert(j);
                                        ClearFinishFlag(j);
                                        break;
                                    case G_XNOR:	// if the gate is XNOR,
                                        Operations[j][i] = Xor(f,g);			// perform operation
                                        ClearFinishFlag(j);
                                        setInvert (j);
                                        SetOperation(j,OP_XOR);
                                        break;
                                    case G_AND:	// if the gate is AND,
                                        Operations[j][i] = And(f,g);			// perform operation
                                        ClearFinishFlag(j);
                                        SetOperation(j,OP_AND);
                                        ClearInvert(j);
                                        break;
                                    case G_NAND:	// if the gate is NAND,
                                        ClearFinishFlag(j);
                                        Operations[j][i] = And(f,g);			// perform operation
                                        SetOperation(j,OP_AND);
                                        setInvert (j);
                                        break;
                                    case G_OR:	// if the gate is OR,
                                        Operations[j][i] = Or(f,g);			// perform operation
                                        ClearFinishFlag(j);
                                        ClearInvert(j);
                                        SetOperation(j,OP_OR);
                                        break;
                                    case G_NOR:	// if the gate is NOR,
                                        Operations[j][i] = Or(f,g);			// perform operation
                                        ClearFinishFlag(j);
                                        setInvert (j);
                                        SetOperation(j,OP_OR);
                                        break;
                                    default:
                                        break;
                                }
                            }
                            if ( (NumOfInputs & 0x1) == 1) {	//  if number of inputs is odd (add the last element)
                                Operations[j][LoopLimit] = GateList[ckt->getGateInList(GateCounter)[NumOfInputs-1]-1];	// add the last input
                                NumOfRemainingPointers[j] = LoopLimit+1 ;
                                SetOperationCount(j, LoopLimit+1);	// set number of operations
                        /*	if (GCEnabled == H_GC_RF_ENABLED){	// if GC is enabled then; we reference the last pointer stored in 'operation' (as this doesn't Ref in enquing 'requests'). I ignore the third argument in the request (H) as it is 'one' or 'zero' node, and it will not GCed anyway. this derefernceing happens when some pointers in 'operations' will be used to creates another BDD. This node will be DeReferenced at the end when the 'operation' list has only 2 pointers, as the last odd pointers remained untouched during the process
                                        gcQueue->Ref(Operations[j][LoopLimit]);
                                }*/
                            }
                            else {					// if the number of inputs is even
                                NumOfRemainingPointers[j] = LoopLimit ;
                                SetOperationCount(j, LoopLimit);
                            }
                        }
                        break;
                }
            }
            ReadCircuitFlag = false;
        }
        else { // this is the case when the function called for more than once (if one or more gates in the level has more than 2 input)
            GateCounter = StartCounter - 1; // initalize gate counter
            for (j = 0 ; j <  numOfGatesInLevel; j++) { // LOOP FOR EVERY GATE IN A LEVEL, including the input level
                GateCounter++; // next gate
                if ( GateCounter >= (2061 -1 ) ) {
                    int x;
                    x = 50;
                }
                NumOfInputs = GetOperationCount(j);	// get number of waited operations
                GateType = GetOperation(j);	// get gate type
                if (!isFinished(j)) {			// if the gate is not finished
                    LoopLimit = NumOfInputs >> 1;	// floor of number of gate divide by 2
                    for (i = 0; i < LoopLimit ; i++ ) {
                        f = Operations[j][(i << 1)];	// get the first input (2 x i)
                        g = Operations[j][(i << 1 ) + 1];	// get the second input (2 x i + 1)

                /*	if (GCEnabled == H_GC_RF_ENABLED){	// if GC is enabled then; we dereference 'f' and 'g' stored in 'operation' (this is the mainly way to remove temporary nodes). I ignore the third argument in the request (H) as it is 'one' or 'zero' node, and it will not GCed anyway. this derefernceing happens when some pointers in 'operations' will be used to creates another BDD.
                                gcQueue->DeRef(f);
                                gcQueue->DeRef(g);
                        }*/

                        switch (GateType)	// we have here only three basic types. the other types are derived from them by using inversion flag.
                        {
                            case OP_XOR:	// if the gate is NOR,
                                Operations[j][i] = Xor(f,g);			// perform operation
                                break;
                            case OP_AND:	// if the gate is AND,
                                Operations[j][i] = And(f,g);			// perform operation
                                break;
                            case OP_OR:	// if the gate is OR,
                                Operations[j][i] = Or(f,g);			// perform operation
                                break;
                            default:
                                break;
                        }

                    }
                    if (NumOfInputs == 2) {	// if the number of inputs are 2, then, set the gate are finished
                        finishedGates++;					// this gate is finished
                        cout << "gate no. : "<< GateCounter << " is finished" << endl;
                        SetFinishFlag (j);
                        if (isInverted(j)){
                            GateList[GateCounter-1] = Operations[j][0];
                            GateList[GateCounter-1].invert();
                        }
                        else
                            GateList[GateCounter-1] = Operations[j][0] ;

                    }

                    if ( (NumOfInputs & 0x1) == 1) {	//  if number of inputs is odd (add the last element)
                        Operations[j][LoopLimit] = Operations[j][NumOfInputs-1];	// add the last input
                        NumOfRemainingPointers[j] = LoopLimit+1 ;
                        SetOperationCount(j, LoopLimit+1);	// set number of
                    }
                    else {					// if the number of inputs is even
                        SetOperationCount(j, LoopLimit);
                        NumOfRemainingPointers[j] = LoopLimit ;
                        SetOperationCount(j, LoopLimit);	// set number of
                    }
                }
             }
        }
    }
    // get the forwarded pointer, this function return a pointer in the hash table corresponding to a forwarded pointer in the queue
    void getElement(BDDPointer <T,V> &bp){
        if (bp.isTemporary()) {	// if it is a temporary pointer, then get it from queue, otherwise, do nothing
            if (bp.isInverted()) {
                bp.ClearInverted();
                queue-> getElement (bp);	// get the forwarded element
                bp.invert();
            }
            else {
                queue-> getElement (bp);	// get the forwarded element
            }
        }
    }
    // this function regulate the pointers of gate (called at the end of each level)
    // reference are incremented for regulated pointers
    void RegulatePointers() {
        // regulate temporary list (operations list)
        int i,j;
        for (i = 0; i < numOfGatesInLevel; i++){
            if( NumOfRemainingPointers[i] > 1 ) { // regulate temporary list (operations list)
                for (j = 0; j < NumOfRemainingPointers[i] ; j++){
                    getElement (Operations[i][j] );
                }
            }
            else {	// regulate final list
                if (( i + StartCounter -1 ) == 2060){
                    cout << "operation schedular: RegulatePointers << regulate gate list no. : " << i + StartCounter -1 << endl;
                }
                getElement ( GateList[ i + StartCounter -1 ] );
            }
        }

    }
    // this function dump the next gate in the circuit
    void DumpTheLevelToDot() {
        int i;
        char fName[40] ;	// create file name
        sprintf (fName , "graph");
        char temp[10];

        for (i = StartCounter; i <= GateCounter; i++){
            sprintf(temp,"%d",i);
            strcat(fName, temp);
            strcat(fName, ".dot");
            dumper->OutputDot (GateList[ i - 1 ] ,fName );
            sprintf (fName , "graph");
        }
    }
    // return the bdd pointer of a gate, this function can't be called before teh circuit is constructed (gate start from 1)
    BDDPointer <T,V> getGateNum(int gate){
        return GateList[gate - 1];
    }

    // this do the same function, but used when you need to neglect the flag
    bool getNextLivePointer(BDDPointer <T,V> & bp){
        bool flag = false;
        return getNextLivePointer(bp,  flag);
    }
    // get the next live pointer in either GateList or 'operation'. the gate that already finished are from 0 to StartCounter-1 (we get them immediately). the gate at this round some of them are in the 'gatelist' and others in 'operation' list.
    // The second arguments "permenant flag" indicates wether the pointer from operation (false) or gate list (true)
    bool getNextLivePointer(BDDPointer <T,V> & bp, bool & PermenantFlag){
        int i;
        mutex.lock();
        // if the gate is one of the finished gate
        if(markedGates < (StartCounter -1)){
            i = markedGates;	// store index
            markedGates++;			// update gate counter
            mutex.unlock();			// unlock
            bp = GateList[ i ];	// get pointer
            PermenantFlag = true;
            if (bp.isTemporary()) {	// if it is a temporary pointer, then try next pointer, this is an impossible situation if every thing is OK
                cout << "OperationSchedular : getNextLivePointer : Fatal error, finished gates have a temporary pointer" << endl;
                exit(-1);
            }
    //	cout << "OperationSchedular : getNextLivePointer : gate no. " << i<< " is fetched for marking" << endl;
            return true;
        }
        // if we are now accessing gates in the current round (some in the 'gatelist' and others in 'operation')
        else if(markedGates < (numOfGatesInLevel+ StartCounter -1) ){	// if there is another gate
            i = markedGates - (StartCounter -1) ;
            if(NumOfRemainingPointers[i] == 1){	// if we have exactly one 'operations' , then 'gate list' will contain our pointer
                markedOperations = 0;	// initialize operation (as you will go to the next one)
                i = markedGates;	// store index
                markedGates++;			// update gate counter
                mutex.unlock();
                bp = GateList[ i ];	// get pointer
                PermenantFlag = true;
                if (bp.isTemporary() ) {	// if it is a temporary pointer, then try next pointer
                    return getNextLivePointer(bp);
                }

        //	cout << "OperationSchedular : getNextLivePointer : gate no. " << i<< " is fetched for marking" << endl;
                return true;
            }
            if( NumOfRemainingPointers[i] > markedOperations ){		// mark temporary list (operations list)
                int j = markedOperations;		// store no. of operation
//		cout << "OperationSchedular : getNextLivePointer : operation no. " << j << " in gate no. " << markedGates << " is fetched for marking" << endl;
                markedOperations++;			// update operation
                if (markedOperations ==  NumOfRemainingPointers[i]){	// if you reach the end of the list 'operation' , initialize its counter, and go to the next gate;
                    markedOperations = 0;
                    markedGates++;
                }
                mutex.unlock();				// unlock
                bp = Operations[i][j];			// get pointer
                PermenantFlag = false;			// non-permenant flag
                if (bp.isTemporary() ) {	// if it is a temporary pointer, then try next pointer
                    return getNextLivePointer(bp);
                }
                return true;
            }
            else {	// if the gatelist is exist	(because the NumOfRemainingPointers[i] = 0)
                i = markedGates;		// store index
                markedGates++;			// update gate counter
                mutex.unlock();			// unlock
//			cout << "OperationSchedular : getNextLivePointer : gate no. " << i<< " is fetched for marking" << endl;
                bp = GateList[ i ];	// get pointer
                PermenantFlag = true;
                if (bp.isTemporary() ) {	// if it is a temporary pointer, then try next pointer
                    return getNextLivePointer(bp);
                }
//		cout << "OperationSchedular : getNextLivePointer : gate no. " << i<< " is fetched for marking" << endl;
                return true;
            }
        }
        // otherwise initialize counters
        if (GCEnabled == H_GC_MARK_ENABLED) {	// for mark GC only; initialize gate counter. In incremental GC, we need to keep trace of how many gate are marked in the previous round. So that, we mark the new one only
            markedGates = 0;
        }
        else if (GCEnabled == H_GC_INCREMENTAL_MARK_ENABLED) {	// for incremental mark GC ; remove the last (unused increment)
            //markedGates --;
        }
        markedOperations = 0;
        mutex.unlock();			// unlock
        return false;	// if there is no more gates, then return true.
    }
    // return number of gates
    int gateCount(){
        return TotalNoOfGate;
    }
    void InitiateMarkedGate(){
        markedGates = StartCounter -1;
    }
    // dump gate list for some level
    void DumpGateList(int fileNo){
        char fName[40];
        sprintf(fName, "GateList %d .txt",fileNo);	// set file name
        ofstream outClientFile( fName, ios::out );
        int i;
        outClientFile << GateCounter << endl;	// store total gate count

        for (i = 0; i < GateCounter; i++){
            GateList[i].DumpToScreen();
        }
    }
};

#endif
