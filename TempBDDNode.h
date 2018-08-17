#ifndef _TEMP_BBD_NODE_CLASS
#define _TEMP_BBD_NODE_CLASS

#include "DepreciatedBDDNode.h"
#include "BDDUtil.h"
// when the node is forwarded, a special value "A" is set in the else pointer. if this node is inverted (after being forwarded), the last bit is inverted.
class TempBDDNode : public DepreciatedBDDNode {
private:

	//DepreciatedBDDPointer forwardedTo;
public:
	// constructor
	TempBDDNode() {
		//forwarded = false;
	}
	// this function check if the node is forwarded or not, if yes, pass the forwarded id to "ForwardTo"
	bool isForwarded (DepreciatedBDDPointer &ForwardedTo) {
		if ((DepreciatedBDDNode::getElse() & 0x7FFFFFFF) == FORWARD_VALUE){	// after removing any possibility the inversion, check if this node is forward
			ForwardedTo = DepreciatedBDDNode::getThenP();
			return true;			
		}	
		else {	
			return false;
		}	
	}
	void invert() {
		DepreciatedBDDNode::setElse (DepreciatedBDDNode::getElse() ^ 0x80000000);	// invert the last bit
	}
	bool isInverted(){
		if ((DepreciatedBDDNode::getElse() & 0x80000000) == 0){
			return false;			
		}	
		else {	
			return true;
		}
	}
	void forwardTo (DepreciatedBDDPointer & To) {
		DepreciatedBDDNode::setElse(FORWARD_VALUE);
		DepreciatedBDDNode::setThen(To);
	}

	void DumpToScreen(){
		cout << "Dump Temp BDD Node" << endl;
		cout << "Forwarded : " <<  DepreciatedBDDNode::getElse() << endl;
		//cout << "pointer to forwarded data " << endl;
		//DepreciatedBDDNode::getThenP()::DumpToScreen();
		DepreciatedBDDNode :: DumpToScreen();
		
		
		
	}
	void Dump(ofstream & ff){
		ff << "Dump Temp BDD Node" << endl;
		ff << "Forwarded : " <<  DepreciatedBDDNode::getElse() << endl;
	//	ff << "pointer to forwarded data " << endl;
	//	DepreciatedBDDNode::getElse().Dump(ff);
		DepreciatedBDDNode :: Dump(ff);
		
		
		
	}
};

#endif 
