#ifndef _CLASS_CIRCUIT
#define _CLASS_CIRCUIT
#include "BDDUtil.h"

#include <vector> 
using std::vector;
#include <string> 
using std::string;
// defined the type of the gate
enum
{
   G_JUNK,         /* 0 */
   G_INPUT,        /* 1 */
   G_OUTPUT,       /* 2 */
   G_XOR,          /* 3 */
   G_XNOR,         /* 4 */
   G_DFF,          /* 5 */
   G_AND,          /* 6 */
   G_NAND,         /* 7 */
   G_OR,           /* 8 */
   G_NOR,          /* 9 */
   G_NOT,          /* 10 */
   G_BUF,          /* 11 */
   G_TIE1,         /* 12 */
   G_TIE0         /* 13 */
};

////////////////////////////////////////////////////////////////////////
// circuit class
////////////////////////////////////////////////////////////////////////

class circuit
{
    	// circuit information
	char *cktName;
    	int numgates;       // total number of gates (faulty included)    
    	int maxlevels;      // number of levels in gate level ckt
    	int numInputs;	// number of inputs
    	int numOutputs;	// number of outputs
    	unsigned char *gtype;	// gate type (This is a 1D array with starting index of 1)
    	short *fanin;		// number of fanins of gate
    	short *fanout;		// number of fanouts of gate
    	int *levelNum;		// level number of gate
    	int **faninlist;		// fanin list of gate (this is a 2D array, row start from 1, and columns start from 0 )
    	int **fanoutlist;		// fanout list of gate
    	vector< int > OutputList;	// store a list of the output gate (the index is start from 0)
    	vector< int > InputList;	// store a list of the input gate (the index is start from 0)
	int * GateNumInEachLevel;
	int MaxNoOfGatesInALevel;	// store the maximun no of gates in a level
	int MaxNoOfInputs;		// store the max. no. of input for any gate
	int GateCount;
	int GateIndex;			// counter to get the next gate to process 
	// lists used in wieghted heuristics variable reordering
	double *HeuristicWeighGateList;	// contains the weights for each gate
	int *MarkedGateList;		// indicate taht whether the gate is included (not marked) or not (marked)
	void propagateWeight(int,double);
	int getMaxWeightedVar();
	void markFanOutputGates(int);
public:
	circuit(char *);            // constructor
	//circuit(string);	// overloaded constructor
    	// you may wish to add more function declarations (or public variables) here
    	int getNumGates();
	int getNumInput();
	int getNumoutput();
	
	int getGateType(int);
	int getGateInNum(int);
	int*  getGateInList(int );	
	int getGateOutputNum(int gate);
	int getNoOfLevel();
	int getMaxNoOfGatesInLevel();
	int getMaxInputInGate();
	int getNumOfGateInLevel(int);
	int getGateCount();
	int getNextGate();
	void ResetGateCounter();
	char * getCircuitName();
	void InitiateWeightedHeuristic();
	int getNextVariable();
	int getGateInputNum(int);
//	int getNumOfGateInLevel(int);
	//void Init(char *);
};

#endif
