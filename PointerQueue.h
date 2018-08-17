#ifndef _POINTER_QUEUE_CLASS
#define _POINTER_QUEUE_CLASS
// list of bdd pointers
#include "BDDPointer.h"
#include "mutex.h"
#include "BDDUtil.h"

template<typename T,typename V>	// T,V : is the templates for bdd pointer
class PointerQueue {
private:
	volatile int size;		// total no. of node stored*/
	volatile int sliderCounter;	// no. dequed nodes
	Mutex mutex;
	vector < BDDPointer<T,V> > List;
public:
	PointerQueue (){
		reset();
	}
	~PointerQueue (){
	}
	void push(BDDPointer <T,V>  bp){
		mutex.lock();
		List.push_back(bp);
		sliderCounter++;
		mutex.unlock();	
	}
		
	BDDPointer <T,V> getElementFromFront() {
		BDDPointer <T,V> temp;
		mutex.lock();
		temp = List[sliderCounter];
		sliderCounter--;
		mutex.unlock();	
		return temp;
	}	
	void reLoad() {
		sliderCounter = List.size()-1;
	}	
	void remove(BDDPointer <T,V>  bp) {
		mutex.lock();
		int sliderCounter= this->sliderCounter;	;
			
		// go to the desired node
		while(!List[sliderCounter].isEqual(bp)){
			sliderCounter++;
		}
		List.erase(List.begin()+sliderCounter);
		mutex.unlock();		
	}	
	void reset(){
		sliderCounter = 0;		
	}
	void dump(){
	}
};
#endif 
