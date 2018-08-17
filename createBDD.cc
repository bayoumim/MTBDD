
////////////////////////////////////////////////////////////////////////
// File: logic simulation
////////////////////////////////////////////////////////////////////////
#include "cuddObj.hh"
#include <iostream>
#include <ctype.h>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h> // or <string.h>
#include <cstring>
#include <sys/time.h>
#include "circuit.hh"
#include "InputVectors.h"

//#include <string.h>
// using std::string;
#include <vector> 
using std::vector;
using namespace std;
using std::ofstream; // output file stream
using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::ios;

#include <string> 
using std::string;
#include <cstring>
#include <sstream>
using std::stringstream;


////////////////////////////////////////////////////////////////////////
// main starts here
////////////////////////////////////////////////////////////////////////

// you MUST type the name without it's extention
// Example: ./cirucuit c17

BDD createBDD(circuit *,Cudd *, int );
circuit *ckt;
BDD *f ;
Cudd mgr(0,0);    		
BDDvector h(1,&mgr);
int GateNum;	// counters
int *varLookUpList;
int VariableCount;
// simulate one BDD (the index start from 1)
char SimulateBDD(int GateNum, char * inputvec){
//	cout << "Evaluate gate no. " << GateNum << endl;
	BDD bp ;
	int VariableCount = ckt->getNumInput();	// get number of input
	// we use two input vectors in case of " don't care" input
	int *Input1  = new int[VariableCount];	
	int *Input2  = new int[VariableCount];
	int i;
	int flag = false;
	// convert input from string to integer
	for (i =0; i < VariableCount; i++){
		int index = VariableCount - 1 - i;	
//		cout << "index is : " << index << endl;
	//	int index = i;	
		if (inputvec[i] == '0'){
			Input1[index] = 0;
			Input2[index] = 0;
		}
		else if (inputvec[i] == '1'){
			Input1[index] = 1;
			Input2[index] = 1;
		}
		else {	// if the input is 'X', then one input is '1' and other is '0'
			flag = true;
			Input1[index] = 0;
			Input2[index] = 1;
		}
	//	cout << "input 1 is " << Input1[i] << " input 2 is " << Input2[i] << endl;
	}

/*	for (i =0; i < VariableCount; i++){
		cout << "input 1 is " << Input1[i] << " input 2 is " << Input2[i] << endl;
	}*/
//	cout << "Input 1 " << Input1 << endl;
	bp = f[GateNum].Eval(Input1);	// evalutate for the first input
	char c1;
	char c2;
	// convert the output form BDD to char
	if (bp == mgr.bddOne() ){
		c1 = '1';
//		cout << "input 1 is 1" << endl;
	}
	else {
		c1 = '0';
//		cout << "input 1 is 0" << endl;
	}
		
	if(flag){	// if the input contains 'X'	
		BDD bp = f[GateNum].Eval(Input2);	// evaluate the second input
		if (bp == mgr.bddOne() ){
			c2 = '1';
//			cout << "input 2 is 1" << endl;
		}
		else{
			c2 = '0';
//			cout << "input 2 is 0" << endl;
		}

		if (c1 != c2)	// if the output are not matched, then return 'x'
			return 'X';
	}	
	return c1;
}

// simulate the circuit for single input
char * SimulateCircuitForInput(char * inputvec){
	int NumOfOutput = ckt-> getNumoutput();	// get number of output
	int VariableCount = ckt->getNumInput();	// get number of input
	
	int i;	
	char * Output = new char[NumOfOutput+1];	// reserve space for output
	Output[NumOfOutput] = 0;			// null terminating charachter
	for(i=0; i<NumOfOutput; i++){		// for every output
		Output[i] = SimulateBDD(ckt->getGateOutputNum(i),inputvec);	// simulate the output
	}	
	return 	Output;
}

void openVarOrder(){
	char fName[40];
	char *cktName = ckt->getCircuitName();	// get the name of the circuit (c432, c880 ... etc)
	strcpy(fName, cktName);
	strcat(fName, ".varOrder");
	FILE *inFile;		// handle to the input (*.lev) file
	inFile = fopen(fName, "r");
	if (inFile == NULL)
	{
		fprintf(stderr, "Can't open .varOrder file\n");
		exit(-1);
	}
	int i;
	int inCount = ckt->getNumInput();
	varLookUpList = new int[inCount+1];
	int j;
	for(i=1; i<= inCount; i++){
	   	fscanf(inFile, "%d", &j);	// number of gates	
		varLookUpList[i] = j +1;
	}
    	fclose(inFile);
}
main(int argc, char *argv[])
{
	VariableCount = 0;
	char cktName[20];
  	int i,j;	// counters

    	if (argc < 2)
    	{
        	fprintf(stderr, "Usage: %s <ckt>\n", argv[0]);
        	exit(-1);
    	}

    	strcpy(cktName, argv[1]);


	FILE *outfile;
//	BDD f = mgr.bddZero();
//	BDD f = mgr.bddOne();
	char fName[40] ;
   	ckt = new circuit(cktName);
	GateNum = ckt->getGateCount();
	f = new BDD[GateNum+1];
	openVarOrder();
	for(i=1; i<= GateNum; i++)
	{
		f[i] = createBDD(ckt, &mgr, i);	// create BDD
		// output BDD to a file
	/*	h[0] = f[i];
		sprintf(fName,"out%d.dot",i);
		outfile = fopen(fName,"w");
		h.DumpDot (NULL, NULL, outfile);*/
	/*	h[0] = f[i];
		sprintf(fName,"gate%d.blif",i);
		outfile = fopen(fName,"w");
		h.DumpBlif (NULL, NULL, NULL, outfile, NULL);	*/
	}

	cout << "Number of variables: " <<  mgr.ReadSize() << "\n";
  	cout << "Node count: " <<  mgr.ReadNodeCount() << "\n";
  	cout << "Number of reorderings: " << mgr.ReadReorderings() << "\n";
  	cout << "Memory in use: " << mgr.ReadMemoryInUse() << " bytes \n\n";		

/*	InputVectors *vct;		// pointer to object that deals with input vectors

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
	}*/
	delete [] f;

/*int main(int argc, char *argv[])
{
	char cktName[20];
    	circuit *ckt;
    	int vecWidth, poNum;
 	int i,j;	// counters

    	if (argc < 2)
    	{
        	fprintf(stderr, "Usage: %s <ckt>\n", argv[0]);
        	exit(-1);
    	}

    	strcpy(cktName, argv[1]);

    	// read in the circuit and build data structure
   	ckt = new circuit(cktName);
	
		
  	Cudd mgr1(0,0);		// initialization value	
  	Cudd mgr(0,0);    
	BDDvector h1(1,&mgr);	// initialization value		
	BDDvector h(1,&mgr);
	FILE *outfile;
	BDD f;
	int outputNum = ckt->getNumoutput();	//get the number of ouput gates
	int outputGateNum;
	char fName[40], fName1[40], fName2[40], fName3[40], fName4[40], fName5[40], fName6[40], fName7[40] ;
	struct timeval start, end;
    	long mtime, seconds, useconds; 
	mtime = 0;
	// remove it is a temporary to defined a certain ouput
	//i = atoi(argv[2]);
	//cout << outputNum;
	
	//mgr.AutodynEnable(CUDD_REORDER_EXACT);
	
	sprintf(fName, " ");
	sprintf(fName1, " ");
	sprintf(fName2, " ");
	sprintf(fName3, " ");
	sprintf(fName4, " ");
	sprintf(fName5, " ");
	sprintf(fName6, " ");
	sprintf(fName7, " ");
	
	strcat(fName, argv[1]); 

	strcpy(fName1, fName);
	strcpy(fName2, fName);
	strcpy(fName3, fName);
	strcpy(fName4, fName);
	strcpy(fName5, fName);
	strcpy(fName6, fName);
	strcpy(fName7, fName);
	
	cout << "-----------------------------------------------------------------------------------\n";
	cout << "output for circuit " << fName << "\n";
	strcat(fName1, "gateNum.txt");
	strcat(fName2, "size.txt");
	strcat(fName3, "nodecount.txt");
	strcat(fName4, "reordering.txt");
	strcat(fName5, "reordertime.txt");
	strcat(fName6, "mem.txt");
	strcat(fName7, "time.txt");
	
	strcat(fName, "out.txt"); 
	//sprintf(fName,"out.txt");
	ofstream outClientFile( fName, ios::out );
	ofstream outClientFile1( fName1, ios::out );
	ofstream outClientFile2( fName2, ios::out );
	ofstream outClientFile3( fName3, ios::out );
	ofstream outClientFile4( fName4, ios::out );
	ofstream outClientFile5( fName5, ios::out );
	ofstream outClientFile6( fName6, ios::out );
	ofstream outClientFile7( fName7, ios::out );
	
	for(i=0; i<outputNum; i++)
	{
		outputGateNum = ckt->getGateOutputNum(i);		
		//printf("the output gate %d\n",outputGateNum);

		gettimeofday(&start, NULL);	// start calculation ()
		f = createBDD(ckt, &mgr, outputGateNum);	// create BDD
		gettimeofday(&end, NULL);

		seconds  = end.tv_sec  - start.tv_sec;
    		useconds = end.tv_usec - start.tv_usec;

		mtime = mtime + seconds * 1e6 + useconds ;		
		//outClientFile << "It takes " << seconds<<" seconds and "<< useconds <<" microsecond to finish ." << "Total time is " << seconds * 1e6 + useconds <<" microseconds\n";

		// temporary commented
		// output BDD to a file
		/*h[0] = f;
		sprintf(fName,"out%d.dot",outputGateNum);
		outfile = fopen(fName,"w");
		h.DumpDot (NULL, NULL, outfile);*/		
		/*outClientFile << "Output No. : " << outputGateNum << "\n";;			
		outClientFile << "Number of variables: " <<  mgr.ReadSize() << "\n";
  		outClientFile << "Node count: " <<  mgr.ReadNodeCount() << "\n";
  		outClientFile << "Number of reorderings: " << mgr.ReadReorderings() << "\n";
		outClientFile << "Time spent in reordering : " << mgr.ReadReorderingTime() << "\n";		
  		outClientFile << "Memory in use: " << mgr.ReadMemoryInUse() /1024/1024<< "MB \n\n";*/
/*		outClientFile1 << outputGateNum << "\n";;			
		outClientFile2 << mgr.ReadSize() << "\n";
  		outClientFile3 << mgr.ReadNodeCount() << "\n";
  		outClientFile4 <<  mgr.ReadReorderings() << "\n";
		outClientFile5 <<  mgr.ReadReorderingTime() << "\n";		
  		outClientFile6 <<  mgr.ReadMemoryInUse() /1024/1024 << "\n";
		outClientFile7 <<  seconds * 1e6 + useconds <<"\n";

		cout << "output " << i+1 << " out of " << outputNum << "\n" ;
		cout << "It takes " << seconds<<" seconds and "<< useconds <<" microsecond to finish ." << "Total time is " << seconds * 1e6 + useconds <<" microseconds\n";
		cout << "Number of variables: " <<  mgr.ReadSize() << "\n";
  	cout << "Node count: " <<  mgr.ReadNodeCount() << "\n";
  	cout << "Number of reorderings: " << mgr.ReadReorderings() << "\n";
  	cout << "Memory in use: " << mgr.ReadMemoryInUse() << " bytes \n\n";		
	}
	outClientFile << " The total time of the whole circuit is  " << mtime << " microseconds\n";
	outClientFile.close();
	/*cout << "Number of variables: " <<  mgr.ReadSize() << "\n";
  	cout << "Node count: " <<  mgr.ReadNodeCount() << "\n";
  	cout << "Number of reorderings: " << mgr.ReadReorderings() << "\n";
  	cout << "Memory in use: " << mgr.ReadMemoryInUse() << " bytes \n\n";*/



/*	// create a file to store the result (time and number of nodes)
	int y = mgr.ReadNodeCount();	
	sprintf(fName,"NumNodeOut%d.txt",i);
	ofstream outClientFile( fName, ios::out );	
	outClientFile << y << " ";
	outClientFile.close();

	y = mtime;	
	sprintf(fName,"stat%d.txt",i);
	ofstream outClientFile1( fName, ios::app );	
	outClientFile1 << y << " ";
	outClientFile1.close();

	
	return y;*/
}
int map (int gate){
	return varLookUpList[gate] ;
}
BDD createBDD(circuit *ckt,Cudd *mgr, int gate)
{
	//mgr->ReduceHeap(CUDD_REORDER_SYMM_SIFT_CONV,1000);
	//mgr->ReduceHeap(CUDD_REORDER_SYMM_SIFT,5000);	// optimal for c2670
	//mgr->ReduceHeap(CUDD_REORDER_SYMM_SIFT,100000); // optimal for c3540
	//mgr->ReduceHeap(CUDD_REORDER_SYMM_SIFT,100000); // optimal for c5315
	int InCount = ckt->getNumInput();
	//mgr->ReduceHeap(CUDD_REORDER_SYMM_SIFT,100000); // optimal for c7552
	BDD f, g,tmp;
	int gateType =ckt->getGateType(gate);
	if (gateType == G_INPUT)	// the gate is input, then it is a base case
	{
	//	int x = ckt->getGateInputNum(VariableCount);
		int x = gate;
		f = mgr->bddVar(InCount - map(x));	// get that variable
		VariableCount++;
	}
	else // the gate is not a basic variable
	{
		int inCount = ckt->getGateInNum(gate);		// get number of inputs
		int i;
		int *inlist = ckt->getGateInList(gate);		// get the input list
		switch (gateType)	// set the initial value for the function depends on the gate
		{
			case G_XOR:	// implement XOR Function
				f = mgr->bddZero();;	// f = 
				for (i=0; i<inCount; i++)
				{	
      					f = f.Xor(createBDD(ckt,mgr,inlist[i]));					
				}								
				break;
			case G_XNOR:	// implement XOR Function
				f = mgr->bddZero();;	// f = 
				for (i=0; i<inCount; i++)
				{	
      					f = f.Xor(createBDD(ckt,mgr,inlist[i]));					
				}
				f = ~f;								
				break;
			case G_AND:	// implement AND Function
				f = mgr->bddOne();;	// f = 1
				for (i=0; i<inCount; i++)
				{	
      					f = f & createBDD(ckt,mgr,inlist[i]);					
				}								
				break;
			case G_NAND:	// implement NAND Function
				f = mgr->bddOne();;	// f = 1
				for (i=0; i<inCount; i++)
				{	
      					f = f & createBDD(ckt,mgr,inlist[i]);					
				}
				f = ~f;				
				break;
			case G_OR:	// implement OR Function
				f = mgr->bddZero();	// f = 0
				for (i=0; i<inCount; i++)
				{	
      					f = f |createBDD(ckt,mgr,inlist[i]);					
				}			
				break;
			case G_NOR:	// implement NOR Function
				f = mgr->bddZero();	// f = 0
				for (i=0; i<inCount; i++)
				{	
      					f = f |createBDD(ckt,mgr,inlist[i]);					
				}	
				f = ~f;
				break;
			case G_NOT:	// implement NOT Function
				f = ~createBDD(ckt,mgr,inlist[0]);;
				break;
			case G_BUF:	// implement Buffer Function
				f = createBDD(ckt,mgr,inlist[0]);;
				break;

	
			case G_OUTPUT:	// if it is an ouput gate, then it's input is the output of the previous gate
				f = createBDD(ckt,mgr,inlist[0]);	//								
				break;
			default:		
				printf("gate type is not defined%d\n",gateType);			
				break;
		}			
	}
	return f;
}
