// the outer header for the package (include it in the threaded classes or teh main method files)
#ifndef _PACKAGE_HEADER
#define _PACKAGE_HEADER

#include "BDDUtil.h"
#include "Hash.h"

#include "Manager.h"
#include "circuit.hh"


// sturctures for console arguments
int opt;
struct option longopts[] = {
{"file", 1, NULL, 'f'},		// .lev file name
{"threadno", 1, NULL, 't'},	// no. of threads
{"incap", 1, NULL, 'i'},		// input capacity
{"hcd", 1, NULL, 'a'},		// hash concurrency degree
{"hmu", 1, NULL, 'b'},		// hash Max. Utilization
{"hbs", 1, NULL, 'd'},		// hash bucket size
{"comptable", 1, NULL, 'c'},	// enable computed table
{"quecap", 1, NULL, 'q'},	// queue size
{"qdc", 1, NULL, 'e'},		// queue concurrency degree
{"qmu", 1, NULL, 'g'},		// queue max. utlilization
{"qbs", 1, NULL, 'h'},		// queue bucket size
{"hic", 1, NULL, 'j'},		// hash Table Integrity check ( 0 - no check; 1 - generate raw data; 2 - do check
{"GCthreadsNo", 1, NULL, 'l'},	// no. of threads used by GC
{"utgc", 1, NULL, 'k'},		// Unique table garbage collector
{"vr", 1, NULL, 'm'},		// variable reordering type
{"rztyp", 1, NULL, 'n'}		// resize type
};
#endif


