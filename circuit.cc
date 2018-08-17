////////////////////////////////////////////////////////////////////////
// File: logic simulation
////////////////////////////////////////////////////////////////////////
#ifndef _CLASS_CIRCUIT_IMPLEMENTATION
#define _CLASS_CIRCUIT_IMPLEMENTATION

#include "BDDUtil.h"
#include "circuit.hh"


////////////////////////////////////////////////////////////////////////
// constructor: reads in the *.lev file and builds basic data structure
// 		for the gate-level ckt
////////////////////////////////////////////////////////////////////////
circuit::circuit(char *cktName)
{
    this->cktName = cktName;
    GateIndex = 0;
    numInputs = 0;
    numOutputs = 0;
    FILE *inFile;		// handle to the input (*.lev) file
    char fName[40];		// max file name is 40 characters
    int i, j, count;
    char c;
    int gatenum, junk;
    int f1, f2, f3;

    strcpy(fName, cktName);	// concatenate the file name with it's extenstion
    strcat(fName, ".lev");
    inFile = fopen(fName, "r");
    if (inFile == NULL)
    {
        fprintf(stderr, "Can't open .lev file\n");
        exit(-1);
    }

    numgates = maxlevels = 0;
    fscanf(inFile, "%d", &count);	// number of gates
    fscanf(inFile, "%d", &junk);	// skip the second line

    //this->GateCount = count;

    // allocate space for gates data structure	(ALL THESE STRUCTURES ARE START WITH 1, WHEN WE CONCERNING GATE NUMEBER, )
    gtype = new unsigned char[count];	// alocate space to store each type of each gate
    fanin = new short[count];		// alocate space to store no of inputs for each gate
    fanout = new short[count];		// alocate space to store no of fanout for each gate
    levelNum = new int[count];		// alocate space to store no of levels for each gate
    faninlist = new int * [count];	// alocate space to store list of inputs for each gate
    fanoutlist = new int * [count];	// alocate space to store list of outputs for each gate

    // now read in the circuit
    for (i=1; i<count; i++)
    {
        fscanf(inFile, "%d", &gatenum);	// read the number of the gate
        fscanf(inFile, "%d", &f1);	// read the type of the gate
        fscanf(inFile, "%d", &f2);	// read the level of the gate
        fscanf(inFile, "%d", &f3);	// read the number of fanin (inputs)

        numgates++;
        gtype[gatenum] = (unsigned char) f1;	// store the type of the gate


        if (gtype[gatenum] > 13)		// check the range of the type
            printf("gate %d is an unimplemented gate type\n", gatenum);
        else if (gtype[gatenum] == G_INPUT){	// update number of inputs (the whole circuit)
            numInputs++;
            InputList.push_back(gatenum);	//store number of gate in the list
        }
        else if (gtype[gatenum] == G_OUTPUT)	// update number of outputs (the whole circuit)
        {
            OutputList.push_back(gatenum);	//store number of gate in the list
            numOutputs++;
        }

        // store the level of the gate
        f2 = (int) f2;
        levelNum[gatenum] = f2;

        if (f2 >= (maxlevels))
            maxlevels = f2 + 5;

        fanin[gatenum] = (int) f3;
        // now read in the faninlist
        faninlist[gatenum] = new int[fanin[gatenum]];
        for (j=0; j<fanin[gatenum]; j++)
        {
            fscanf(inFile, "%d", &f1);
            faninlist[gatenum][j] = (int) f1;
        }

        // CAN SKIP THE RESET OF THE LINE IF DESIRED
        for (j=0; j<fanin[gatenum]; j++) // followed by samethings
            fscanf(inFile, "%d", &junk);

        // read in the fanout list
        fscanf(inFile, "%d", &f1);
        fanout[gatenum] = (int) f1;

        // now read in the fanoutlist
        fanoutlist[gatenum] = new int[fanout[gatenum]];
        for (j=0; j<fanout[gatenum]; j++)
        {
            fscanf(inFile, "%d", &f1);
            fanoutlist[gatenum][j] = (int) f1;
        }

        // skip till end of line
        while ((c = getc(inFile)) != '\n' && c != EOF)
            ;
    }	// for (i...)
    fclose(inFile);

    this->maxlevels = maxlevels/5 ;	// NUMBER OF level (including the input stage)
    GateNumInEachLevel = new int[this->maxlevels + 1];		// alocate space to store no of levels for each gate (index start from 0)

    for (i = 0;i <= maxlevels ; i++){	// initialize the list
            GateNumInEachLevel [i] = 0;
    }

    // calculate number of gate in each level, max number of level, and
//	int c = 
    MaxNoOfGatesInALevel = 0;
    MaxNoOfInputs = 0;
    int index;
    for(i = 1;i < count ; i++){
        index = levelNum[i] / 5 ;
        GateNumInEachLevel [ index ]++;			// increment the level counters

        if ( (GateNumInEachLevel[index] > MaxNoOfGatesInALevel) ) {
            MaxNoOfGatesInALevel = GateNumInEachLevel[index]; // store the maximum no. of gates
        }
        if ( (fanin[i] > MaxNoOfInputs) && (gtype[i] != G_INPUT) ) {
            MaxNoOfInputs = fanin[i]; // store the maximum no. of gates
        }		

    }
}



////////////////////////////////////////////////////////////////////////
// other member functions here
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
// getNumGates: retun number of gates (without including inputs and ouputs)
////////////////////////////////////////////////////////////////////////
int circuit::  getNumGates()
{
    return numgates - numInputs - numOutputs;
}

////////////////////////////////////////////////////////////////////////
// getNumInput: retun number of Inputs (variables)
////////////////////////////////////////////////////////////////////////
int circuit::  getNumInput()
{
    return numInputs;
}

////////////////////////////////////////////////////////////////////////
// getNumoutput: retun number of outputs 
////////////////////////////////////////////////////////////////////////
int circuit::  getNumoutput()
{
    return numOutputs;
}
////////////////////////////////////////////////////////////////////////
// getNumoutput: retun the gate type 
////////////////////////////////////////////////////////////////////////
int circuit::  getGateType(int gate)
{
    return gtype[gate];
}
////////////////////////////////////////////////////////////////////////
// getNumoutput: retun number of inputs of a gate
////////////////////////////////////////////////////////////////////////
int circuit::  getGateInNum(int gate)
{
    return fanin[gate];
}
////////////////////////////////////////////////////////////////////////
// getGateOutputNum: retun outputs number 'gate' (input start at 0)
////////////////////////////////////////////////////////////////////////
int circuit::  getGateOutputNum(int gate)
{
    return OutputList[gate];
}
////////////////////////////////////////////////////////////////////////
// getGateInputNum: retun input no. 'gate' (input start at 0)
////////////////////////////////////////////////////////////////////////
int circuit::  getGateInputNum(int gate)
{
    return InputList[gate];
}
////////////////////////////////////////////////////////////////////////
// getNumoutput: retun number of outputs 
////////////////////////////////////////////////////////////////////////
int* circuit::  getGateInList(int gate)
{
    int lengthIn = getGateInNum(gate);
    int *list = new int[lengthIn];
    int i;
    for(i = 0; i< lengthIn; i++)
        list[i]=faninlist[gate][i];

    return list;
}
////////////////////////////////////////////////////////////////////////
// getNoOfLevel: return the number of level 
////////////////////////////////////////////////////////////////////////
int circuit::getNoOfLevel()
{
    return this->maxlevels;
}
////////////////////////////////////////////////////////////////////////////
// getMaxNoOfGatesInLevel: return the max number of gates in a single level 
////////////////////////////////////////////////////////////////////////////
int circuit::getMaxNoOfGatesInLevel()
{
    return MaxNoOfGatesInALevel;
}
/////////////////////////////////////////////////////////////////////////////////////////////
// getMaxNoOfGatesInLevel: return the max number of inputs in all gates in the whole circuit
/////////////////////////////////////////////////////////////////////////////////////////////s
int circuit::getMaxInputInGate()
{
    return MaxNoOfInputs;
}
/////////////////////////////////////////////////////////////////////////////////////////////
// getNumOfGateInLevel: return the number of gates in a level, input: number of level
/////////////////////////////////////////////////////////////////////////////////////////////s
int circuit::getNumOfGateInLevel(int level)
{
    return GateNumInEachLevel[level];
}
///////////////////////////////////////////////////////////////////////////////////////////////
// getGateCount: return the total number of gates in the circuit (including inputs and outputs)
///////////////////////////////////////////////////////////////////////////////////////////////
int circuit::getGateCount()
{
    return numgates;
}
///////////////////////////////////////////////////////////////////////////////////////////////
// getNextGate: return the index of the next gate to process
///////////////////////////////////////////////////////////////////////////////////////////////
int circuit::getNextGate()
{
    return ++GateIndex;
}
///////////////////////////////////////////////////////////////////////////////////////////////
// ResetGateCounter: reset the index of gate
///////////////////////////////////////////////////////////////////////////////////////////////
void circuit::ResetGateCounter()
{
    GateIndex = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////
// ResetGateCounter: reset the index of gate
///////////////////////////////////////////////////////////////////////////////////////////////
char * circuit::getCircuitName()
{
    return cktName;
}
// 
void circuit::InitiateWeightedHeuristic(){
    HeuristicWeighGateList = new double[numgates+1];
    MarkedGateList = new int[numgates+1];
    int i;
    // initialize weighted and marked list for every gate
    for(i = 1; i<= numgates; i++){
        MarkedGateList[i] = 0 ;
    }
}
// recurrsively assign weights for a gate (start from index 1)
void circuit:: propagateWeight(int gate, double propagatedWeight){
    int faninNum = 	fanin[gate];
    int i;
    int indegree = 0 ;
    int in;
    if( gtype[gate] == G_INPUT )	// if it is an input gate
            return;
    //
    // compute in-degree
    for(i = 0; i< faninNum; i++){
        in = faninlist[gate][i];	// get input no. i
        if(MarkedGateList[in] == 0){	// if this input is not marked, then take it into account
            indegree++; // increments in-degree
        }
    }
    double weight = propagatedWeight/indegree;	// compute the weight for every fan-in gates
    // if there is un-marked gates; then assign it a weight and to 	its predecessors
    for(i = 0; i< faninNum; i++){
        in = faninlist[gate][i];	// get input no. i
        if(MarkedGateList[in] == 0){	// if this input is not marked, then update its weight
            HeuristicWeighGateList[in]+= weight;
            propagateWeight(in,weight);
        }
    }
}
// get variable with max. weight, skiping marked inputs
int circuit::getMaxWeightedVar(){
    int i;
    double max = 0;
    int var,j;
    for(i = 0; i< numInputs; i++){
        j = InputList[i];	// get input no. i
        if(MarkedGateList[j] == 0){	// if it is not marked variable
            if(max < HeuristicWeighGateList[j]){
                max = HeuristicWeighGateList[j];	// set new max. weight
                var = j;				// store this input
            }
        }
    }

    MarkedGateList[var] = 1;	// mark this variable
    return var;
}
// mark gates whose ALL inputs are marked
void circuit::markFanOutputGates(int gate){
    int fanout1 = fanout[gate];	// get fanout
    int i;
    for(i = 0; i< fanout1; i++){	// for every gate in fanout
        int j = fanoutlist[gate][i];	// get the i-th fanout gate
        int fanin1 = fanin[j];		// get the corresponding fan-in gate for j- gate
        int k;
        bool flag = true;		// flag (true : all inputs are marked)
        for(k = 0; k< fanin1; k++){	// for every gate in fanin list
            int in = faninlist[j][k];
            if (MarkedGateList[in] == 0){	// if it is not marked variable
                flag = false;
                break;
            }
        }
        if(flag){	// if all inputs of gate j is marked
            MarkedGateList[j] = 1; // mark j
            markFanOutputGates(j);	// see next gate
        }
    }
}

int circuit::getNextVariable(){
    int i;
    // initialize weighted and marked list for every gate
    for(i = 1; i<= numgates; i++){
        HeuristicWeighGateList[i] = 0 ;
    }

    // set weight for every output gate
    int j;
    for(i = 0; i< numOutputs; i++){
        j = getGateOutputNum(i);
        HeuristicWeighGateList[j] = 1 ;
        propagateWeight(j,1);	// propagates weight (1) to fan-in gates (recurrsively)
    }
    // get variable with maxmium wight
    int var = getMaxWeightedVar();
    markFanOutputGates(var);
    var--;
    cout << " circuit::getNextVariable : variable no. " << var << endl;
    return var ;
}


#endif

