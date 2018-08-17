// this class used to output BDD to various types
#ifndef _DUMPER_CLASS
#define _DUMPER_CLASS
#include "EdgeQueue.h"
#include "BDDPointer.h"
#include "circuit.hh"
#include "Queue.h"
#include "VariableCreator.h"
#include "TripleChecker.h"
#include "BDDUtil.h"

template<typename T,typename V,typename L,typename K,typename FLG>
class Dumper {
private:
    int fileCounter;
    unsigned short ThreadsNo;
    L VariableCount;
    BDDPointer <T,V> Zero;
    BDDPointer <T,V> One;
    BDDPointer <T,V> *Variable;
    Hash  <T,V,L,K,FLG> * UniqueTable;
    Queue <T,V,L,K,FLG> * queue;
    circuit *ckt;	// pointer to the circuit
    BDDPointer <T,V> *output;	// pointer to output list
    VariableCreator <T,V,L,K,FLG> *vc;

    // check if BDD Node is inverted
    bool isInverted (BDDPointer <T,V> &f) {
        return f.isInverted();
    }

    BDDPointer <T,V> getRegularEdge (BDDPointer <T,V> &f) {
        BDDPointer <T,V> bp = f;
        bp.ClearInverted();
        return bp;
    }
    // get number of output
    int getNumberOfOutput(){
        return ckt-> getNumoutput();
    }

public:
    Dumper (unsigned short ThreadsNo,
            L VariableCount,
            BDDPointer <T,V> Zero,
            BDDPointer <T,V> One,
            BDDPointer <T,V> *Variable,
            Hash <T,V,L,K,FLG> * UniqueTable,
            Queue <T,V,L,K,FLG> * queue,
            BDDPointer <T,V> *output,
            VariableCreator<T,V,L,K,FLG> *vc ) {

        fileCounter = 0;
        this->ThreadsNo = ThreadsNo;
        this->VariableCount = VariableCount;
        this->Zero = Zero;
        this->One = One;
        this->Variable = Variable;
        this->UniqueTable = UniqueTable;
        this->queue = queue;
        this->output = output;
        this->vc = vc;
    }


    ///////////////////////////////////////////////////////////////////////////////////////////////////
    // dump data to different formats
    ///////////////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////
    // dump internal data of the package
    ///////////////////////////////////////////////

    // dump all information for data in manager
    void Dump (){
        // create a stream file
        char fName[40];
        fileCounter++;
        sprintf(fName, "Manager Dump %d.txt",fileCounter);	// set file name
        ofstream outClientFile( fName, ios::out );

        int i;
        outClientFile << "----------------------------------------------------------------------------" << endl;
        outClientFile << "----------------------------------------------------------------------------" << endl;
        outClientFile << "			Manager Internal DATA" << endl;
        outClientFile << "No. of Threads : " << ThreadsNo << endl;
        outClientFile << "Variable Count : " << VariableCount << endl;
        //outClientFile << "Variable Bit Count : " << VariableBitCount << endl;
        // dump zero and one nodes
        outClientFile << "----------------------------------------------------------------------------" << endl;
        outClientFile << "zero node " << endl;
        Zero.Dump(outClientFile);
        outClientFile << "----------------------------------------------------------------------------" << endl;
        outClientFile << "One node " << endl;
        One.Dump(outClientFile);
        outClientFile << "----------------------------------------------------------------------------" << endl;
        outClientFile << endl<< endl<< endl<< endl<< endl << endl;
        outClientFile << endl<< endl<< endl<< endl<< endl << endl;
        outClientFile << "----------------------------------------------------------------------------" << endl;
        outClientFile << "Variables" << endl;
        outClientFile << "----------------------------------------------------------------------------" << endl;
        for (i = 0; i < VariableCount ; i++) {
            outClientFile << "Variable No. :" << i << endl;
            Variable[i].Dump(outClientFile);
            outClientFile << "----------------------------------------------------------------------------" << endl;
        }
        outClientFile << endl<< endl<< endl<< endl<< endl << endl;
        outClientFile << "----------------------------------------------------------------------------" << endl;
        outClientFile << "Unique Table" << endl;
        outClientFile << "----------------------------------------------------------------------------" << endl;
        UniqueTable ->Dump(outClientFile);

        queue->Dump(outClientFile);

    }
    //////////////////////////////////////////////////////////////////////////////////////////////
    // dump UNIQUE TABLE to a file for integrity check purposes
    //////////////////////////////////////////////////////////////////////////////////////////////

    // dump hash table (unique table) to a file, for integrity purposes ...
    void DumpHashTable(){
        UniqueTable ->DumpHashTable();
    }

    ///////////////////////////////////////////////
    // dump to a DOT file
    ///////////////////////////////////////////////

    // output the graph to a dot file, it accept a pointer to the graph, and the file name, return true if the file is written, false otherwise
    void OutputDot (BDDPointer <T,V> bp,char *fName ){
        //cout << "------------------------------------------------------------------------------------------" << endl;
        //cout << "------------------------------------------------------------------------------------------" << endl;

        EdgeQueue <T,V>dq;
        ofstream outClientFile( fName, ios::out );	// create file stream
        outClientFile << "digraph G {" << endl;	// header of dot file
        //outClientFile <<"size = \"7.5,10\"" << endl;
        //outClientFile << "center = true;";

        outClientFile << "{f [shape=box, label=\"F\"]}" << endl;	// create the header node
        stringstream **ss;
        stringstream ssEdge(stringstream::in | stringstream::out);

        // create array of string stream
        ss = ( stringstream **) malloc(sizeof(stringstream *) * VariableCount);
        int i;
        bool *varFlags = new bool[VariableCount];	// indicate whether the variable is used or not
        for (i = 0; i < VariableCount; i++ ){
                ss[i] = new stringstream(stringstream::in | stringstream::out);
                (*ss[i]) << "{ rank = same;  \"" << vc->mapVar(i) << "\"" ;
                varFlags[i]= false;
                //cout << ss[i]-> str() << endl;
        }
        //
        if (bp.isEqual(Zero)){	// f = 0
                outClientFile << "{One [shape=box,label=\"1\"]}" << endl;	// create zero node
                outClientFile << "f-> One [headlabel = \"1\", arrowhead = none, style = dotted] " << endl; // create edge from "f" to "zero (inversion of node one)
        }
        else if (bp.isEqual(One)){	// f = 1
                outClientFile << "{one [shape=box,label=\"1\"]}" << endl;	// create zero node
                outClientFile << "f-> one [headlabel = \"1\", arrowhead = none] " << endl; // create edge from "f" to "zero
        }
        else { // if is a function of some variables
            if (isInverted(bp)) {
                bp.ClearInverted();
                outClientFile << "f-> x" << (T)bp.getLocalId() << (T)bp.getHashString() << "[label=\"1\", arrowhead = none, style = dotted]" << endl;	// create edge "f->x1 [label="1"]"	make it a dotted line
            }
            else
                outClientFile << "f-> x" << (T)bp.getLocalId() << (T)bp.getHashString() << "[label=\"1\", arrowhead = none]" << endl;	// create edge "f->x1 [label="1"]"
            outClientFile << "{one [shape=box,label=\"1\"]}" << endl;	// create one node
            recurrsiveDot (bp,ss, dq,varFlags,ssEdge);	// write graph to file
            // finish the file
            outClientFile << "{ node [shape = plaintext]; edge [style = invis];" ;
            bool firstFlag = true;
            stringstream ss1(stringstream::in | stringstream::out);
            // ouput	"4" -> "3" -> "2"
            for (i = VariableCount - 1; i>=0; i--){
                for (i = VariableCount - 1; i>=0; i--){
                    if (varFlags[i]) {	// if the variable is exists
                        if (firstFlag){
                            ss1 << "\"" << vc->mapVar(i) << "\"";
                            firstFlag = false;
                        }
                        else {
                            ss1 << "->" << "\"" << vc->mapVar(i) << "\"";
                        }
                    }
                }
            }
            outClientFile << ss1.str() ;

            outClientFile << "}" << endl;
            // output node in the same level
            for (i = VariableCount - 1; i>=0; i--){
                if (varFlags[i]) {	// if the variable is exists
                    outClientFile << ss[i]->str() << "}" << endl;
                }
            }
            outClientFile << ssEdge.str() << endl;
        }
        outClientFile << "}" << endl;	// end of dot file
        outClientFile.close();		// close file
    }
    void recurrsiveDot(BDDPointer <T,V> bp, stringstream ** & ss, EdgeQueue<T,V> & dq, bool *& varFlags, stringstream & ssEdge) {
        BDDNode <T,V,L> result;
        L level1;
        L level = this->UniqueTable->getLevelFromID(bp);	// get level of the node
        if (level != 0)	{
            *ss[level - 1] << "; x" << (T)bp.getLocalId() << (T)bp.getHashString() << "[label=\"X" << vc->mapVar(level - 1)  << "\"]" << endl;	// create node "x1 [label="X1"]"
            varFlags[level - 1] = true;
        }
        UniqueTable->getInfo (bp, result);	// get corresponding node
        BDDPointer <T,V> t = result.getThen(); // get then pointer
        BDDPointer <T,V> e = result.getElse(); // get else pointer
        // create edge between node and its then node
        if (dq.isExistAndEnq(bp,t)){		// if then have been visited before;
            // just enque it
        }
        else if (t.isEqual(Zero) ) {	// if then is equal zero
            ssEdge << "x" << (T)bp.getLocalId() << (T)bp.getHashString() << "-> one "  << "[headlabel = \"1\", arrowhead = none, style = dotted]" << endl;	// create an edge between node and zero node "x2-> zero [label = "1"]"
        }
        else if (t.isEqual(One)){ 	// if then is equal one
            ssEdge << "x" << (T)bp.getLocalId() << (T)bp.getHashString() << "-> one "  << "[headlabel = \"1\", arrowhead = none]" << endl;	// create an edge between node and one node "x2-> one [label = "1"]"
        }
        else {
            level1 = this->UniqueTable->getLevelFromID(t);	// get level of the then
            if (isInverted(t)) {
                t = getRegularEdge (t);	// get the regular edge
                ssEdge << "x" << (T)bp.getLocalId() << (T)bp.getHashString() << "-> x" << (T)t.getLocalId() << (T)t.getHashString() << "[headlabel = \"1\", arrowhead = none, style = dotted]" << endl;	// create an edge between node and then node "x2-> x1 [label = "1"]" , THIS CASE SHOULD NOT BE CALLED (IF THE ALGORITHM IS VALID), IT IS USED FOR DUBBUGGING PURPOSES
            }
            else
                ssEdge << "x" << (T)bp.getLocalId() << (T)bp.getHashString() << "-> x" << (T)t.getLocalId() << (T)t.getHashString() << "[headlabel = \"1\", arrowhead = none]" << endl;	// create an edge between node and then node "x2-> x1 [label = "1"]"
            recurrsiveDot(t,ss, dq,varFlags,ssEdge);
        }
        // create edge between node and its else node
        if (dq.isExistAndEnq(bp,e)){		// if else have been visited before;
            // just enque it
        }
        else if (e.isEqual(Zero) ) {	// if then is equal zero
            ssEdge << "x" << (T)bp.getLocalId() << (T)bp.getHashString() << "-> one "  << "[headlabel = \"0\", fontcolor = red, color = red, arrowhead = none, style = dotted]" << endl;	// create an edge between node and zero node "x2-> zero [label = "1"]"
        }
        else if (e.isEqual(One)){ 	// if then is equal one
            ssEdge << "x" << (T)bp.getLocalId() << (T)bp.getHashString() << "-> one "  << "[headlabel = \"0\", fontcolor = red, color = red, arrowhead = none]" << endl;	// create an edge between node and one node "x2-> one [label = "1"]"
        }
        else {
            level1 = this->UniqueTable->getLevelFromID(e);	// get level of the then
            if (isInverted(e)) {
                e = getRegularEdge (e);	// get the regular edge
                ssEdge << "x" << (T)bp.getLocalId() << (T)bp.getHashString() << "-> x" << (T)e.getLocalId() << (T)e.getHashString() << "[headlabel = \"0\", fontcolor = red, color = red, arrowhead = none, style = dotted]" << endl;	// create an edge between node and then node "x2-> x1 [label = "1"]", inverted edge
            }
            else
                ssEdge << "x" << (T)bp.getLocalId() << (T)bp.getHashString() << "-> x" << (T)e.getLocalId() << (T)e.getHashString() << "[headlabel = \"0\", fontcolor = red, color = red, arrowhead = none]" << endl;	// create an edge between node and then node "x2-> x1 [label = "1"]"
            recurrsiveDot(e,ss, dq,varFlags,ssEdge);
        }
    }
    void OutputAllOutputToDot (char *fName, circuit *ckt ) {
        this->ckt = ckt;
        int outputNumber = getNumberOfOutput();
        char temp[10];
        int i;

        for (i = 0; i < outputNumber; i++){
            sprintf(temp,"%d",i);
            strcat(fName, temp);
            strcat(fName, ".dot");
            OutputDot(output[i],fName);
        }
    }
};
#endif
