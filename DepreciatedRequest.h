#ifndef _DEPRECIATED_REQUEST_CLASS
#define _DEPRECIATED_REQUEST_CLASS


#include "DepreciatedBDDPointer.h"
#include "BDDNode.h"
#include "DepreciatedRequestKey.h"
#include "TempBDDNode.h"

class DepreciatedRequest {
private:
	DepreciatedRequestKey key;
	TempBDDNode R;
public:
	// constructor
	DepreciatedRequest(){
	}
	DepreciatedRequest (DepreciatedBDDPointer &  F, DepreciatedBDDPointer & G, DepreciatedBDDPointer & H, TempBDDNode & R) {
		this->setF(F);
		this->setG(G);
		this->setH(H);
		this->setR(R);
	}

	DepreciatedRequest (DepreciatedBDDPointer &  F, DepreciatedBDDPointer & G, DepreciatedBDDPointer & H) {
		this->setF(F);
		this->setG(G);
		this->setH(H);
	}
	// function simulates else, then and level
	int getThen(){
		return key.getThen();
	}
	int getElse(){
		return key.getElse();
	}
	int getLevel(){
		return key.getLevel();
	}
	// get key
	DepreciatedRequestKey  getKey () {
		return (this->key);
	}
	void setKey (DepreciatedRequestKey key) {
		this->key = key;
	}
	// set and get for F
	DepreciatedBDDPointer getF(){
		return this->key.getF();
	}
	void setF(DepreciatedBDDPointer& F){
		key.setF(F);
	}
	// set and get for G
	DepreciatedBDDPointer  getG(){
		return key.getG();
	}
	void setG(DepreciatedBDDPointer & G){
		key.setG(G);
	}
	// set and get for h
	DepreciatedBDDPointer getH(){
		return key.getH();
	}
	void setH(DepreciatedBDDPointer & H){
		key.setH(H);
	}
	// set and get for R
	TempBDDNode  getR(){
		return R;
	}
	void setR(TempBDDNode & R){
		this->R = R;
	}
	// set then , else , id for the node R
	DepreciatedBDDPointer getRThen(){
		return this->R.getThenP();
	}	
	DepreciatedBDDPointer getRElse(){
		return this->R.getElseP();
	}	

	void setThen(DepreciatedBDDPointer Then){
		this->R.setThen(Then);
	}	
	void setElse(DepreciatedBDDPointer Else){
		this->R.setElse(Else);
	}	
	// set and get for id pointer
	DepreciatedBDDPointer getIdP() {
		return R.getIdP();
	}
	void setIdP( DepreciatedBDDPointer & bp){
		R.setIdP(bp);
	}
	// get id
	int getId(){
		return R.getId();
	}
	void DumpToScreen() {
		key.DumpToScreen();
		R.DumpToScreen();
	}
	void Dump(ofstream & ff){
		key.Dump(ff);
		R.Dump(ff);
	}
	
};


#endif 
