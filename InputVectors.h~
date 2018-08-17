////////////////////////////////////////////////////////////////////////
// Input Vectors class
////////////////////////////////////////////////////////////////////////
#ifndef _INPUT_VECTOR_CLASS
#define _INPUT_VECTOR_CLASS
#include "BDDUtil.h"

// this class is responsible of reading the input vectors that used to simulate the circuit
class InputVectors
{
    // vector information
	int vecnum;		// store number of input vector
	vector <char*> inputVec;	// will point to list of input vectors stored in the .vec file
     // you may wish to add more private variables here

public:
    	InputVectors(char *vecName){            // constructor
		FILE *inFile;
		char fName[256];
		int j;
		int numInput;		// store number of input vector

		strcpy(fName, vecName);
		strcat(fName, ".vec");
		inFile = fopen(fName, "r");

		if (inFile == NULL)
		{
			fprintf(stderr, "Can't open .vec file\n");
			exit(-1);
		}
		vecnum = 0;
		fscanf(inFile, "%d", &numInput);	// number of input vector

		// allocate space for input vectors (as we have unkown number ofinput vectors, and each is 'vecnum+1' length - don't forget the null charachter at the end of each input vector). So, we will use a dynamic data structure "Vector" to store the input vectors
		char *tempVec ;	// allocate one temporary vector to read

		// read input vectors
		do {
			tempVec = new char[numInput+1]; 	// allocate space for new vector
			fscanf(inFile , "%s",tempVec);	// read a line from the .vec file (corresponding to a single input vector)
			tempVec[numInput] = 0;		// set null character at the end of the string

			// change the case from 'x' to 'X'
			for(j = 0; j< numInput; j++){
				if (tempVec[j] == 'x')	// if an input is 'x'
					tempVec[j] = 'X';	// then change it to 'X'
			}
			// check if the line you read is 'input vector' or 'END of the file' 
			if (strcmp( tempVec , "END") != 0)	// if there is an input vector in the file
				inputVec.push_back(tempVec);
			else {	// otherwise, leave while loop
				delete [] tempVec;
				break;
			}	
			vecnum++;
		}
		while(true);

	}
////////////////////////////////////////////////////////////////////////
// other member functions here
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
// getNumberOfVectors: retun number of input vectors
////////////////////////////////////////////////////////////////////////
	int getNumberOfVectors(){
		return vecnum;
	}
////////////////////////////////////////////////////////////////////////
// getInputVectors: retun number of input vectors
////////////////////////////////////////////////////////////////////////
	vector <char*> getInputVectors(){
		return inputVec;
	}

};
#endif
