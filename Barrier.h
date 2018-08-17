#ifndef _BARRIER_CLASS
#define _BARRIER_CLASS

#include "BDDUtil.h"
class Barrier {
private:
	pthread_barrier_t barr;
	int num_threads;
public:
	Barrier(){
	}
	Barrier(int num_threads){
		this->num_threads = num_threads;		
		Init();
	}
	void wait() {
		pthread_barrier_wait(&barr);
	}
	void Init(){
		pthread_barrier_init(&barr, NULL, num_threads);
	}
};

#endif 
