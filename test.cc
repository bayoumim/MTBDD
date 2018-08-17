#include "PackageHeader.h"


int main(int argc, char *argv[]){
/*	char cktName[20];
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
	for (i = 0 ; i < ckt->getNoOfLevel() ; i++) {
		cout << ckt->getNumOfGateInLevel(i) << endl;
	}
	cout << "no of levels : " << ckt->getNoOfLevel() << endl;
	cout << "Max. no. of gates : " << ckt->getMaxNoOfGatesInLevel() << endl;
	cout << "Max. no. of Inputs : " << ckt->getMaxInputInGate() << endl;*/
/*	int inCapacity = 5000000;
	int HashMaxUtilization = 90;
	int VariableCount = 1;
	double QueueMaxUtilization = 90;
	int ThreadsNumber = 20;
	int HashConcurrrecyDegree = 100;
	int QueueConcurrencyDegree = 100;
	int HashBucketSize = 32;
	int QueueBucketSize = 32;
	bool enableComputedTable = true;
	char cktName[20];
    	circuit *ckt;
    	int vecWidth, poNum;
 	int i,j;	// counters
	int QueueSize = 2* inCapacity;
    	if (argc < 2)
    	{
        	fprintf(stderr, "Usage: %s <ckt>\n", argv[0]);
        	exit(-1);
    	}*/
	// default values
	int inCapacity = 5000000;
	int HashMaxUtilization = 90;
	//int VariableCount = 1;
	unsigned char QueueMaxUtilization = 90;
	int ThreadsNumber = 20;
	int HashConcurrrecyDegree = 100;
	int QueueConcurrencyDegree = 100;
	int HashBucketSize = 32;
	int QueueBucketSize = 32;
	bool enableComputedTable = false;
	char cktName[20];
    	circuit *ckt;
	sprintf(cktName, "c17");	// store the file name				 
  //  	int vecWidth, poNum;
 //	int i,j;
	int QueueSize = 2* inCapacity;
	bool fileExist = false;	// flag for the file
	int hashIntegrityCheck = 0;
	int GCEnabled = H_NO_GC;
	int GCThreadsNumber = 1;
	int VariableReordering = VR_NO_VARIABLE_REORDERING;
	int ResizeType = H_RESIZE_TYPE_SWAP;
	while((opt = getopt_long(argc, argv, ":f:t:i:a:b:d:ce:q:g:h:l:k:m:n:", longopts, NULL)) != -1) {
		switch(opt) {
			case 'f':	// get teh file name 
				 fileExist = true;
				 sprintf(cktName,"%s", optarg);	// store the file name				 
				 break;
			case 't':				// store number of threads
				 ThreadsNumber = atoi(optarg);
				 break;
			case 'i':				// store number of threads
				 inCapacity = atoi(optarg);
				 break;
			case 'a':				// hash concurrency degree
				 HashConcurrrecyDegree = atoi(optarg);
				 break;
			case 'b':				// hash Max. Utilization
				 HashMaxUtilization = atoi(optarg);
				 break;
			case 'd':				// hash bucket size
				 HashBucketSize = atoi(optarg);
				 break;
			case 'c':				// enable computed table
				 enableComputedTable = true;
				 break;
			case 'e':				// queue concurrency degree
				 QueueConcurrencyDegree = atoi(optarg);
				 break;
			case 'q':				// queue size
				 QueueSize = atoi(optarg);
				 break;
			case 'g':				// queue max. utlilization
				 QueueMaxUtilization = atoi(optarg);
				 break;
			case 'h':				// queue bucket size
				 QueueBucketSize = atoi(optarg);
				 break;
			case 'j':				// queue bucket size
				 hashIntegrityCheck = atoi(optarg);
				 break;
			case 'k':				// garbage collector
				 GCEnabled = atoi(optarg);
				 break;
			case 'l':				// garbage collector
				 GCThreadsNumber = atoi(optarg);
				 break;
			case 'm':				// garbage collector
				 VariableReordering = atoi(optarg);
				 break;
			case 'n':				// garbage collector
				 ResizeType = atoi(optarg);
				 break;
			default:
				printf("unknown option: %c\n", optopt);
				exit(-1);
				break;
		}
	}
	if (!fileExist)
    	{
        	fprintf(stderr, "Usage: %s <ckt>\n", argv[0]);
        	exit(-1);
    	}
	ckt = new circuit(cktName);	// create a circuit object
	//Manager manager(cktName,  inCapacity,  HashMaxUtilization,  QueueMaxUtilization,  ThreadsNumber,  HashConcurrrecyDegree,  QueueConcurrencyDegree,  HashBucketSize,  QueueBucketSize,  enableComputedTable);	// initialize manager
	Manager <unsigned int, unsigned char, unsigned short, int, unsigned short> manager (ckt,  inCapacity,  HashMaxUtilization,  QueueMaxUtilization,  ThreadsNumber,  HashConcurrrecyDegree,  QueueConcurrencyDegree, HashBucketSize,  	QueueBucketSize,  enableComputedTable, QueueSize, hashIntegrityCheck,GCEnabled,GCThreadsNumber,VariableReordering,ResizeType);	// initialize manager
//	manager.ConstructIteOuput();
	//struct timeval start, end;
   // 	long mtime, seconds, useconds; 
//	mtime = 0;

//	gettimeofday(&start, NULL);	// start calculation ()
	manager.getOutputCktBfIte ();	// build circuit
//	manager.Dump();	

/*	gettimeofday(&end, NULL);

	seconds  = end.tv_sec  - start.tv_sec;
	useconds = end.tv_usec - start.tv_usec;

	mtime = mtime + seconds * 1e6 + useconds ;		
	cout << "It takes " << seconds<<" seconds and "<< useconds <<" microsecond to finish ." << "Total time is " << seconds * 1e6 + useconds <<" microseconds\n";*/
	//char fName[40] ;	// create file name
	//sprintf (fName , "graph");
	//manager.OutputAllOutputToDot(fName);	// dump outputs to a file

/*	Manager manager( inCapacity,  HashMaxUtilization,  VariableCount, QueueMaxUtilization,  ThreadsNumber,  HashConcurrrecyDegree,  QueueConcurrencyDegree,HashBucketSize,QueueBucketSize, enableComputedTable);	// add initialization here
	manager.Dump();
	string s = "graph";
	string s1;
	int i;
	BDDPointer f = manager.getZero();	// f = 0
	//BDDPointer f = manager.getOne();	// f = 1
	BDDPointer g;
	char fName[40] ;	// create file name
	for (i = 0; i<VariableCount; i++){
//	for (i = 0; i<7; i++){
//	for (i = VariableCount - 1; i>= 0; i--){
		g = manager.getVariable(i);
		//f =  manager.And(f,g);
		//f =  manager.Or(f,g);
		//f =  manager.Not(g);
		//f =  manager.Xor(f,g);
		f =  manager.Nor(f,g);
		sprintf(fName, "graph%d.dot", i);
	//	manager.OutputDot(f, fName);		// dump h to dot file
	//	manager.Dump();	
	}*/
//	manager.finalize ();
/*	manager.Dump();	
	sprintf (fName , "graph0.dot");
	manager.OutputDot(f, fName);		// dump h to dot file
//	BDDPointer zero = manager.getZero(); // get zero
//	BDDPointer one = manager.getOne(); // get zero
	char fName[40] ="graph.dot";	// create file name
//	manager.OutputDot(one, fName);
	BDDPointer f = manager.getVariable(0);	// x0
	//f.Dump();
	BDDPointer g = manager.getVariable(1);	// x1
	//g.Dump();
	//BDDPointer h = manager.And(f,g);	// h = x1 . x0
	//BDDPointer h = manager.Or(f,g);	// h = x1 + x0
	//BDDPointer h = manager.Not(f);	// h = NOT( x0)
	//BDDPointer h = manager.Xor(f,g);	// h = x1 XOR x0
	//BDDPointer h = manager.Nor(f,g);	// h = x1 NOR x0 Nand
	//BDDPointer h = manager.Nand(f,g);	// h = x1 NAND x0 
	BDDPointer h = manager.Xnor(f,g);	// h = x1 NAND x0 
	manager.Dump();	
	manager.OutputDot(h, fName);		// dump h to dot file*/

}
