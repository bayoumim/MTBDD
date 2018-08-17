#./BFSmain

#g++ -pedantic -Wall -Werror  -c  NodeKey.h
#g++ -pedantic -Wall -Werror  -c  BDDNode.hh
#g++ -pedantic -Wall -Werror  -c  BDDPointer.h
#g++ -pedantic -Wall -Werror  -c  Bucket.h
#g++ -pedantic -Wall -Werror  -c  RWLock.hh
#g++ -pedantic -Wall -Werror  -c  TestThread.h
#g++ -pedantic -Wall -Werror  -c  Segment.h
#g++ -pedantic -Wall -Werror  -c  Hash.hh
#g++ -pedantic -Wall -Werror  -c  Manager.h
#g++ -pedantic -Wall -Werror  -c  Request.h
#g++ -pedantic -Wall -Werror  -c  RequestKey.h
#g++ -pedantic -Wall -Werror  -c  TempBDDNode.h
#g++ -pedantic -Wall -Werror  -c  HashQueue.h
#g++ -pedantic -Wall -Werror  -c  RequestBucket.h
#g++ -pedantic -Wall -Werror  -c  Queue.h
#g++ -pedantic -Wall -Werror  -c  Manager.h
#g++ -pedantic -Wall -Werror  -c  BFSThread.h
#g++ -pedantic -Wall -Werror  -c  condition.h
#g++ -pedantic -Wall -Werror  -c  mutex.h
#g++ -pedantic -Wall -Werror  -c  IDGenerator.h
#g++ -pedantic -Wall -Werror  -c  HashQueue.h
#g++ -pedantic -Wall -Werror  -c  Barrier.h
#g++ -pedantic -Wall -Werror  -c  EdgeQueue.h
#g++ -pedantic -Wall -Werror  -c  OperationSchedular.h
#g++ -pedantic -Wall -Werror  -c  VariableCreator.h
#g++ -pedantic -Wall -Werror  -c  TripleChecker.h
#g++ -pedantic -Wall -Werror  -c  Dumper.h
#g++ -pedantic -Wall -Werror  -c  LookUpTable.h
#g++ -pedantic -Wall -Werror  -c  Condition.h
#g++ -pedantic -Wall -Werror  -c  PointerQueueNode.h
#g++ -pedantic -Wall -Werror  -c  PointerQueue.h
#g++ -pedantic -Wall -Werror  -c  RawTableNode.h
#g++ -pedantic -Wall -Werror  -c  RawTable.h
#g++ -pedantic -Wall -Werror  -c  LookUpTableNode.h
#g++ -pedantic -Wall -Werror  -c  NewToRawLookUpTable.h
#g++ -pedantic -Wall -Werror  -c  IntegrityChecker.h
#g++ -pedantic -Wall -Werror  -c  RawQueueNode.h
#g++ -pedantic -Wall -Werror  -c  RawQueue.h
#g++ -pedantic -Wall -Werror  -c  RawQueueNodeAfterApply.h
#g++ -pedantic -Wall -Werror  -c  RawQueueAfterApply.h
#g++ -pedantic -Wall -Werror  -c  RawQueueNodeAfterReduce.h
#g++ -pedantic -Wall -Werror  -c  RawQueueAfterReduce.h
#g++ -pedantic -Wall -Werror  -c  GCHashNode.h
#g++ -pedantic -Wall -Werror  -c  GCHashBucket.h
#g++ -pedantic -Wall -Werror  -c  GCHash.h
#g++ -pedantic -Wall -Werror  -c GCQueue.h
#g++ -pedantic -Wall -Werror  -c GCThread.h
#g++ -pedantic -Wall -Werror  -c  GCedRawTable.h
#g++ -pedantic -Wall -Werror  -c  GCedRawTableNode.h
#g++ -pedantic -Wall -Werror  -c  InputVectors.h



#install Graphviz
#sudo apt-get install yum

#sudo apt-get install graphviz 

#compile and run file170000c1
#g++ -c -g condition.cpp
#WORKING COMMANDS
./t1 --file=c432 --threadno=100 --hcd=100 --incap=7000 --hmu=90 --hbs=32 --quecap=20000 --qdc=1 --qmu=90 --qbs=32

time ./t1 --file=c432 --threadno=20 --hcd=100 --incap=70000 --hmu=90 --hbs=32 --quecap=20000 --qdc=100 --qmu=90 --qbs=32
time ./t1 --file=c432 --threadno=100 --hcd=10 --incap=70000 --hmu=90 --hbs=32 --quecap=20000 --qdc=100 --qmu=90 --qbs=32
time ./t1 --file=c880 --threadno=1 --hcd=10 --incap=5000000 --hmu=90 --hbs=32 --quecap=500000 --qdc=100 --qmu=90 --qbs=32 -c
time ./t1 --file=c432 --threadno=1 --hcd=10 --incap=70000 --hmu=90 --hbs=32 --quecap=20000 --qdc=100 --qmu=90 --qbs=32 -c
time ./t1 --file=c432 --threadno=20 --hcd=10 --incap=70000 --hmu=90 --hbs=32 --quecap=20000 --qdc=100 --qmu=90 --qbs=32 -c
time ./t1 --file=c432 --threadno=100 --hcd=10 --incap=70000 --hmu=90 --hbs=32 --quecap=20000 --qdc=100 --qmu=90 --qbs=32 -c
time ./t1 --file=c880 --threadno=10 --hcd=10 --incap=5000000 --hmu=90 --hbs=32 --quecap=500000 --qdc=100 --qmu=90 --qbs=32 -c

time ./t1 --file=c432 --threadno=20 --hcd=100 --incap=70000 --hmu=90 --hbs=32 --quecap=20000 --qdc=100 --qmu=90 --qbs=32
time ./t1 --file=c432 --threadno=10 --hcd=100 --incap=40000 --hmu=90 --hbs=32 --quecap=20000 --qdc=100 --qmu=90 --qbs=32 

time ./t1 --file=c880 --threadno=1 --hcd=10 --incap=5000000 --hmu=90 --hbs=32 --quecap=500000 --qdc=100 --qmu=90 --qbs=32 -c
time ./t1 --file=c7552 --threadno=2 --hcd=10 --incap=5000000 --hmu=90 --hbs=32 --quecap=10000000 --qdc=100 --qmu=90 --qbs=32


#===================================================================================================================================================================
# Multi-threaded debugging
#===================================================================================================================================================================

valgrind --tool=drd --trace-mutex=yes --trace-rwlock=yes --report-signal-unlocked=yes --first-race-only=yes ./t1 --file=c432 --threadno=20 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=20000 --qdc=100 --qmu=90 --qbs=32 >& drd.txt

valgrind --tool=drd  --report-signal-unlocked=yes --first-race-only=yes ./t1 --file=c432 --threadno=20 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=20000 --qdc=100 --qmu=90 --qbs=32 >& drd.txt

valgrind --tool=drd  --report-signal-unlocked=yes --first-race-only=yes  ./t1 --file=c432 --threadno=100 --hcd=500 --incap=7000 --hmu=90 --hbs=32 --quecap=20000 --qdc=100 --qmu=90 --qbs=32  >& drd.txt


valgrind --tool=drd  --report-signal-unlocked=yes --first-race-only=yes ./t1 --file=c432 --threadno=100 --hcd=1000 --incap=7000 --hmu=90 --hbs=32 --quecap=20000 --qdc=100 --qmu=90 --qbs=32 >& drd.txt
 valgrind --tool=drd  --report-signal-unlocked=yes --first-race-only=yes ./t1 --file=c432 --threadno=2 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=20000 --qdc=1 --qmu=90 --qbs=32 >& drd1.txt

valgrind --tool=drd  --report-signal-unlocked=yes --first-race-only=yes ./t1 --file=c432 --threadno=2 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=20000 --qdc=1 --qmu=90 --qbs=32 >& drd.txt


valgrind --tool=drd  --report-signal-unlocked=yes --first-race-only=yes ./t1 --file=c432 --threadno=2 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=20000 --qdc=1 --qmu=90 --qbs=32 >& drd.txt

valgrind --tool=drd  --report-signal-unlocked=yes --first-race-only=yes ../t1 --file=c432 --threadno=10 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=20000 --qdc=1 --qmu=90 --qbs=32 >& drd.txt

valgrind --tool=drd  --report-signal-unlocked=yes --first-race-only=yes ../t1 --file=c432 --threadno=10 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=20000 --qdc=1 --qmu=90 --qbs=32 >& drd.txt

valgrind --tool=helgrind  --read-var-info=yes ./t1 --file=c432 --threadno=20 --hcd=100 --incap=7000 --hmu=90 --hbs=32 --quecap=20000 --qdc=1 --qmu=90 --qbs=32 >& helgrind.txt

valgrind --tool=drd  --trace-rwlock=yes ./t1 --file=c432 --threadno=20 --hcd=100 --incap=7000 --hmu=90 --hbs=32 --quecap=20000 --qdc=1 --qmu=90 --qbs=32 >& drd1.txt

valgrind --tool=drd  --report-signal-unlocked=yes --first-race-only=yes  ./t1 --file=c1355 --threadno=2 --hcd=100 --incap=500000 --hmu=90 --hbs=32 --quecap=50000 --qdc=100 --qmu=90 --qbs=32 --hic=0 >& output

valgrind --tool=drd  --report-signal-unlocked=yes --first-race-only=yes  ./t1 --file=c432 --threadno=1 --hcd=10 --incap=50000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=1 --GCthreadsNo=2>& output

valgrind --tool=drd  --report-signal-unlocked=yes --first-race-only=yes ./t1 --file=c432 --threadno=1 --hcd=10 --incap=50000 --hmu=90 --hbs=32 --quecap=2000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=2 --GCthreadsNo=1>& output 

valgrind --tool=drd  --report-signal-unlocked=yes --first-race-only=yes ./t1 --file=c432 --threadno=1 --hcd=10 --incap=50000 --hmu=90 --hbs=32 --quecap=2000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=2 --GCthreadsNo=2>& output

valgrind --tool=drd  --report-signal-unlocked=yes --first-race-only=yes ./t1 --file=c432 --threadno=1 --hcd=10 --incap=50000 --hmu=90 --hbs=32 --quecap=2000 --qdc=50 --qmu=90 --qbs=32 --hic=0 --utgc=2 --GCthreadsNo=2>& output

valgrind --tool=drd  --report-signal-unlocked=yes --first-race-only=yes ./t1 --file=c432 --threadno=2 --hcd=10 --incap=50000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=0 --GCthreadsNo=2>& output 

valgrind --tool=helgrind   --read-var-info=yes ./t1 --file=c432 --threadno=2 --hcd=10 --incap=50000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=0 --GCthreadsNo=2>& output 

#===================================================================================================================================================================
# memcheck
#===================================================================================================================================================================
valgrind --tool=memcheck --track-origins=yes ./t1 --file=c432 --threadno=1 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=6 >& output 

valgrind --tool=memcheck --track-origins=yes ./t1 --file=c432 --threadno=1 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=8 >& output

valgrind --tool=memcheck --track-origins=yes ./t1 --file=c1355 --threadno=2 --hcd=100 --incap=500000 --hmu=90 --hbs=32 --quecap=50000 --qdc=100 --qmu=90 --qbs=32 --hic=0 >& output

valgrind --tool=memcheck --track-origins=yes ./t1 --file=c432 --threadno=1 --hcd=10 --incap=50000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=1 --GCthreadsNo=1>& output 

valgrind --tool=memcheck --track-origins=yes ./t1 --file=c432 --threadno=1 --hcd=10 --incap=50000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=1 --GCthreadsNo=2>& output1 



#===================================================================================================================================================================
# Profiling
#===================================================================================================================================================================
valgrind --tool=callgrind ./t1 --file=c432 --threadno=1 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=0 >& output 

valgrind --tool=callgrind ./t1  --file=c432 --threadno=1 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=2000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=1 --GCthreadsNo=1 >& output 


valgrind --tool=callgrind ./t1 --file=c432 --threadno=2 --hcd=100 --incap=25000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=1 --GCthreadsNo=1>& output 

 

cd $HOME
g++ -D_REENTRANT -c -O0 -g thread.cpp
g++ -D_REENTRANT -c -O0 -g mutex.cpp
g++ -g -D_REENTRANT -pedantic -Wall -Werror -O0 -c circuit.cc
g++ -D_REENTRANT -c -pedantic -Wall -Werror -O0 -g test.cc
g++ -pedantic -Wall -Werror -O0 -D_REENTRANT -o t1 -g mutex.o  thread.o test.o circuit.o -lm -lpthread
time ./t1 --file=c17 --threadno=2 --hcd=10 --incap=50000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=0 --GCthreadsNo=1 --vr=1>& output


#===================================================================================================================================================================
# Variable Reordering
#===================================================================================================================================================================
# weighted heuristics
time ./t1 --file=c17 --threadno=2 --hcd=10 --incap=50000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=0 --GCthreadsNo=1 --vr=1>& output

#===================================================================================================================================================================
# hybrid approach
#===================================================================================================================================================================
time ./t1 --file=c432 --threadno=2 --hcd=10 --incap=50000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=0 --GCthreadsNo=1>& output
time ./t1 --file=c880 --threadno=2 --hcd=10 --incap=50000 --hmu=90 --hbs=32 --quecap=50000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=0 --GCthreadsNo=1>& output 
time ./t1 --file=c1355 --threadno=1 --hcd=10 --incap=2000000 --hmu=90 --hbs=32 --quecap=50000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=0 --GCthreadsNo=2>& output 
time ./t1 --file=c5315 --threadno=1 --hcd=10 --incap=5000000 --hmu=90 --hbs=32 --quecap=50000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=1 --GCthreadsNo=2>& output 


#===================================================================================================================================================================
# node validation
#===================================================================================================================================================================
time ./t1 --file=c17 --threadno=2 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=2000 --qdc=50 --qmu=90 --qbs=32 --hic=12 --utgc=0 --GCthreadsNo=1
time ./t1 --file=c432 --threadno=2 --hcd=10 --incap=15000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=12 --utgc=0 --GCthreadsNo=2
time ./t1 --file=c499 --threadno=2 --hcd=10 --incap=500000 --hmu=90 --hbs=32 --quecap=500000 --qdc=100 --qmu=90 --qbs=32 --hic=12 --utgc=0 --GCthreadsNo=2 
time ./t1 --file=c880 --threadno=1 --hcd=100000 --incap=5000000 --hmu=90 --hbs=32 --quecap=5000000 --qdc=100000 --qmu=90 --qbs=32 --hic=12 --utgc=0 --GCthreadsNo=1
time ./t1 --file=c1355 --threadno=1 --hcd=10 --incap=2000000 --hmu=90 --hbs=32 --quecap=5000000 --qdc=100 --qmu=90 --qbs=32 --hic=12 --utgc=0 --GCthreadsNo=2
time ./t1 --file=c1908 --threadno=1 --hcd=10 --incap=5000000 --hmu=90 --hbs=32 --quecap=50000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=1 --GCthreadsNo=2>& output 

#===================================================================================================================================================================
# simulation validation
#===================================================================================================================================================================

time ./t1 --file=c499 --threadno=2 --hcd=10 --incap=500000 --hmu=90 --hbs=32 --quecap=500000 --qdc=100 --qmu=90 --qbs=32 --hic=11 --utgc=0 --GCthreadsNo=2>& output 
time ./t1 --file=c17 --threadno=2 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=2000 --qdc=50 --qmu=90 --qbs=32 --hic=11 --utgc=0 --GCthreadsNo=1
time ./t1 --file=c432 --threadno=2 --hcd=10 --incap=15000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=11 --utgc=0 --GCthreadsNo=2

#===================================================================================================================================================================
# computed table	(very amazing result with computed table)
#===================================================================================================================================================================
time ./t1 --file=c17 --threadno=2 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=7000 --qdc=50 --qmu=90 --qbs=32 --hic=0 --utgc=0 --GCthreadsNo=1 -c
time ./t1 --file=c432 --threadno=2 --hcd=10 --incap=15000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=0 --GCthreadsNo=2 -c
time ./t1 --file=c432 --threadno=2 --hcd=10 --incap=15000 --hmu=90 --hbs=32 --quecap=2000 --qdc=50 --qmu=90 --qbs=32 --hic=0 --utgc=0 --GCthreadsNo=2 -c&>output
time ./t1 --file=c880 --threadno=1 --hcd=100000 --incap=5000000 --hmu=90 --hbs=32 --quecap=5000000 --qdc=100000 --qmu=90 --qbs=32 --hic=0 --utgc=0 --GCthreadsNo=1 -c>& output 
time ./t1 --file=c880 --threadno=2 --hcd=10 --incap=50000 --hmu=90 --hbs=32 --quecap=50000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=0 --GCthreadsNo=1 -c>& output 
time ./t1 --file=c1355 --threadno=2 --hcd=10 --incap=2000000 --hmu=90 --hbs=32 --quecap=50000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=0 --GCthreadsNo=2 -c>& output 
time ./t1 --file=c5315 --threadno=2 --hcd=10 --incap=5000000 --hmu=90 --hbs=32 --quecap=50000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=0 --GCthreadsNo=2 -c>& output 
time ./t1 --file=c5315 --threadno=2 --hcd=10 --incap=5000000 --hmu=90 --hbs=32 --quecap=5000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=0 --GCthreadsNo=2 -c>& output 





time ./t1 --file=c2670 --threadno=1 --hcd=100 --incap=500000 --hmu=90 --hbs=32 --quecap=500000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=2 --GCthreadsNo=2>& output 


time ./t1 --file=c432 --threadno=2 --hcd=10 --incap=500 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=0 --GCthreadsNo=2>& output 


time ./t1 --file=c880 --threadno=1 --hcd=100000 --incap=5000000 --hmu=90 --hbs=32 --quecap=5000000 --qdc=100000 --qmu=90 --qbs=32 --hic=0 --utgc=1 --GCthreadsNo=1>& output 


time ./t1 --file=c432 --threadno=2 --hcd=1000 --incap=50000 --hmu=90 --hbs=32 --quecap=50000 --qdc=1000 --qmu=90 --qbs=32 --hic=0 --utgc=0 --GCthreadsNo=2>& output 

time ./t1 --file=c432 --threadno=2 --hcd=10 --incap=50000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=0 --GCthreadsNo=2>& output 



time ./t1 --file=c432 --threadno=1 --hcd=10 --incap=50000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=2 --GCthreadsNo=2>& output 


time ./t1 --file=c432 --threadno=1 --hcd=10 --incap=5000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=2 --GCthreadsNo=1>& output 

time ./t1 --file=c432 --threadno=1 --hcd=10 --incap=5000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=2 --GCthreadsNo=2>& output 

time ./t1 --file=c1355 --threadno=1 --hcd=10 --incap=2000000 --hmu=90 --hbs=32 --quecap=50000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=0 --GCthreadsNo=2>& output 

time ./t1 --file=c880 --threadno=1 --hcd=10 --incap=500000 --hmu=90 --hbs=32 --quecap=50000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=2 --GCthreadsNo=1>& output

time ./t1 --file=c432 --threadno=1 --hcd=10 --incap=50000 --hmu=90 --hbs=32 --quecap=2000 --qdc=50 --qmu=90 --qbs=32 --hic=0 --utgc=2 --GCthreadsNo=2>& output


time ./t1 --file=c880 --threadno=1 --hcd=10 --incap=50000 --hmu=90 --hbs=32 --quecap=50000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=2 --GCthreadsNo=1>& output

time ./t1 --file=c880 --threadno=1 --hcd=10 --incap=5000000 --hmu=90 --hbs=32 --quecap=5000000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=2 --GCthreadsNo=1>& output 


time ./t1 --file=c432 --threadno=1 --hcd=10 --incap=50000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=2 --GCthreadsNo=2>& output 

time ./t1 --file=c432 --threadno=1 --hcd=10 --incap=50000 --hmu=90 --hbs=32 --quecap=2000 --qdc=50 --qmu=90 --qbs=32 --hic=0 --utgc=2 --GCthreadsNo=2>& output

time ./t1 --file=c432 --threadno=1 --hcd=10 --incap=50000 --hmu=90 --hbs=32 --quecap=2000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=2 --GCthreadsNo=1>& output 

time ./t1 --file=c432 --threadno=1 --hcd=10 --incap=50000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=2 --GCthreadsNo=1>& output 

time ./t1 --file=c432 --threadno=1 --hcd=10 --incap=50000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=1 --utgc=0 --GCthreadsNo=1>& output2

time ./t1 --file=c432 --threadno=1 --hcd=10 --incap=50000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=2 --GCthreadsNo=1>& output 

time ./t1 --file=c880 --threadno=1 --hcd=10 --incap=50000 --hmu=90 --hbs=32 --quecap=50000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=1 --GCthreadsNo=1>& output

 time ./t1 --file=c432 --threadno=20 --hcd=10 --incap=20000 --hmu=90 --hbs=32 --quecap=2000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=1 --GCthreadsNo=20>& output 

time ./t1 --file=c432 --threadno=1 --hcd=10 --incap=50000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=1 --GCthreadsNo=2>& output2 

time ./t1 --file=c5315 --threadno=1 --hcd=10 --incap=5000000 --hmu=90 --hbs=32 --quecap=50000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=1 --GCthreadsNo=2>& output 

time ./t1 --file=c1355 --threadno=1 --hcd=10 --incap=50000 --hmu=90 --hbs=32 --quecap=50000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=0 --GCthreadsNo=2>& output 

time ./t1 --file=c880 --threadno=1 --hcd=10 --incap=50000 --hmu=90 --hbs=32 --quecap=50000 --qdc=100 --qmu=90 --qbs=32 --hic=10 --utgc=1 --GCthreadsNo=1>& output2

time ./t1 --file=c880 --threadno=1 --hcd=10 --incap=500000 --hmu=90 --hbs=32 --quecap=5000000 --qdc=100 --qmu=90 --qbs=32 --hic=9 --utgc=1 --GCthreadsNo=1>& output 

time ./t1 --file=c880 --threadno=1 --hcd=10 --incap=500000 --hmu=90 --hbs=32 --quecap=5000000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=1 --GCthreadsNo=1>& output 

time ./t1 --file=c880 --threadno=1 --hcd=10 --incap=50000 --hmu=90 --hbs=32 --quecap=50000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=1 --GCthreadsNo=1>& output2


time ./t1 --file=c432 --threadno=10 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=2000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=1 --GCthreadsNo=1>& output 

time ./t1 --file=c432 --threadno=1 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=2000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=0 --GCthreadsNo=0>& output 


time ./t1 --file=c880 --threadno=1 --hcd=10 --incap=50000 --hmu=90 --hbs=32 --quecap=50000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=0 --GCthreadsNo=1>& output 


time ./t1 --file=c880 --threadno=1 --hcd=10 --incap=50000 --hmu=90 --hbs=32 --quecap=50000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=1 --GCthreadsNo=1>& output 

time ./t1 --file=c432 --threadno=1 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=2000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=1 --GCthreadsNo=1>& output 

time ./t1 --file=c1355 --threadno=1 --hcd=10 --incap=50000 --hmu=90 --hbs=32 --quecap=5000000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=1 --GCthreadsNo=2>& output 

time ./t1 --file=c880 --threadno=1 --hcd=10 --incap=50000 --hmu=90 --hbs=32 --quecap=5000000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=1 --GCthreadsNo=1>& output 


time ./t1 --file=c432 --threadno=1 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=1 --GCthreadsNo=1>& output 


time ./t1 --file=c880 --threadno=10 --hcd=10 --incap=5000000 --hmu=90 --hbs=32 --quecap=5000000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=1 --GCthreadsNo=10>& output 

time ./t1 --file=c1355 --threadno=1 --hcd=10 --incap=5000000 --hmu=90 --hbs=32 --quecap=5000000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=1 --GCthreadsNo=2>& output 

time ./t1 --file=c880 --threadno=1 --hcd=10 --incap=5000000 --hmu=90 --hbs=32 --quecap=5000000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=1 --GCthreadsNo=2>& output 
time ./t1 --file=c880 --threadno=1 --hcd=10 --incap=5000000 --hmu=90 --hbs=32 --quecap=5000000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=1 --GCthreadsNo=1>& output 

time ./t1 --file=c432 --threadno=1 --hcd=10 --incap=50000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=1 --GCthreadsNo=2>& output 

time ./t1 --file=c1355 --threadno=1 --hcd=10 --incap=5000000 --hmu=90 --hbs=32 --quecap=5000000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=1 --GCthreadsNo=1>& output 

time ./t1 --file=c880 --threadno=1 --hcd=10 --incap=5000000 --hmu=90 --hbs=32 --quecap=5000000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=1 --GCthreadsNo=1>& output 

time ./t1 --file=c432 --threadno=1 --hcd=10 --incap=50000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=1 --GCthreadsNo=1>& output 





time ./t1 --file=c17 --threadno=1 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=1 --GCthreadsNo=1>& output 

time ./t1 --file=c17 --threadno=1 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=0 --GCthreadsNo=1>& output 


time ./t1 --file=c432 --threadno=1 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=0 --utgc=0 >& output 



time ./t1 --file=c3540 --threadno=1 --hcd=100 --incap=500000 --hmu=90 --hbs=32 --quecap=5000 --qdc=10 --qmu=90 --qbs=32 --hic=0 >& output35401 


time ./t1 --file=c3540 --threadno=1 --hcd=100 --incap=500000 --hmu=90 --hbs=32 --quecap=500000 --qdc=100 --qmu=90 --qbs=32 --hic=0 >& output35401 

time ./t1 --file=c3540 --threadno=2 --hcd=100 --incap=500000 --hmu=90 --hbs=32 --quecap=500000 --qdc=100 --qmu=90 --qbs=32 --hic=0 >& output35402 



time ./t1 --file=c2670 --threadno=1 --hcd=100 --incap=500000 --hmu=90 --hbs=32 --quecap=500000 --qdc=100 --qmu=90 --qbs=32 --hic=0 >& output26701 

time ./t1 --file=c2670 --threadno=2 --hcd=100 --incap=500000 --hmu=90 --hbs=32 --quecap=500000 --qdc=100 --qmu=90 --qbs=32 --hic=0 >& output26702 


time ./t1 --file=c1355 --threadno=1 --hcd=100 --incap=500000 --hmu=90 --hbs=32 --quecap=5000000 --qdc=100 --qmu=90 --qbs=32 --hic=0 >& output13551 
time ./t1 --file=c1355 --threadno=2 --hcd=100 --incap=500000 --hmu=90 --hbs=32 --quecap=50000 --qdc=100 --qmu=90 --qbs=32 --hic=0 >& output13552 


time ./t1 --file=c1355 --threadno=2 --hcd=100 --incap=7000 --hmu=90 --hbs=32 --quecap=50000 --qdc=100 --qmu=90 --qbs=32 --hic=0 >& output1355 

time ./t1 --file=c880 --threadno=2 --hcd=100 --incap=7000 --hmu=90 --hbs=32 --quecap=50000 --qdc=100 --qmu=90 --qbs=32 --hic=0 >& output880 

time ./t1 --file=c432 --threadno=2 --hcd=100 --incap=50000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=0 >& output432 

time ./t1 --file=c432 --threadno=2 --hcd=100 --incap=7000 --hmu=90 --hbs=32 --quecap=2000 --qdc=50 --qmu=90 --qbs=32 --hic=0 >& output 


time ./t1 --file=c432 --threadno=1 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=0 >& output 

time ./t1 --file=c432 --threadno=1 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=8 >& output 

time ./t1 --file=c432 --threadno=1 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=7 >& output 


time ./t1 --file=c432 --threadno=1 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=2000 --qdc=100 --qmu=90 --qbs=32 --hic=6 >& output 

time ./t1 --file=c432 --threadno=1 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=6 >& output 

 valgrind --tool=memcheck --leak-check=yes --read-var-info=yes --track-origins=yes ./t1 --file=c432 --threadno=1 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=5 >& output

 valgrind --tool=memcheck --leak-check=yes --read-var-info=yes --track-origins=yes ./t1 --file=c432 --threadno=1 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=6 >& output

valgrind --tool=memcheck --read-var-info=yes --track-origins=yes ./t1 --file=c432 --threadno=1 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=0 >& output 


valgrind --tool=memcheck --track-origins=yes ./t1 --file=c432 --threadno=1 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=6 >& output 


time ./t1 --file=c432 --threadno=1 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=0 >& output

time ./t1 --file=c432 --threadno=1 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=6 >& output 

time ./t1 --file=c432 --threadno=1 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=6 >& output 

time ./t1 --file=c432 --threadno=1 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=5 >& output


time ./t1 --file=c432 --threadno=1 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=3 >& output


time ./t1 --file=c432 --threadno=1 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=4 >& output


time ./t1 --file=c432 --threadno=1 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=3 >& output


time ./t1 --file=c432 --threadno=1 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=2000 --qdc=100 --qmu=90 --qbs=32 --hic=2 >& output

time ./t1 --file=c432 --threadno=1 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=7000 --qdc=100 --qmu=90 --qbs=32 --hic=1 >& output

time ./t1 --file=c432 --threadno=1 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=2000 --qdc=100 --qmu=90 --qbs=32 --hic=1 >& output

time ./t1 --file=c432 --threadno=1 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=2000 --qdc=100 --qmu=90 --qbs=32 >& output.txt

g++ -D_REENTRANT -c -pedantic -Wall -Werror -Weffc++ -fsyntax-only -Wformat -O0 -g test.cc >& c.txt

g++ -g -D_REENTRANT -pedantic -Wall -Werror -Wextra  -O0 -c test.cc


time ./t1 --file=c432 --threadno=1 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=20000 --qdc=100 --qmu=90 --qbs=32
time ./t1 --file=c432 --threadno=2 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=20000 --qdc=100 --qmu=90 --qbs=32

time ./t1 --file=c880 --threadno=1 --hcd=10 --incap=50000 --hmu=90 --hbs=32 --quecap=500000 --qdc=100 --qmu=90 --qbs=32

time ./t1 --file=c1355 --threadno=1 --hcd=10 --incap=50000 --hmu=90 --hbs=32 --quecap=500000 --qdc=100 --qmu=90 --qbs=32 
time ./t1 --file=c432 --threadno=20 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=20000 --qdc=100 --qmu=90 --qbs=32

time ./t1 --file=c880 --threadno=2 --hcd=10 --incap=50000 --hmu=90 --hbs=32 --quecap=500000 --qdc=100 --qmu=90 --qbs=32

time ./t1 --file=c1355 --threadno=1 --hcd=10 --incap=50000 --hmu=90 --hbs=32 --quecap=500000 --qdc=100 --qmu=90 --qbs=32 

time ./t1 --file=c880 --threadno=1 --hcd=10 --incap=50000 --hmu=90 --hbs=32 --quecap=500000 --qdc=100 --qmu=90 --qbs=32 
time ./t1 --file=c432 --threadno=2 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=20000 --qdc=100 --qmu=90 --qbs=32
time ./t1 --file=c432 --threadno=1 --hcd=10 --incap=7000 --hmu=90 --hbs=32 --quecap=20000 --qdc=100 --qmu=90 --qbs=32

time ./t1 --file=c432 --threadno=1 --hcd=10 --incap=70000 --hmu=90 --hbs=32 --quecap=20000 --qdc=100 --qmu=90 --qbs=32 -c

time ./t1 --file=c880 --threadno=2 --hcd=10 --incap=5000000 --hmu=90 --hbs=32 --quecap=500000 --qdc=300000 --qmu=90 --qbs=32

time ./t1 --file=c880 --threadno=2 --hcd=10 --incap=5000000 --hmu=90 --hbs=32 --quecap=500000 --qdc=100000 --qmu=90 --qbs=32

time ./t1 --file=c880 --threadno=1 --hcd=10 --incap=5000000 --hmu=90 --hbs=32 --quecap=500000 --qdc=100000 --qmu=90 --qbs=32

time ./t1 --file=c880 --threadno=1 --hcd=10 --incap=5000000 --hmu=90 --hbs=32 --quecap=500000 --qdc=100 --qmu=90 --qbs=32

time ./t1 --file=c432 --threadno=20 --hcd=10 --incap=70000 --hmu=90 --hbs=32 --quecap=20000 --qdc=100 --qmu=90 --qbs=32

time ./t1 --file=c432 --threadno=1 --hcd=10 --incap=70000 --hmu=90 --hbs=32 --quecap=20000 --qdc=100 --qmu=90 --qbs=32

time ./t1 --file=c880 --threadno=1 --hcd=10 --incap=5000000 --hmu=90 --hbs=32 --quecap=500000 --qdc=10 --qmu=90 --qbs=32

 time ./t1 --file=c432 --threadno=2 --hcd=3 --incap=70000 --hmu=90 --hbs=32  --quecap=20000 --qdc=3 --qmu=90 --qbs=32
 time ./t1 --file=c432 --threadno=20 --hcd=3 --incap=70000 --hmu=90 --hbs=32  --quecap=20000 --qdc=3 --qmu=90 --qbs=32

time ./t1 --file=c432 --threadno=2 --hcd=3 --incap=70000 --hmu=90 --hbs=32  --quecap=20000 --qdc=3 --qmu=90 --qbs=32

time ./t1 --file=c432 --threadno=1 --hcd=3 --incap=70000 --hmu=90 --hbs=32  --quecap=20000 --qdc=3 --qmu=90 --qbs=32 -c

 time ./t1 --file=c880 --threadno=1 --hcd=3 --incap=5000000 --hmu=90 --hbs=32  --quecap=500000 --qdc=3 --qmu=90 --qbs=32
 time ./t1 --file=c880 --threadno=10 --hcd=3 --incap=5000000 --hmu=90 --hbs=32  --quecap=500000 --qdc=3 --qmu=90 --qbs=32
time ./t1 --file=c880 --threadno=2 --hcd=3 --incap=5000000 --hmu=90 --hbs=32  --quecap=500000 --qdc=3 --qmu=90 --qbs=32

./t1 --file=c17 --threadno=2 --hcd=3 --incap=100 --hmu=90 --hbs=32 -c --quecap=200 --qdc=3 --qmu=90 --qbs=32
time ./t1 --file=c432 --threadno=2 --hcd=3 --incap=70000 --hmu=90 --hbs=32 --quecap=700000 --qdc=3 --qmu=90 --qbs=32
./t1 c432
 time ./t1 --file=c432 --threadno=1 --hcd=3 --incap=70000 --hmu=90 --hbs=32  --quecap=20000 --qdc=3 --qmu=90 --qbs=32
time ./t1 --file=c17 --threadno=2 --hcd=3 --incap=10 --hmu=90 --hbs=32  --quecap=10 --qdc=3 --qmu=90 --qbs=32
 time ./t1 --file=c880 --threadno=1 --hcd=3 --incap=5000000 --hmu=90 --hbs=32  --quecap=10000000 --qdc=3 --qmu=90 --qbs=32

time ./t1 --file=c432 --threadno=1 --hcd=100 --incap=70000 --hmu=90 --hbs=32 --quecap=20000 --qdc=100 --qmu=90 --qbs=32 

g++ -c -ggdb thread.cpp
g++ -c -ggdb test.cc
g++ -c -ggdb mutex.cpp
g++ -ggdb -c circuit.cc
g++ -pedantic -Wall -o t1 -ggdb mutex.o  thread.o test.o circuit.o -lm -lpthread

g++ -c -g -O3 thread.cpp
g++ -c -g -O3 test.cc
g++ -c -g -O3 mutex.cpp
g++ -g -c circuit.cc
g++ -pedantic -Wall -o t1 -g -O3 mutex.o  thread.o test.o circuit.o -lm -lpthread

g++ -c -pg thread.cpp
g++ -c -pg test.cc
g++ -c -pg mutex.cpp
g++ -pg -c circuit.cc
g++ -pedantic -Wall -o t1 -pg mutex.o  thread.o test.o circuit.o -lm -lpthread


#g++ -o t1 -g mutex.o condition.o thread.o test.o -lm -lpthread

#./t1
#./t1 c17
#./t1 c880


dot -Tgif graph1.dot -o graph1.gif
dot -Tgif graph2.dot -o graph2.gif
dot -Tgif graph3.dot -o graph3.gif
dot -Tgif graph4.dot -o graph4.gif
dot -Tgif graph5.dot -o graph5.gif
dot -Tgif graph6.dot -o graph6.gif
dot -Tgif graph7.dot -o graph7.gif
dot -Tgif graph8.dot -o graph8.gif
dot -Tgif graph9.dot -o graph9.gif
dot -Tgif graph10.dot -o graph10.gif
dot -Tgif graph11.dot -o graph11.gif
dot -Tgif graph12.dot -o graph12.gif
dot -Tgif graph13.dot -o graph13.gif
dot -Tgif graph14.dot -o graph14.gif
dot -Tgif graph15.dot -o graph15.gif
dot -Tgif graph16.dot -o graph16.gif
dot -Tgif graph17.dot -o graph17.gif
dot -Tgif graph18.dot -o graph18.gif
dot -Tgif graph19.dot -o graph19.gif
dot -Tgif graph20.dot -o graph20.gif
dot -Tgif graph21.dot -o graph21.gif
dot -Tgif graph22.dot -o graph22.gif
dot -Tgif graph23.dot -o graph23.gif
dot -Tgif graph24.dot -o graph24.gif
dot -Tgif graph25.dot -o graph25.gif
dot -Tgif graph26.dot -o graph26.gif
dot -Tgif graph27.dot -o graph27.gif
dot -Tgif graph28.dot -o graph28.gif
dot -Tgif graph29.dot -o graph29.gif
dot -Tgif graph30.dot -o graph30.gif
dot -Tgif graph31.dot -o graph31.gif
dot -Tgif graph32.dot -o graph32.gif
dot -Tgif graph33.dot -o graph33.gif
dot -Tgif graph34.dot -o graph34.gif
dot -Tgif graph35.dot -o graph35.gif
dot -Tgif graph36.dot -o graph36.gif
dot -Tgif graph37.dot -o graph37.gif
dot -Tgif graph38.dot -o graph38.gif
dot -Tgif graph39.dot -o graph39.gif
dot -Tgif graph40.dot -o graph40.gif
dot -Tgif graph41.dot -o graph41.gif
dot -Tgif graph42.dot -o graph42.gif
dot -Tgif graph43.dot -o graph43.gif
dot -Tgif graph44.dot -o graph44.gif
dot -Tgif graph45.dot -o graph45.gif
dot -Tgif graph46.dot -o graph46.gif
dot -Tgif graph47.dot -o graph47.gif
dot -Tgif graph48.dot -o graph48.gif
dot -Tgif graph49.dot -o graph49.gif
dot -Tgif graph50.dot -o graph50.gif
dot -Tgif graph51.dot -o graph51.gif
dot -Tgif graph52.dot -o graph52.gif
dot -Tgif graph53.dot -o graph53.gif
dot -Tgif graph54.dot -o graph54.gif
dot -Tgif graph55.dot -o graph55.gif
dot -Tgif graph56.dot -o graph56.gif
dot -Tgif graph57.dot -o graph57.gif
dot -Tgif graph58.dot -o graph58.gif
dot -Tgif graph59.dot -o graph59.gif
dot -Tgif graph60.dot -o graph60.gif
dot -Tgif graph61.dot -o graph61.gif
dot -Tgif graph62.dot -o graph62.gif
dot -Tgif graph63.dot -o graph63.gif
dot -Tgif graph64.dot -o graph64.gif
dot -Tgif graph65.dot -o graph65.gif
dot -Tgif graph66.dot -o graph66.gif
dot -Tgif graph67.dot -o graph67.gif
dot -Tgif graph68.dot -o graph68.gif
dot -Tgif graph69.dot -o graph69.gif
dot -Tgif graph70.dot -o graph70.gif
dot -Tgif graph71.dot -o graph71.gif
dot -Tgif graph72.dot -o graph72.gif
dot -Tgif graph73.dot -o graph73.gif
dot -Tgif graph74.dot -o graph74.gif
dot -Tgif graph75.dot -o graph75.gif
dot -Tgif graph76.dot -o graph76.gif
dot -Tgif graph77.dot -o graph77.gif
dot -Tgif graph78.dot -o graph78.gif
dot -Tgif graph79.dot -o graph79.gif
dot -Tgif graph80.dot -o graph80.gif
dot -Tgif graph81.dot -o graph81.gif
dot -Tgif graph82.dot -o graph82.gif
dot -Tgif graph83.dot -o graph83.gif
dot -Tgif graph84.dot -o graph84.gif
dot -Tgif graph85.dot -o graph85.gif
dot -Tgif graph86.dot -o graph86.gif
dot -Tgif graph87.dot -o graph87.gif
dot -Tgif graph88.dot -o graph88.gif
dot -Tgif graph89.dot -o graph89.gif
dot -Tgif graph90.dot -o graph90.gif
dot -Tgif graph91.dot -o graph91.gif
dot -Tgif graph92.dot -o graph92.gif
dot -Tgif graph93.dot -o graph93.gif
dot -Tgif graph94.dot -o graph94.gif
dot -Tgif graph95.dot -o graph95.gif
dot -Tgif graph96.dot -o graph96.gif
dot -Tgif graph97.dot -o graph97.gif
dot -Tgif graph98.dot -o graph98.gif
dot -Tgif graph99.dot -o graph99.gif
dot -Tgif graph100.dot -o graph100.gif
dot -Tgif graph101.dot -o graph101.gif
dot -Tgif graph102.dot -o graph102.gif
dot -Tgif graph103.dot -o graph103.gif
dot -Tgif graph104.dot -o graph104.gif
dot -Tgif graph105.dot -o graph105.gif
dot -Tgif graph106.dot -o graph106.gif
dot -Tgif graph107.dot -o graph107.gif
dot -Tgif graph108.dot -o graph108.gif
dot -Tgif graph109.dot -o graph109.gif
dot -Tgif graph110.dot -o graph110.gif
dot -Tgif graph111.dot -o graph111.gif
dot -Tgif graph112.dot -o graph112.gif
dot -Tgif graph113.dot -o graph113.gif
dot -Tgif graph114.dot -o graph114.gif
dot -Tgif graph115.dot -o graph115.gif
dot -Tgif graph116.dot -o graph116.gif
dot -Tgif graph117.dot -o graph117.gif
dot -Tgif graph118.dot -o graph118.gif
dot -Tgif graph119.dot -o graph119.gif
dot -Tgif graph120.dot -o graph120.gif
dot -Tgif graph121.dot -o graph121.gif
dot -Tgif graph122.dot -o graph122.gif
dot -Tgif graph123.dot -o graph123.gif
dot -Tgif graph124.dot -o graph124.gif
dot -Tgif graph125.dot -o graph125.gif
dot -Tgif graph126.dot -o graph126.gif
dot -Tgif graph127.dot -o graph127.gif
dot -Tgif graph128.dot -o graph128.gif
dot -Tgif graph129.dot -o graph129.gif
dot -Tgif graph130.dot -o graph130.gif
dot -Tgif graph131.dot -o graph131.gif
dot -Tgif graph132.dot -o graph132.gif
dot -Tgif graph133.dot -o graph133.gif
dot -Tgif graph134.dot -o graph134.gif
dot -Tgif graph135.dot -o graph135.gif
dot -Tgif graph136.dot -o graph136.gif
dot -Tgif graph137.dot -o graph137.gif
dot -Tgif graph138.dot -o graph138.gif
dot -Tgif graph139.dot -o graph139.gif
dot -Tgif graph140.dot -o graph140.gif
dot -Tgif graph141.dot -o graph141.gif
dot -Tgif graph142.dot -o graph142.gif
dot -Tgif graph143.dot -o graph143.gif
dot -Tgif graph144.dot -o graph144.gif
dot -Tgif graph145.dot -o graph145.gif
dot -Tgif graph146.dot -o graph146.gif
dot -Tgif graph147.dot -o graph147.gif
dot -Tgif graph148.dot -o graph148.gif
dot -Tgif graph149.dot -o graph149.gif
dot -Tgif graph150.dot -o graph150.gif
dot -Tgif graph151.dot -o graph151.gif
dot -Tgif graph152.dot -o graph152.gif
dot -Tgif graph153.dot -o graph153.gif
dot -Tgif graph154.dot -o graph154.gif
dot -Tgif graph155.dot -o graph155.gif
dot -Tgif graph156.dot -o graph156.gif
dot -Tgif graph157.dot -o graph157.gif
dot -Tgif graph158.dot -o graph158.gif
dot -Tgif graph159.dot -o graph159.gif
dot -Tgif graph160.dot -o graph160.gif
dot -Tgif graph161.dot -o graph161.gif
dot -Tgif graph162.dot -o graph162.gif
dot -Tgif graph163.dot -o graph163.gif
dot -Tgif graph164.dot -o graph164.gif
dot -Tgif graph165.dot -o graph165.gif
dot -Tgif graph166.dot -o graph166.gif
dot -Tgif graph167.dot -o graph167.gif
dot -Tgif graph168.dot -o graph168.gif
dot -Tgif graph169.dot -o graph169.gif
dot -Tgif graph170.dot -o graph170.gif
dot -Tgif graph171.dot -o graph171.gif
dot -Tgif graph172.dot -o graph172.gif
dot -Tgif graph173.dot -o graph173.gif
dot -Tgif graph174.dot -o graph174.gif
dot -Tgif graph175.dot -o graph175.gif
dot -Tgif graph176.dot -o graph176.gif
dot -Tgif graph177.dot -o graph177.gif
dot -Tgif graph178.dot -o graph178.gif
dot -Tgif graph179.dot -o graph179.gif
dot -Tgif graph180.dot -o graph180.gif
dot -Tgif graph181.dot -o graph181.gif
dot -Tgif graph182.dot -o graph182.gif
dot -Tgif graph183.dot -o graph183.gif
dot -Tgif graph184.dot -o graph184.gif
dot -Tgif graph185.dot -o graph185.gif
dot -Tgif graph186.dot -o graph186.gif
dot -Tgif graph187.dot -o graph187.gif
dot -Tgif graph188.dot -o graph188.gif
dot -Tgif graph189.dot -o graph189.gif
dot -Tgif graph190.dot -o graph190.gif
dot -Tgif graph191.dot -o graph191.gif
dot -Tgif graph192.dot -o graph192.gif
dot -Tgif graph193.dot -o graph193.gif
dot -Tgif graph194.dot -o graph194.gif
dot -Tgif graph195.dot -o graph195.gif
dot -Tgif graph196.dot -o graph196.gif
dot -Tgif graph197.dot -o graph197.gif
dot -Tgif graph198.dot -o graph198.gif
dot -Tgif graph199.dot -o graph199.gif
dot -Tgif graph200.dot -o graph200.gif
dot -Tgif graph201.dot -o graph201.gif
dot -Tgif graph202.dot -o graph202.gif
dot -Tgif graph203.dot -o graph203.gif
dot -Tgif graph204.dot -o graph204.gif
dot -Tgif graph205.dot -o graph205.gif
dot -Tgif graph206.dot -o graph206.gif
dot -Tgif graph207.dot -o graph207.gif
dot -Tgif graph208.dot -o graph208.gif
dot -Tgif graph209.dot -o graph209.gif
dot -Tgif graph210.dot -o graph210.gif
dot -Tgif graph211.dot -o graph211.gif
dot -Tgif graph212.dot -o graph212.gif
dot -Tgif graph213.dot -o graph213.gif
dot -Tgif graph214.dot -o graph214.gif
dot -Tgif graph215.dot -o graph215.gif
dot -Tgif graph216.dot -o graph216.gif
dot -Tgif graph217.dot -o graph217.gif
dot -Tgif graph218.dot -o graph218.gif
dot -Tgif graph219.dot -o graph219.gif
dot -Tgif graph220.dot -o graph220.gif
dot -Tgif graph221.dot -o graph221.gif
dot -Tgif graph222.dot -o graph222.gif
dot -Tgif graph223.dot -o graph223.gif
dot -Tgif graph224.dot -o graph224.gif
dot -Tgif graph225.dot -o graph225.gif
dot -Tgif graph226.dot -o graph226.gif
dot -Tgif graph227.dot -o graph227.gif
dot -Tgif graph228.dot -o graph228.gif
dot -Tgif graph229.dot -o graph229.gif
dot -Tgif graph230.dot -o graph230.gif
dot -Tgif graph231.dot -o graph231.gif
dot -Tgif graph232.dot -o graph232.gif
dot -Tgif graph233.dot -o graph233.gif
dot -Tgif graph234.dot -o graph234.gif
dot -Tgif graph235.dot -o graph235.gif
dot -Tgif graph236.dot -o graph236.gif
dot -Tgif graph237.dot -o graph237.gif
dot -Tgif graph238.dot -o graph238.gif
dot -Tgif graph239.dot -o graph239.gif
dot -Tgif graph240.dot -o graph240.gif
dot -Tgif graph241.dot -o graph241.gif
dot -Tgif graph242.dot -o graph242.gif
dot -Tgif graph243.dot -o graph243.gif
dot -Tgif graph244.dot -o graph244.gif
dot -Tgif graph245.dot -o graph245.gif
dot -Tgif graph246.dot -o graph246.gif
dot -Tgif graph247.dot -o graph247.gif
dot -Tgif graph248.dot -o graph248.gif
dot -Tgif graph249.dot -o graph249.gif
dot -Tgif graph250.dot -o graph250.gif
dot -Tgif graph251.dot -o graph251.gif
dot -Tgif graph252.dot -o graph252.gif
dot -Tgif graph253.dot -o graph253.gif
dot -Tgif graph254.dot -o graph254.gif
dot -Tgif graph255.dot -o graph255.gif
dot -Tgif graph256.dot -o graph256.gif
dot -Tgif graph257.dot -o graph257.gif
dot -Tgif graph258.dot -o graph258.gif
dot -Tgif graph259.dot -o graph259.gif
dot -Tgif graph260.dot -o graph260.gif
dot -Tgif graph261.dot -o graph261.gif
dot -Tgif graph262.dot -o graph262.gif
dot -Tgif graph263.dot -o graph263.gif
dot -Tgif graph264.dot -o graph264.gif
dot -Tgif graph265.dot -o graph265.gif
dot -Tgif graph266.dot -o graph266.gif
dot -Tgif graph267.dot -o graph267.gif
dot -Tgif graph268.dot -o graph268.gif
dot -Tgif graph269.dot -o graph269.gif
dot -Tgif graph270.dot -o graph270.gif
dot -Tgif graph271.dot -o graph271.gif
dot -Tgif graph272.dot -o graph272.gif
dot -Tgif graph273.dot -o graph273.gif
dot -Tgif graph274.dot -o graph274.gif
dot -Tgif graph275.dot -o graph275.gif
dot -Tgif graph276.dot -o graph276.gif
dot -Tgif graph277.dot -o graph277.gif
dot -Tgif graph278.dot -o graph278.gif
dot -Tgif graph279.dot -o graph279.gif
dot -Tgif graph280.dot -o graph280.gif
dot -Tgif graph281.dot -o graph281.gif
dot -Tgif graph282.dot -o graph282.gif
dot -Tgif graph283.dot -o graph283.gif
dot -Tgif graph284.dot -o graph284.gif
dot -Tgif graph285.dot -o graph285.gif
dot -Tgif graph286.dot -o graph286.gif
dot -Tgif graph287.dot -o graph287.gif
dot -Tgif graph288.dot -o graph288.gif
dot -Tgif graph289.dot -o graph289.gif
dot -Tgif graph290.dot -o graph290.gif
dot -Tgif graph291.dot -o graph291.gif
dot -Tgif graph292.dot -o graph292.gif
dot -Tgif graph293.dot -o graph293.gif
dot -Tgif graph294.dot -o graph294.gif
dot -Tgif graph295.dot -o graph295.gif
dot -Tgif graph296.dot -o graph296.gif
dot -Tgif graph297.dot -o graph297.gif
dot -Tgif graph298.dot -o graph298.gif
dot -Tgif graph299.dot -o graph299.gif
dot -Tgif graph300.dot -o graph300.gif
dot -Tgif graph301.dot -o graph301.gif
dot -Tgif graph302.dot -o graph302.gif
dot -Tgif graph303.dot -o graph303.gif
dot -Tgif graph304.dot -o graph304.gif
dot -Tgif graph305.dot -o graph305.gif
dot -Tgif graph306.dot -o graph306.gif
dot -Tgif graph307.dot -o graph307.gif
dot -Tgif graph308.dot -o graph308.gif
dot -Tgif graph309.dot -o graph309.gif
dot -Tgif graph310.dot -o graph310.gif
dot -Tgif graph311.dot -o graph311.gif
dot -Tgif graph312.dot -o graph312.gif
dot -Tgif graph313.dot -o graph313.gif
dot -Tgif graph314.dot -o graph314.gif
dot -Tgif graph315.dot -o graph315.gif
dot -Tgif graph316.dot -o graph316.gif
dot -Tgif graph317.dot -o graph317.gif
dot -Tgif graph318.dot -o graph318.gif
dot -Tgif graph319.dot -o graph319.gif
dot -Tgif graph320.dot -o graph320.gif
dot -Tgif graph321.dot -o graph321.gif
dot -Tgif graph322.dot -o graph322.gif
dot -Tgif graph323.dot -o graph323.gif
dot -Tgif graph324.dot -o graph324.gif
dot -Tgif graph325.dot -o graph325.gif
dot -Tgif graph326.dot -o graph326.gif
dot -Tgif graph327.dot -o graph327.gif
dot -Tgif graph328.dot -o graph328.gif
dot -Tgif graph329.dot -o graph329.gif
dot -Tgif graph330.dot -o graph330.gif
dot -Tgif graph331.dot -o graph331.gif
dot -Tgif graph332.dot -o graph332.gif
dot -Tgif graph333.dot -o graph333.gif
dot -Tgif graph334.dot -o graph334.gif
dot -Tgif graph335.dot -o graph335.gif
dot -Tgif graph336.dot -o graph336.gif
dot -Tgif graph337.dot -o graph337.gif
dot -Tgif graph338.dot -o graph338.gif
dot -Tgif graph339.dot -o graph339.gif
dot -Tgif graph340.dot -o graph340.gif
dot -Tgif graph341.dot -o graph341.gif
dot -Tgif graph342.dot -o graph342.gif
dot -Tgif graph343.dot -o graph343.gif
dot -Tgif graph344.dot -o graph344.gif
dot -Tgif graph345.dot -o graph345.gif
dot -Tgif graph346.dot -o graph346.gif
dot -Tgif graph347.dot -o graph347.gif
dot -Tgif graph348.dot -o graph348.gif
dot -Tgif graph349.dot -o graph349.gif
dot -Tgif graph350.dot -o graph350.gif
dot -Tgif graph351.dot -o graph351.gif
dot -Tgif graph352.dot -o graph352.gif
dot -Tgif graph353.dot -o graph353.gif
dot -Tgif graph354.dot -o graph354.gif
dot -Tgif graph355.dot -o graph355.gif
dot -Tgif graph356.dot -o graph356.gif
dot -Tgif graph357.dot -o graph357.gif
dot -Tgif graph358.dot -o graph358.gif
dot -Tgif graph359.dot -o graph359.gif
dot -Tgif graph360.dot -o graph360.gif
dot -Tgif graph361.dot -o graph361.gif
dot -Tgif graph362.dot -o graph362.gif
dot -Tgif graph363.dot -o graph363.gif
dot -Tgif graph364.dot -o graph364.gif
dot -Tgif graph365.dot -o graph365.gif
dot -Tgif graph366.dot -o graph366.gif
dot -Tgif graph367.dot -o graph367.gif
dot -Tgif graph368.dot -o graph368.gif
dot -Tgif graph369.dot -o graph369.gif
dot -Tgif graph370.dot -o graph370.gif
dot -Tgif graph371.dot -o graph371.gif
dot -Tgif graph372.dot -o graph372.gif
dot -Tgif graph373.dot -o graph373.gif
dot -Tgif graph374.dot -o graph374.gif
dot -Tgif graph375.dot -o graph375.gif
dot -Tgif graph376.dot -o graph376.gif
dot -Tgif graph377.dot -o graph377.gif
dot -Tgif graph378.dot -o graph378.gif
dot -Tgif graph379.dot -o graph379.gif
dot -Tgif graph380.dot -o graph380.gif
dot -Tgif graph381.dot -o graph381.gif
dot -Tgif graph382.dot -o graph382.gif
dot -Tgif graph383.dot -o graph383.gif
dot -Tgif graph384.dot -o graph384.gif
dot -Tgif graph385.dot -o graph385.gif
dot -Tgif graph386.dot -o graph386.gif
dot -Tgif graph387.dot -o graph387.gif
dot -Tgif graph388.dot -o graph388.gif
dot -Tgif graph389.dot -o graph389.gif
dot -Tgif graph390.dot -o graph390.gif
dot -Tgif graph391.dot -o graph391.gif
dot -Tgif graph392.dot -o graph392.gif
dot -Tgif graph393.dot -o graph393.gif
dot -Tgif graph394.dot -o graph394.gif
dot -Tgif graph395.dot -o graph395.gif
dot -Tgif graph396.dot -o graph396.gif
dot -Tgif graph397.dot -o graph397.gif
dot -Tgif graph398.dot -o graph398.gif
dot -Tgif graph399.dot -o graph399.gif
dot -Tgif graph400.dot -o graph400.gif
dot -Tgif graph401.dot -o graph401.gif
dot -Tgif graph402.dot -o graph402.gif
dot -Tgif graph403.dot -o graph403.gif
dot -Tgif graph404.dot -o graph404.gif
dot -Tgif graph405.dot -o graph405.gif
dot -Tgif graph406.dot -o graph406.gif
dot -Tgif graph407.dot -o graph407.gif
dot -Tgif graph408.dot -o graph408.gif
dot -Tgif graph409.dot -o graph409.gif
dot -Tgif graph410.dot -o graph410.gif
dot -Tgif graph411.dot -o graph411.gif
dot -Tgif graph412.dot -o graph412.gif
dot -Tgif graph413.dot -o graph413.gif
dot -Tgif graph414.dot -o graph414.gif
dot -Tgif graph415.dot -o graph415.gif
dot -Tgif graph416.dot -o graph416.gif
dot -Tgif graph417.dot -o graph417.gif
dot -Tgif graph418.dot -o graph418.gif
dot -Tgif graph419.dot -o graph419.gif
dot -Tgif graph420.dot -o graph420.gif
dot -Tgif graph421.dot -o graph421.gif
dot -Tgif graph422.dot -o graph422.gif
dot -Tgif graph423.dot -o graph423.gif
dot -Tgif graph424.dot -o graph424.gif
dot -Tgif graph425.dot -o graph425.gif
dot -Tgif graph426.dot -o graph426.gif
dot -Tgif graph427.dot -o graph427.gif
dot -Tgif graph428.dot -o graph428.gif
dot -Tgif graph429.dot -o graph429.gif
dot -Tgif graph430.dot -o graph430.gif
dot -Tgif graph431.dot -o graph431.gif
dot -Tgif graph432.dot -o graph432.gif
dot -Tgif graph433.dot -o graph433.gif
dot -Tgif graph434.dot -o graph434.gif
dot -Tgif graph435.dot -o graph435.gif
dot -Tgif graph436.dot -o graph436.gif
dot -Tgif graph437.dot -o graph437.gif
dot -Tgif graph438.dot -o graph438.gif
dot -Tgif graph439.dot -o graph439.gif
dot -Tgif graph440.dot -o graph440.gif
dot -Tgif graph441.dot -o graph441.gif
dot -Tgif graph442.dot -o graph442.gif
dot -Tgif graph443.dot -o graph443.gif
dot -Tgif graph444.dot -o graph444.gif
dot -Tgif graph445.dot -o graph445.gif
dot -Tgif graph446.dot -o graph446.gif
dot -Tgif graph447.dot -o graph447.gif
dot -Tgif graph448.dot -o graph448.gif
dot -Tgif graph449.dot -o graph449.gif
dot -Tgif graph450.dot -o graph450.gif
dot -Tgif graph451.dot -o graph451.gif
dot -Tgif graph452.dot -o graph452.gif
dot -Tgif graph453.dot -o graph453.gif
dot -Tgif graph454.dot -o graph454.gif
dot -Tgif graph455.dot -o graph455.gif
dot -Tgif graph456.dot -o graph456.gif
dot -Tgif graph457.dot -o graph457.gif
dot -Tgif graph458.dot -o graph458.gif
dot -Tgif graph459.dot -o graph459.gif
dot -Tgif graph460.dot -o graph460.gif
dot -Tgif graph461.dot -o graph461.gif
dot -Tgif graph462.dot -o graph462.gif
dot -Tgif graph463.dot -o graph463.gif
dot -Tgif graph464.dot -o graph464.gif
dot -Tgif graph465.dot -o graph465.gif
dot -Tgif graph466.dot -o graph466.gif
dot -Tgif graph467.dot -o graph467.gif
dot -Tgif graph468.dot -o graph468.gif
dot -Tgif graph469.dot -o graph469.gif


#./installBDD.sh
#g++ -c -g circuitTest.cc

