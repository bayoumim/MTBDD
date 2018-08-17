////////////////////////////////////////////////////////////////////////
// File: logic simulation
////////////////////////////////////////////////////////////////////////
//#include "cuddObj.hh"
#include <stdio.h>
#include <stdlib.h>
#include <strings.h> // or <string.h>
#include <cstring>
#include <string>
using namespace std;
#include <iostream>
#include <sstream>

//#include <string.h>
// using std::string;
main(int argc, char *argv[])
{
	stringstream ss (stringstream::in | stringstream::out);
	int i =6;
	ss << "wehds" << i << endl;
	cout << ss.str();
	string s(ss.str());
	cout << s;
/*	int VarNum = 10;
  	Cudd mgr(0,0);    		
	BDDvector h(1,&mgr);
	FILE *outfile;
	int i = 1;
	BDD f = mgr.bddZero();
//	BDD f = mgr.bddOne();
	char fName[40] ;
	//BDD x = mgr.bddZero();
	for (i = VarNum-1 ; i >=0; i--) {
		//f = f.Xor(mgr.bddVar());
		f = f.Nor(mgr.bddVar(i));
		h[0] = f;
		sprintf(fName,"out%d.dot",i);
		outfile = fopen(fName,"w");			
		h.DumpDot (NULL, NULL, outfile);
	}*/
/*	for (i = 0 ; i <VarNum; i++) {
		//f = f.Xor(mgr.bddVar());
		f = f.Nor(mgr.bddVar());
		h[0] = f;
		sprintf(fName,"out%d.dot",i);
		outfile = fopen(fName,"w");			
		h.DumpDot (NULL, NULL, outfile);
	}*/
//	BDD y = mgr.bddVar();
//	f = x.Xor(y);
	// output BDD to a file
/*	h[0] = f;
	sprintf(fName,"out1.dot");
	outfile = fopen(fName,"w");			
	h.DumpDot (NULL, NULL, outfile);
/*	h[0] = f;
	sprintf(fName,"out%d.dot",i);
	outfile = fopen(fName,"w");			
	h.DumpDot (NULL, NULL, outfile);*/
}

