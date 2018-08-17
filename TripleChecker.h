// this class is responsible for checking for the terminal case and put the triple in the standard form
#ifndef _TRIPLE_CHECKER_CLASS
#define _TRIPLE_CHECKER_CLASS
#include "BDDPointer.h"
#include "Hash.h"

template<typename T,typename V,typename L,typename K,typename FLG>
// T,V : for the bdd pointers
class TripleChecker {
private:
	BDDPointer <T,V> One;
	BDDPointer <T,V> Zero;
        Hash <T,V,L,K,FLG> * UniqueTable;

	// check if this case is terminal or not; if yes, return results in R
	// F, G, H are inputs, and R is output
	bool isTerminalCase (BDDPointer <T,V> F, BDDPointer <T,V> G, BDDPointer <T,V> H, BDDPointer <T,V>& R) {
		if(F.isEqual(One) ){	//ITE (1,F,G) = F
			R = G;
			return true;
		}
		else if(F.isEqual(Zero)) {	// ITE (0,G,F) = F
			R = H;
			return true;
		}
		else if (G.isEqual(One) && H.isEqual(Zero)) { // ITE (F,1,0) = F
			R = F;
			return true;
		}		
		else if(G.isEqual(H)){	// ITE (F,G,G) = G
			R = G;
			return true;
		}
		else if (G.isEqual(Zero) && H.isEqual(One)) { // ITE(Fn,0,1) = F
                    R = F;
                    R.invert();
                    return true;
		}
		// no matching terminal case
		return false;
	}

public:
	// constructor
	TripleChecker(BDDPointer <T,V> Zero, BDDPointer <T,V> One, Hash <T,V,L,K,FLG> * UniqueTable){
		this->One = One;
		this->Zero = Zero;
                this->UniqueTable = UniqueTable;
	}

	// it check if the requests is terminal case for the terminal case
	bool isTerminalCaseForInvertedEdge  (BDDPointer <T,V> F, BDDPointer <T,V> G, BDDPointer <T,V> H, BDDPointer <T,V> & R) {
		if(isTerminalCase ( F, G, H, R) )	// if this a regular terminal case
			return true;
		else 
			return false;	
	}
	// put the triple in the standard form, and return true if the ouptput is inverted
	bool putRequestInStandardTriple(BDDPointer <T,V> &  F, BDDPointer <T,V> & G, BDDPointer <T,V> & H) {
		// set triples in a simpler form
		BDDPointer <T,V> temp;
		if (F.isEqual(G)){	// ITE(F,F,G) = ITE(F,1,G)
			G = One;
		}
		if (F.isEqual(H)){	// ITE(F,G,F) = ITE(F,G,0)
			H = Zero;
		}
                temp = H;
                temp.invert();
 		if (F.isEqual(temp)){	// ITE(F,G,Fn) = ITE(F,G,1)
			H = One;
		}
                temp =G;
                temp.invert();
		if (F.isEqual(temp)){	// ITE(F,Fn,G) = ITE(F,0,G)
			G = Zero;
		}
		// reorder the rank of the pointers
		L flev = UniqueTable->getLevelFromID(F);
		L glev = UniqueTable->getLevelFromID(G);
		L hlev = UniqueTable->getLevelFromID(H);
		if (G.isEqual(One)) {	// ITE(F,1,G) = ITE(G,1,F)
			if (flev < hlev) {	// swap if F is lower than G
				temp = H;
				H = F;
				F = temp;				
			}
		}
		else if (H.isEqual(Zero)) {	// ITE(F,G,0) = ITE(G,F,0)
			if (flev < glev) {	// swap if F is lower than G
				temp = G;
				G = F;
				F = temp;				
			}
		}
		else if (H.isEqual(One)) {	// ITE(F,G,1) = ITE(Gn,Fn,1)
			if (flev < glev) {	// swap if F is lower than G
                            temp = G;
                            temp.invert();         // Gn
                            G = F;
                            G.invert();            // Fn
                            F = temp;
			}
		}
		else if (G.isEqual(Zero)) {	// ITE(F,0,G) = ITE(Gn,0,Fn)
			if (flev < hlev) {	// swap if F is lower than G
                            temp = H;
                            temp.invert();
                            H = F;
                            H.invert();
                            F = temp;
			}
		}
		else {
                    temp = G;
                    temp.invert();
                    if (H.isEqual(temp)) {              // ITE(F,G,Gn) = ITE(G,F,Fn)
                        if (flev < hlev){
                                temp = F;               // swap F and G
                                F = G;
                                G = temp;
                                H = G;
                                H.invert();        // invert G (now it is F)
                        }
                    }
		}

		// adapt the inverted edges
		bool fedge = F.isInverted();
		bool gedge = G.isInverted();
		//bool hedge = idGen->isInverted(H);
		
		if (!fedge && !gedge)	// F and G are regular, 			ITE(F,G,H)
			return false;
		else if (!fedge && gedge) {	// F is regular and G is inverted, 	 ITE(F,G,H) =  ITE(F,Gn,Hn)n
                    G.invert();
                    H.invert();
                    return true;	// the ouput is inverted
		}		
		else if (fedge && !gedge) {	// F is inverted and G is regular,	ITE(F,G,H) =  ITE(Fn,H,G)
			F.invert();	// invert F
			temp = H;			// swap G and H
			H = G;
			G = temp;
			return false;	// the ouput is regular
		}		
		else if (fedge && gedge) {	// F is inverted and G is inverted, 	 ITE(F,G,H) =  ITE(Fn,Hn,Gn)n
			F.invert();	// invert F
			G.invert();	// invert G
			H.invert();	// invert H
			temp = H;			// swap G and H
			H = G;
			G = temp;			
			return true;	// the ouput is inverted
		}		
		cout << "TripleChecker : you shouldn't be here" << endl;
		return false; 	
	}
};

#endif
