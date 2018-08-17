#ifndef _REQUEST_CLASS
#define _REQUEST_CLASS


#include "BDDPointer.h"
#include "BDDNode.h"
#include "RequestKey.h"

template<typename T,typename V, typename L>	// T and V are types of the fields in the BDDPointer, L: is the type of the level (Default: unsigned short) ,
class Request {
private:
    RequestKey <T,V> key;
    BDDNode <T,V,L>  R;
public:
    // constructor
    Request(){}
    Request (BDDPointer <T,V> &  F, BDDPointer <T,V> & G, BDDPointer <T,V> & H, BDDNode <T,V,L>  & R) {
        this->setF(F);
        this->setG(G);
        this->setH(H);
        this->setR(R);
    }

    Request (BDDPointer <T,V> &  F, BDDPointer <T,V> & G, BDDPointer <T,V> & H) {
        this->setF(F);
        this->setG(G);
        this->setH(H);
    }

    // set and get for F
    BDDPointer <T,V> getF(){
        return this->key.getF();
    }
    void setF(BDDPointer <T,V>& F){
        key.setF(F);
    }
    // set and get for G
    BDDPointer <T,V>  getG(){
        return key.getG();
    }
    void setG(BDDPointer <T,V> & G){
        key.setG(G);
    }
    // set and get for h
    BDDPointer <T,V> getH(){
        return key.getH();
    }
    void setH(BDDPointer <T,V> & H){
        key.setH(H);
    }
    // set and get for R
    BDDNode <T,V,L>   getR(){
        return R;
    }
    void setR(BDDNode <T,V,L>  R){
        this->R = R;
    }
    // get key
    RequestKey <T,V>  getKey () {
        return (this->key);
    }
    void setKey (RequestKey <T,V>  key) {
        this->key = key;
    }
    void setLevel(L lev){
        R.setLevel(lev);
    }
    void setThen(BDDPointer <T,V> bp){
        R.setThen(bp);
    }
    void setElse(BDDPointer <T,V> bp){
        R.setElse(bp);
    }
    void clearInverted(){
        R.clearInverted();
    }
    void clearForwarded(){
        R.clearForwarded();
    }
    // compare two nodes
    bool isEqual(Request  <T,V,L> & right){
        RequestKey <T,V> key1 = right.getKey();
        BDDNode <T,V,L> node = right.getR();
        if ( getKey().isEqual(key1) && getR().isEqual(node) ){
            return true;
        }
        return false;
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
